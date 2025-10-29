# client.py
"""
Cliente GUI simple en tkinter que usa XML-RPC.
Estructura:
 - RobotRPCClient: wrapper de llamadas RPC. Cambiar métodos aquí si el servidor usa otros nombres.
 - LoginFrame: pide IP, puerto, usuario y contraseña.
 - MainFrame: botones para home, move, get_status, list_commands y un log.
Cambiar SERVER_IP/PORT desde la UI antes de conectarse.
"""

import tkinter as tk

from ui import *
from RPCwrapper import *

class RobotClientApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Client RPC - Robot Controller")
        self.geometry("720x480")
        self.resizable(True, True)
        # cliente RPC central
        self.rpc_client = RobotRPCClient()
        # frames
        self.current_frame = None
        self.show_login()

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
    app = RobotClientApp()
    app.mainloop()
