import tkinter as tk
from tkinter import messagebox, scrolledtext, simpledialog
import threading
import time
from PIL import Image, ImageTk

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
        self.user_entry.insert(0, "Nico")
        self.user_entry.grid(row=row, column=1, padx=5, pady=3)
        row += 1

        tk.Label(self, text="Contraseña:").grid(row=row, column=0, sticky="e")
        self.pass_entry = tk.Entry(self, show="*")
        self.pass_entry.insert(0, "777")
        self.pass_entry.grid(row=row, column=1, padx=5, pady=3)
        row += 1

        self.btn_connect = tk.Button(self, text="Conectar y Login", command=self.attempt_login)
        self.btn_connect.grid(row=row, column=0, columnspan=2, pady=8)
        row += 1

        Logo = Image.open("Logo/image.png")
        Logo = ImageTk.PhotoImage(Logo)
        self.logo_label = tk.Label(self, image=Logo)
        self.logo_label.image = Logo
        self.logo_label.grid(row=row, columnspan=2, pady=10)

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
                
                #print(f"Respuesta-Inicio: {res}")
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

        # --- Tareas ---
        tarea_frame = tk.LabelFrame(self, text="Tareas")
        tarea_frame.pack(fill="x", padx=5, pady=5)

        self.task_mode_var = tk.BooleanVar(value=False)
        self.task_number_var = tk.StringVar(value="1")
        self.task_prefix_var = tk.StringVar(value="tarea ")

        self.chk_task_mode = tk.Checkbutton(
            tarea_frame, text="Modo guardar en tarea", variable=self.task_mode_var,
            command=lambda: self.log_msg(
                f"Modo tarea: {'ON' if self.task_mode_var.get() else 'OFF'} (nro={self.task_number_var.get()})"
            )
        )
        self.chk_task_mode.grid(row=0, column=0, sticky="w", padx=3)

        tk.Label(tarea_frame, text="Nro:").grid(row=0, column=1, sticky="e")
        self.task_number_entry = tk.Entry(tarea_frame, width=6, textvariable=self.task_number_var)
        self.task_number_entry.grid(row=0, column=2, padx=3)

        tk.Label(tarea_frame, text="Nombre base:").grid(row=0, column=3, sticky="e")
        self.task_prefix_entry = tk.Entry(tarea_frame, width=10, textvariable=self.task_prefix_var)
        self.task_prefix_entry.grid(row=0, column=4, padx=3)

        self.btn_run_task = tk.Button(tarea_frame, text="Ejecutar tarea", command=self.run_task_dialog)
        self.btn_run_task.grid(row=0, column=5, padx=6)

        # --- Funciones adicionales ---
        extra_frame = tk.LabelFrame(self, text="Reporte")
        extra_frame.pack(fill="x", padx=5, pady=5)

        tk.Button(extra_frame, text="Filtrar", command=self.reporte_filtrado).grid(row=0, column=9, padx=5, pady=5)
        niveles = ["INFO", "WARNING", "ERROR", "DEBUG"]
        dominios = ["main", "G_Code", "Reporte", "Inicio"]

        tk.Label(extra_frame, text="Nivel:").grid(row=0, column=1, padx=5, pady=5)
        self.nivel_var = tk.StringVar(value=niveles[0])
        tk.OptionMenu(extra_frame, self.nivel_var, *niveles).grid(row=0, column=3, padx=5, pady=5)

        tk.Label(extra_frame, text="Dominio").grid(row=0, column=5, padx=5, pady=5)
        self.dominio_var = tk.StringVar(value=dominios[0])
        tk.OptionMenu(extra_frame, self.dominio_var, *dominios).grid(row=0, column=7, padx=5, pady=5)

                # --- Estado y comandos ---
        info_frame = tk.Frame(self)
        info_frame.pack(fill="both", expand=True, padx=5, pady=5)

        # --- Área de log ocupando todo ---
        tk.Label(info_frame, text="Log / Respuestas:").pack(anchor="w")
        self.log = scrolledtext.ScrolledText(info_frame, height=12, state="disabled")
        self.log.pack(fill="both", expand=True)



        #boton help
        self.frame_botones = tk.Frame(self)
        self.frame_botones.pack(fill="x", padx=5, pady=5)

        self.btn_help = tk.Button(self.frame_botones, text="Help", command=self.toggle_help_buttons)
        self.btn_help.pack(pady=5)

        self.btn_help_gcode = tk.Button(self.frame_botones, text="Help G_Code", command=self.help_gcode)
        self.btn_help_reporte = tk.Button(self.frame_botones, text="Help Reporte", command=self.help_reporte)
        self.help_visible = False 
        


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
                # Si está activo el modo tarea, agregamos la línea a la tarea y no ejecutamos G_Code
                if self.task_mode_var.get():
                    try:
                        g = int(g_code)
                    except ValueError:
                        raise Exception("G-Code debe ser entero para modo tarea")

                    # Generar linea: <ID><num> [Xv] [Yv] [Zv]
                    if g < 1000:
                        head = f"G{g}"
                    else:
                        gs = str(g)
                        head = f"M{gs[4:]}" if len(gs) > 4 else f"M{g}"

                    tokens = [head, f"X{x}", f"Y{y}", f"Z{z}"]
                    line = " ".join(tokens)
                    num = self.task_number_var.get().strip() or "1"
                    prefix = self.task_prefix_var.get()
                    name = f"{prefix}{num}".strip()

                    res = self.rpc.tarea(["add", name, line])
                    self.master.after(0, lambda: self.log_msg(f"tarea add [{name}] <- {line} -> {res}"))
                    return

                # Modo normal: enviar a G_Code
                print(f"{g_code}, [{posicion}]")
                res = self.rpc.move_xyz(int(g_code), posicion)
                print("Respuesta de RPC:", res)
                self.master.after(0, lambda: self.log_msg(f"move_xyz(G{g_code}, {x},{y},{z}) -> {res}"))
            except Exception as e:
                msg = f"ERROR move_xyz: {e}"
                print(msg)
                self.master.after(0, lambda: self.log_msg(msg))



        threading.Thread(target=do_move, daemon=True).start()

    def home(self):
        def do_home():
            try:
                res = self.rpc.home()
                self.master.after(0, lambda: self.log_msg(f"home() -> {res}"))
            except Exception as e:
                self.master.after(0, lambda: self.log_msg(f"ERROR home: {e}"))
        threading.Thread(target=do_home, daemon=True).start()

    # --- Ejecutar tarea (pide número) ---
    def run_task_dialog(self):
        try:
            num = simpledialog.askstring("Ejecutar tarea", "Número de tarea:", parent=self)
            if num is None:
                return
            num = num.strip()
            if not num:
                messagebox.showwarning("Dato requerido", "Debes ingresar un número de tarea.")
                return
            prefix = self.task_prefix_var.get()
            name = f"{prefix}{num}".strip()

            def t():
                try:
                    res = self.rpc.tarea(["run", name])
                    self.master.after(0, lambda: self.log_msg(f"Tarea run [{name}] ->\n{res}"))
                except Exception as e:
                    self.master.after(0, lambda: self.log_msg(f"ERROR run tarea: {e}"))
            threading.Thread(target=t, daemon=True).start()
        except Exception as e:
            messagebox.showerror("Ejecutar tarea", f"Error: {e}")

    def reporte_filtrado(self):
        def t():
            try:
                # Diccionarios tipo switch
                niveles = {
                    "INFO": 0,
                    "WARNING": 1,
                    "ERROR": 2,
                    "DEBUG": 3
                }

                dominios = {
                    "main": 0,
                    "G_Code": 1,
                    "Reporte": 2,
                    "Inicio": 3
                }

                # Obtiene los valores seleccionados en los OptionMenu
                nivel_str = self.nivel_var.get()
                dominio_str = self.dominio_var.get()

                # Traduce a número según diccionario (default 0 si no se encuentra)
                nivel = niveles.get(nivel_str, 0)
                dominio = dominios.get(dominio_str, 0)

                # Llamada RPC
                res = self.rpc.reporte(nivel, dominio)
                self.master.after(0, lambda: self.log_msg(f"Reporte filtrado ({nivel_str}, {dominio_str}) -> {res}"))

            except Exception as e:
                msg = f"ERROR reporte_filtrado: {e}"
                self.master.after(0, lambda: self.log_msg(msg))
        threading.Thread(target=t, daemon=True).start()

        

    def help_move(self):
        def t():
            try:
                res = self.rpc.help_move()
                self.master.after(0, lambda: self.log_msg(f"HelpMove -> {res}"))
            except Exception as e:
                self.master.after(0, lambda: self.log_msg(f"ERROR help_move: {e}"))
        threading.Thread(target=t, daemon=True).start()

    def toggle_help_buttons(self):
        if not self.help_visible:
            self.btn_help_gcode.pack(pady=2)
            self.btn_help_reporte.pack(pady=2)
            self.help_visible = True
        else:
            self.btn_help_gcode.pack_forget()
            self.btn_help_reporte.pack_forget()
            self.help_visible = False

    def help_gcode(self):
        def t():
            try:
                res = self.rpc.help("G_Code")
                self.master.after(0, lambda: self.log_msg(f"Help G_Code ->\n{res}"))
            except Exception as e:
                self.master.after(0, lambda: self.log_msg(f"ERROR Help G_Code: {e}"))
        threading.Thread(target=t, daemon=True).start()


    def help_reporte(self):
        def t():
            try:
                res = self.rpc.help("Reporte")
                self.master.after(0, lambda: self.log_msg(f"Help Reporte ->\n{res}"))
            except Exception as e:
                self.master.after(0, lambda: self.log_msg(f"ERROR Help Reporte: {e}"))
        threading.Thread(target=t, daemon=True).start()

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
