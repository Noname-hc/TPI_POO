import tkinter as tk
from tkinter import messagebox, scrolledtext
import threading
import time

class LoginFrame(tk.Frame):
    def __init__(self, master, rpc_client, on_success, *args, **kwargs):
        super().__init__(master, *args, **kwargs)
        self.rpc = rpc_client
        self.on_success = on_success
        self.create_widgets()

    def create_widgets(self):
        row = 0
        tk.Label(self, text="Server IP:").grid(row=row, column=0, sticky="e")
        self.ip_entry = tk.Entry(self)
        self.ip_entry.insert(0, self.rpc.server_ip)
        self.ip_entry.grid(row=row, column=1, padx=5, pady=3)
        row += 1

        tk.Label(self, text="Server Port:").grid(row=row, column=0, sticky="e")
        self.port_entry = tk.Entry(self)
        self.port_entry.insert(0, self.rpc.server_port)
        self.port_entry.grid(row=row, column=1, padx=5, pady=3)
        row += 1

        tk.Label(self, text="Usuario:").grid(row=row, column=0, sticky="e")
        self.user_entry = tk.Entry(self)
        self.user_entry.grid(row=row, column=1, padx=5, pady=3)
        row += 1

        tk.Label(self, text="Contraseña:").grid(row=row, column=0, sticky="e")
        self.pass_entry = tk.Entry(self, show="*")
        self.pass_entry.grid(row=row, column=1, padx=5, pady=3)
        row += 1

        self.btn_connect = tk.Button(self, text="Conectar y Login", command=self.attempt_login)
        self.btn_connect.grid(row=row, column=0, columnspan=2, pady=8)

    def attempt_login(self):
        ip = self.ip_entry.get().strip()
        port = self.port_entry.get().strip()
        user = self.user_entry.get().strip()
        pwd = self.pass_entry.get().strip()

        if not ip or not port or not user:
            messagebox.showwarning("Datos incompletos", "Rellena IP, puerto y usuario.")
            return

        try:
            self.rpc.connect(server_ip=ip, server_port=port)
        except Exception as e:
            messagebox.showerror("Conexión", f"No se pudo conectar al servidor: {e}")
            return

        # Llamada de login en hilo para no bloquear GUI
        def do_login():
            try:
                res = self.rpc.login(user, pwd)
                # Se espera que el servidor devuelva True/False o "OK"/"ERROR"
                ok = False
                
                if res in ("Bienvenido cliente", "Bienvenido administrador"):
                    ok = True
                else:
                    ok = False

                if ok:
                    self.master.after(0, lambda: self.on_success(user))
                else:
                    self.master.after(0, lambda: messagebox.showerror("Login fallido", f"Respuesta del servidor: {res}"))
            except Exception as e:
                msg = str(e)   # ← capturo acá
                self.master.after(0, lambda m=msg: messagebox.showerror("Login error", f"Error al autenticarse: {m}"))

        threading.Thread(target=do_login, daemon=True).start()


