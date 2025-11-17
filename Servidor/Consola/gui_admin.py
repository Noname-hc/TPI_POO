#!/usr/bin/env python3
import sys
import os
import tkinter as tk
from tkinter import messagebox


def _add_client_path():
    """
    Agrega la carpeta Cliente/ al sys.path para poder reutilizar
    la misma UI y wrapper RPC del cliente.
    """
    base_dir = os.path.dirname(os.path.abspath(__file__))
    client_dir = os.path.abspath(os.path.join(base_dir, "..", "..", "Cliente"))
    if client_dir not in sys.path:
        sys.path.insert(0, client_dir)


def _ensure_pil_stub():
    """
    Asegura que el import "from PIL import Image, ImageTk" de Cliente/ui.py
    no falle aunque Pillow no esté instalado en este entorno.
    Si PIL existe, no se toca nada; si no, se registran módulos dummy.
    """
    try:
        import importlib
        # Import dynamically so static analyzers don't flag a missing PIL package,
        # but still attempt to detect Pillow at runtime.
        importlib.import_module("PIL")
        return  # Pillow disponible
    except Exception:
        pass

    import types

    pil_mod = types.ModuleType("PIL")
    image_mod = types.ModuleType("PIL.Image")
    imagetk_mod = types.ModuleType("PIL.ImageTk")

    def _open(*_args, **_kwargs):
        # No cargamos imagen; la UI del cliente ya envuelve esto en try/except.
        raise RuntimeError("Pillow (PIL) no disponible")

    def _photo_image(_img):
        # Devolvemos None; la UI manejará que no haya imagen.
        return None

    image_mod.open = _open
    imagetk_mod.PhotoImage = _photo_image
    pil_mod.Image = image_mod
    pil_mod.ImageTk = imagetk_mod

    sys.modules["PIL"] = pil_mod
    sys.modules["PIL.Image"] = image_mod
    sys.modules["PIL.ImageTk"] = imagetk_mod


def main():
    if len(sys.argv) < 5:
        print("Uso: gui_admin.py HOST PORT USER PASS")
        sys.exit(2)

    host = sys.argv[1]
    port = sys.argv[2]
    user = sys.argv[3]
    pwd = sys.argv[4]

    _add_client_path()
    _ensure_pil_stub()

    try:
        # Los módulos del cliente están en la carpeta Cliente/, que ya
        # agregamos al sys.path, por eso los importamos dinámicamente.
        import importlib
        rpc_mod = importlib.import_module("RPCwrapper")
        ui_mod = importlib.import_module("ui")
        RobotRPCClient = getattr(rpc_mod, "RobotRPCClient")
        MainFrame = getattr(ui_mod, "MainFrame")
    except Exception as e:
        print(f"Error importando módulos del cliente: {e}")
        messagebox.showerror("Importación", f"No se pudo importar la UI del cliente: {e}")
        sys.exit(1)

    # Crear cliente RPC reutilizando la misma lógica que el cliente
    rpc = RobotRPCClient(server_ip=host, server_port=port)
    try:
        rpc.connect(server_ip=host, server_port=port)
    except Exception as e:
        print(f"Error de conexión: {e}")
        messagebox.showerror("Conexión", f"No se pudo conectar al servidor: {e}")
        sys.exit(1)

    # Login y verificación de rol administrador
    try:
        res = rpc.login(user, pwd)
    except Exception as e:
        messagebox.showerror("Login", f"Error al autenticarse: {e}")
        sys.exit(1)

    if not (isinstance(res, str) and "Bienvenido administrador" in res):
        messagebox.showerror(
            "Permiso denegado",
            f"Se requiere usuario administrador.\nRespuesta del servidor: {res}"
        )
        sys.exit(1)

    # Crear ventana principal con la misma MainFrame del cliente
    root = tk.Tk()
    root.title("Servidor - Interfaz Admin")
    root.geometry("300x390")

    frame = MainFrame(root, rpc_client=rpc, username=user, on_logout=root.destroy)
    frame.pack(fill="both", expand=True, padx=8, pady=8)
    try:
        frame.log_msg(f"Admin conectado a {rpc.server_ip}:{rpc.server_port}")
    except Exception:
        pass

    root.mainloop()


if __name__ == "__main__":
    main()
