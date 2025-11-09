import xmlrpc.client as RPC_XML
import socket

class RobotRPCClient:
    def __init__(self, server_ip="192.168.100.37", server_port="8001"):
        self.server_ip = server_ip
        self.server_port = server_port
        self.server = None
        # centralizar nombres de métodos para fácil edición
        self.methods = {
            "login": "Inicio",            # rpc.login(user, password)
            "get_status": "get_status",  # rpc.get_status()
            "move_xyz": "G_Code",   # rpc.G_Code([4,"x,y,z"])
            "home": "home",              # rpc.home()
            "list_commands": "list_commands",  # rpc.list_commands()
            "help_move": "G_Code_help",   #lista de comandos
            "reporte": "Reporte"
                }

    def _ensure_proxy(self):
        if self.server is None:
            url = f"http://{self.server_ip}:{self.server_port}"
            # crear proxy
            try:
                self.server = RPC_XML.ServerProxy(url, allow_none=True)
            except Exception as e:
                raise ConnectionError(f"Error creando ServerProxy: {e}")

    # Conectar explícito (por si se quiere testear antes del login)
    def connect(self, server_ip=None, server_port=None, timeout=5):
        if server_ip:
            self.server_ip = server_ip
        if server_port:
            self.server_port = server_port
        # Test de conexión simple: create proxy and call a dummy (si existe)
        self._ensure_proxy()
        # no hacemos llamada forzada para no depender de método en servidor
        return True

    def call(self, method_name, *args, **kwargs):
        """
        Método genérico para llamar a RPC. Captura excepciones y las re-lanza.
        """
        self._ensure_proxy()
        if method_name not in self.methods:
            raise AttributeError(f"Método '{method_name}' no definido en métodos RPC locales.")
        remote_name = self.methods[method_name]
        try:
            func = getattr(self.server, remote_name)
        except AttributeError:
            raise AttributeError(f"El servidor no expone el método '{remote_name}'")
        try:
            return func(*args)
        except socket.error as se:
            # invalid socket / network error
            raise ConnectionError(f"Error de red al llamar {remote_name}: {se}")
        except Exception as e:
            raise RuntimeError(f"Error en llamada RPC {remote_name}: {e}")

    # Wrappers específicos (facilitan el uso en UI, pero podés llamarlos por call())
    def login(self, username, password):
        return self.call("login", username, password)

    def get_status(self):
        return self.call("get_status")

    def move_xyz(self, x, y, z):
        return self.call("move_xyz", float(x), float(y), float(z))

    def home(self):
        return self.call("home")

    def list_commands(self):
        return self.call("list_commands")

    def help_move(self):
        comandos = {
            "Movimiento_Lineal": 0,
            "Rampa": 4,
            "Homing": 28,
            "C_Absolutas": 90,
            "C_Relativas": 91,
            "Set_0": 92,
            "Bomba_ON": 10001,
            "Bomba_OFF": 10002,
            "Griper_ON": 10003,
            "Griper_OFF": 10005,
            "Laser_ON": 10006,
            "Laser_OFF": 10007,
            "Motor_ON": 100017,
            "Motor_OFF": 100018,
            "Ventiladores_ON": 1000106,
            "Ventiladores_OFF": 1000107,
            "Reporte_General": 1000114,
            "Reporte_finales": 1000119
        }
        texto = "Comandos disponibles"

        for nombre, codigo in comandos.items():
            texto += f"{nombre:20s} = {codigo}\n"
        return texto
        
    def reporte(self):
        return self.call("reporte")