class MainFrame(tk.Frame):
    def __init__(self, master, rpc_client, username, on_logout, *args, **kwargs):
        super().__init__(master, *args, **kwargs)
        self.rpc = rpc_client
        self.username = username
        self.on_logout = on_logout
        self.create_widgets()

    def create_widgets(self):
        # --- Usuario y logout ---
        top_frame = tk.Frame(self)
        top_frame.pack(fill="x", padx=5, pady=5)

        tk.Label(top_frame, text=f"Usuario: {self.username}").pack(side="left")

        logout_btn = tk.Button(top_frame, text="Logout", command=self.logout)
        logout_btn.pack(side="right")

        # --- Movimiento manual ---
        move_frame = tk.LabelFrame(self, text="Movimiento manual")
        move_frame.pack(fill="x", padx=5, pady=5)

        tk.Label(move_frame, text="X:").grid(row=0, column=0)
        self.x_entry = tk.Entry(move_frame, width=8)
        self.x_entry.insert(0, "0.0")
        self.x_entry.grid(row=0, column=1, padx=3)

        tk.Label(move_frame, text="Y:").grid(row=0, column=2)
        self.y_entry = tk.Entry(move_frame, width=8)
        self.y_entry.insert(0, "170.0")
        self.y_entry.grid(row=0, column=3, padx=3)

        tk.Label(move_frame, text="Z:").grid(row=0, column=4)
        self.z_entry = tk.Entry(move_frame, width=8)
        self.z_entry.insert(0, "120.0")
        self.z_entry.grid(row=0, column=5, padx=3)

        tk.Label(move_frame, text="G-Code:").grid(row=0, column=6)
        self.gcode_entry = tk.Entry(move_frame, width=8)
        self.gcode_entry.insert(0, "0")
        self.gcode_entry.grid(row=0, column=7, padx=3)

        move_btn = tk.Button(move_frame, text="Move XYZ", command=self.move_xyz)
        move_btn.grid(row=0, column=8, padx=8)

        home_btn = tk.Button(move_frame, text="Home", command=self.home)
        home_btn.grid(row=0, column=9, padx=4)

        # --- Funciones adicionales ---
        extra_frame = tk.LabelFrame(self, text="Funciones adicionales")
        extra_frame.pack(fill="x", padx=5, pady=5)

        tk.Button(extra_frame, text="Reporte", command=self.reporte).grid(row=0, column=0, padx=5, pady=5)
        tk.Button(extra_frame, text="HelpMove", command=self.help_move).grid(row=0, column=1, padx=5, pady=5)
        tk.Button(extra_frame, text="HelpReporte", command=self.help_reporte).grid(row=0, column=2, padx=5, pady=5)

        # --- Estado y comandos ---
        info_frame = tk.Frame(self)
        info_frame.pack(fill="both", expand=True, padx=5, pady=5)

        left = tk.Frame(info_frame)
        left.pack(side="left", fill="both", expand=True)

        tk.Button(left, text="Get Status", command=self.get_status).pack(fill="x", pady=2)
        tk.Button(left, text="List Commands", command=self.list_commands).pack(fill="x", pady=2)

        # --- Área de log ---
        right = tk.Frame(info_frame)
        right.pack(side="right", fill="both", expand=True)

        tk.Label(right, text="Log / Respuestas:").pack(anchor="w")
        self.log = scrolledtext.ScrolledText(right, height=12, state="disabled")
        self.log.pack(fill="both", expand=True)

    def log_msg(self, msg):
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
        self.log.configure(state="normal")
        self.log.insert("end", f"[{timestamp}] {msg}\n")
        self.log.see("end")
        self.log.configure(state="disabled")

    # --- Wrappers que ejecutan en hilo ---
    def move_xyz(self):
        x = self.x_entry.get().strip()
        y = self.y_entry.get().strip()
        z = self.z_entry.get().strip()
        g_code = self.gcode_entry.get().strip()

        # Validación
        try:
            float(x); float(y); float(z)
        except ValueError:
            messagebox.showwarning("Valores inválidos", "X, Y y Z deben ser numéricos.")
            return

        if not g_code.isdigit():
            messagebox.showwarning("Valor inválido", "El G-Code debe ser un número entero.")
            return

        posicion = f"{x},{y},{z}"

        def do_move():
            try:
                print(f"{g_code}, [{posicion}]")
                res = self.rpc.move_xyz(g_code, posicion)
                
                self.master.after(0, lambda: self.log_msg(f"move_xyz(G{g_code}, {x},{y},{z}) -> {res}"))
            except Exception as e:
                self.master.after(0, lambda: self.log_msg(f"ERROR move_xyz: {e}"))

        threading.Thread(target=do_move, daemon=True).start()

    def home(self):
        def do_home():
            try:
                res = self.rpc.home()
                self.master.after(0, lambda: self.log_msg(f"home() -> {res}"))
            except Exception as e:
                self.master.after(0, lambda: self.log_msg(f"ERROR home: {e}"))
        threading.Thread(target=do_home, daemon=True).start()

    def reporte(self):
        def t():
            try:
                res = self.rpc.reporte()
                self.master.after(0, lambda: self.log_msg(f"Reporte -> {res}"))
            except Exception as e:
                self.master.after(0, lambda: self.log_msg(f"ERROR reporte: {e}"))
        threading.Thread(target=t, daemon=True).start()

    def help_move(self):
        def t():
            try:
                res = self.rpc.help_move()
                self.master.after(0, lambda: self.log_msg(f"HelpMove -> {res}"))
            except Exception as e:
                self.master.after(0, lambda: self.log_msg(f"ERROR help_move: {e}"))
        threading.Thread(target=t, daemon=True).start()

    def help_reporte(self):
        messagebox.showinfo("HelpReporte", "Esta sección se completará más adelante.")

    def get_status(self):
        def do_status():
            try:
                res = self.rpc.get_status()
                self.master.after(0, lambda: self.log_msg(f"get_status() -> {res}"))
            except Exception as e:
                self.master.after(0, lambda: self.log_msg(f"ERROR get_status: {e}"))
        threading.Thread(target=do_status, daemon=True).start()

    def list_commands(self):
        def do_list():
            try:
                res = self.rpc.list_commands()
                self.master.after(0, lambda: self.log_msg(f"list_commands() -> {res}"))
            except Exception as e:
                self.master.after(0, lambda: self.log_msg(f"ERROR list_commands: {e}"))
        threading.Thread(target=do_list, daemon=True).start()

    def logout(self):
        if messagebox.askyesno("Logout", "Cerrar sesión y volver al login?"):
            self.on_logout()