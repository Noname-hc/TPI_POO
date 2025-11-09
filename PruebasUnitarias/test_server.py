#!/usr/bin/env python3
"""
Pruebas unitarias básicas para Servidor/Server.cpp

Qué valida:
- Compilación opcional del servidor (si no existe el binario o si se exporta BUILD_SERVER=1)
- Arranque del servidor en un puerto de prueba
- XML-RPC: system.listMethods, Inicio (credenciales inválidas y válidas),
  permisos en Reporte (requiere admin) y una llamada a G_Code (no asume hardware serie)

Ejecutar:
  python3 PruebasUnitarias/test_server.py

Variables de entorno opcionales:
  TEST_PORT=8005          # puerto a usar
  BUILD_SERVER=1          # fuerza compilación con make en Servidor/
  SERVER_EXE=...          # ruta al ejecutable si no es Servidor/Server.exe
"""

import os
import sys
import time
import signal
import subprocess
import unittest
import contextlib

try:
    import xmlrpc.client as xmlrpc
except Exception as e:
    print("Falta xmlrpc.client (Python3)", e)
    raise


ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SERVER_DIR = os.path.join(ROOT_DIR, "Servidor")
DEFAULT_EXE = os.path.join(SERVER_DIR, "Server.exe")
TEST_PORT = int(os.environ.get("TEST_PORT", "8001"))


def ensure_built():
    exe = os.environ.get("SERVER_EXE", DEFAULT_EXE)
    if os.path.exists(exe) and not os.environ.get("BUILD_SERVER"):
        return exe
    # Compilar con make en Servidor/
    print("[build] Compilando servidor con make en Servidor/ ...")
    subprocess.run(["make"], cwd=SERVER_DIR, check=True)
    if not os.path.exists(exe):
        raise FileNotFoundError(f"No se generó el ejecutable esperado: {exe}")
    return exe


@contextlib.contextmanager
def run_server(port: int):
    exe = ensure_built()
    print(f"[run] Lanzando servidor: {exe} puerto {port}")
    proc = subprocess.Popen([exe, str(port)], cwd=SERVER_DIR,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            text=True)
    try:
        # Esperar disponibilidad: intentos a system.listMethods
        url = f"http://127.0.0.1:{port}"
        proxy = xmlrpc.ServerProxy(url, allow_none=True)
        start = time.time()
        last_err = None
        while time.time() - start < 15.0:
            try:
                methods = proxy.system.listMethods()
                if isinstance(methods, list) and methods:
                    print("[wait] Servidor listo.")
                    break
            except Exception as e:
                last_err = e
                time.sleep(0.3)
        else:
            raise RuntimeError(f"Servidor no respondió a tiempo: {last_err}")
        yield proxy
    finally:
        print("[stop] Terminando servidor...")
        if proc.poll() is None:
            try:
                proc.terminate()
                try:
                    proc.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    proc.kill()
            except Exception:
                pass
        # Mostrar últimas líneas de salida para debug si falló algo
        try:
            out = proc.stdout.read() if proc.stdout else ""
            if out:
                print("[server output]\n" + out)
        except Exception:
            pass


class TestServerRPC(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.port = TEST_PORT
        # Arrancamos una instancia por clase para acelerar
        cls.ctx = run_server(cls.port)
        cls.proxy = cls.ctx.__enter__()

    @classmethod
    def tearDownClass(cls):
        try:
            cls.ctx.__exit__(None, None, None)
        except Exception:
            pass

    def test_01_list_methods(self):
        methods = self.proxy.system.listMethods()
        self.assertIn("Inicio", methods)
        self.assertIn("G_Code", methods)
        self.assertIn("Reporte", methods)
        # Introspección activada en Server.cpp
        self.assertIn("system.listMethods", methods)
        self.assertIn("system.methodHelp", methods)

    def test_02_login_invalido(self):
        # Debe devolver un string con el mensaje de error
        res = self.proxy.Inicio("foo", "bar")
        self.assertIsInstance(res, str)
        self.assertTrue("invalid" in res.lower() or "invalida" in res.lower())

    def test_03_login_valido_usuario(self):
        res = self.proxy.Inicio("Nacho", "123")
        self.assertIsInstance(res, str)
        self.assertTrue("bienvenido" in res.lower())
        # Ahora G_Code debería permitir ejecutar (nivel 1)
        # Usar un comando que no requiera parámetros: G90 (C_Absolutas)
        # API espera [int, string]
        out = self.proxy.G_Code([90, ""])  # G90
        self.assertIsInstance(out, str)

    def test_04_reporte_requiere_admin(self):
        # Después del login de usuario, Reporte debería fallar por permisos
        with self.assertRaises(Exception):
            _ = self.proxy.Reporte([])

    def test_05_login_admin_y_reporte_ok(self):
        res = self.proxy.Inicio("Nico", "777")
        self.assertIn("Bienvenido", res)
        # Sin parámetros: debe devolver todo el log como string
        log_all = self.proxy.Reporte([])
        self.assertIsInstance(log_all, str)
        # Filtro por nivel (ej: INFO=0)
        log_info = self.proxy.Reporte([0])
        self.assertIsInstance(log_info, str)


if __name__ == "__main__":
    # Tip: export BUILD_SERVER=1 para forzar compilación
    unittest.main(verbosity=2)
