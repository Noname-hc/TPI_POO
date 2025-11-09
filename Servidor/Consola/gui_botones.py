#!/usr/bin/env python3
import sys
import tkinter as tk
from tkinter import messagebox, scrolledtext
import xmlrpc.client as RPC


def call_inicio(server, user, pwd):
    try:
        res = server.Inicio(user, pwd)
        if isinstance(res, str) and "Bienvenido" in res:
            return True, res
        return False, str(res)
    except Exception as e:
        return False, str(e)


def make_ui(server):
    root = tk.Tk()
    root.title("Control Robot - Botones G/M")

    # XYZ frame
    f = tk.LabelFrame(root, text="Movimiento (G0)")
    f.pack(fill="x", padx=6, pady=6)
    tk.Label(f, text="X:").grid(row=0, column=0)
    x = tk.Entry(f, width=8); x.insert(0, "0"); x.grid(row=0, column=1, padx=4)
    tk.Label(f, text="Y:").grid(row=0, column=2)
    y = tk.Entry(f, width=8); y.insert(0, "0"); y.grid(row=0, column=3, padx=4)
    tk.Label(f, text="Z:").grid(row=0, column=4)
    z = tk.Entry(f, width=8); z.insert(0, "0"); z.grid(row=0, column=5, padx=4)

    out = scrolledtext.ScrolledText(root, height=10)
    out.pack(fill="both", expand=True, padx=6, pady=6)

    # Barra de estado sencilla
    status = tk.Label(root, text="Listo", anchor="w", fg="gray")
    status.pack(fill="x", padx=6, pady=(0,6))

    def log(msg):
        out.insert("end", msg + "\n")
        out.see("end")

    def send_a(a, extra=""):
        try:
            status.config(text="Enviando comando...", fg="orange")
            root.update_idletasks()
            # El servidor expone G_Code(int, string): enviar dos parámetros.
            res = server.G_Code(a, extra)
            log(str(res))
            status.config(text="Comando enviado correctamente", fg="green")
        except Exception as e:
            messagebox.showerror("RPC", str(e))
            status.config(text=f"Error al enviar: {e}", fg="red")

    # Botones superiores
    top = tk.Frame(root); top.pack(fill="x", padx=6)
    tk.Button(top, text="G90 Abs", command=lambda: send_a(90, "")).pack(side="left", padx=2, pady=4)
    tk.Button(top, text="G91 Rel", command=lambda: send_a(91, "")).pack(side="left", padx=2)
    tk.Button(top, text="G92 Set0", command=lambda: send_a(92, "")).pack(side="left", padx=2)
    tk.Button(top, text="G28 Home", command=lambda: send_a(28, "")).pack(side="left", padx=2)

    # Movimiento lineal (G0 con XYZ)
    tk.Button(f, text="Mover", command=lambda: send_a(0, f"{x.get()},{y.get()},{z.get()}"))\
        .grid(row=0, column=6, padx=8)

    # Línea de botones M-codes
    m = tk.Frame(root); m.pack(fill="x", padx=6)
    tk.Button(m, text="Bomba ON", command=lambda: send_a(10001, "")).pack(side="left", padx=2, pady=2)
    tk.Button(m, text="Bomba OFF", command=lambda: send_a(10002, "")).pack(side="left", padx=2)
    tk.Button(m, text="Griper ON", command=lambda: send_a(10003, "")).pack(side="left", padx=2)
    tk.Button(m, text="Griper OFF", command=lambda: send_a(10005, "")).pack(side="left", padx=2)
    tk.Button(m, text="Laser ON", command=lambda: send_a(10006, "")).pack(side="left", padx=2)
    tk.Button(m, text="Laser OFF", command=lambda: send_a(10007, "")).pack(side="left", padx=2)
    tk.Button(m, text="Motor ON", command=lambda: send_a(100017, "")).pack(side="left", padx=2)
    tk.Button(m, text="Motor OFF", command=lambda: send_a(100018, "")).pack(side="left", padx=2)
    tk.Button(m, text="Fan ON", command=lambda: send_a(1000106, "")).pack(side="left", padx=2)
    tk.Button(m, text="Fan OFF", command=lambda: send_a(1000107, "")).pack(side="left", padx=2)
    tk.Button(m, text="Reporte (M114)", command=lambda: send_a(1000114, "")).pack(side="left", padx=2)
    tk.Button(m, text="Finales (M119)", command=lambda: send_a(1000119, "")).pack(side="left", padx=2)

    return root


def main():
    if len(sys.argv) < 5:
        print("Uso: gui_botones.py HOST PORT USER PASS")
        sys.exit(2)
    host = sys.argv[1]; port = int(sys.argv[2]); user = sys.argv[3]; pwd = sys.argv[4]
    url = f"http://{host}:{port}"
    server = RPC.ServerProxy(url, allow_none=True)

    ok, msg = call_inicio(server, user, pwd)
    if not ok:
        messagebox.showerror("Login", f"Login fallido: {msg}")
        sys.exit(1)

    root = make_ui(server)
    root.mainloop()


if __name__ == '__main__':
    main()
