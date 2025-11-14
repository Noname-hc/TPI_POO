# client.py

import os
import tkinter as tk
from ui import *
from RPCwrapper import *
import logging as log

# Configurar logging siempre relativo a este archivo, sin depender del CWD
_BASE = os.path.dirname(os.path.abspath(__file__))
_LOG_DIR = os.path.join(_BASE, "logs")
os.makedirs(_LOG_DIR, exist_ok=True)
_LOG_PATH = os.path.join(_LOG_DIR, "cliente.log")

log.basicConfig(filename=_LOG_PATH, level=log.INFO,
                format='[%(asctime)s]-[%(levelname)s]-[%(name)s]-%(message)s')

class RobotClientApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Cliente - Controlador de Robot")
        self.geometry("300x390")
        self.resizable(True, True)
        # cliente RPC central
        self.rpc_client = RobotRPCClient()
        # frames
        self.current_frame = None
        self.show_login()
        self.username = None
        self.password = None
        
    def clear_frame(self):
        if self.current_frame:
            self.current_frame.destroy()
            self.current_frame = None

    def show_login(self):
        self.clear_frame()
        self.current_frame = LoginFrame(self, self.rpc_client, on_success=self.on_login_success)
        self.current_frame.pack(fill="both", expand=True, padx=10, pady=10)

    def on_login_success(self, username):
        # show main frame
        self.clear_frame()
        self.current_frame = MainFrame(self, self.rpc_client, username=username, on_logout=self.show_login)
        self.current_frame.pack(fill="both", expand=True, padx=8, pady=8)
        # welcome log
        self.current_frame.log_msg(f"Login exitoso: {username}. Conectado a {self.rpc_client.server_ip}:{self.rpc_client.server_port}")

if __name__ == "__main__":
    log = log.getLogger(__name__)
    app = RobotClientApp()
    log.info("Cliente creado")
    app.mainloop()
