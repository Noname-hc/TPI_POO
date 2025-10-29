import xmlrpc.client as RPC_XML
import socket

class RobotRPCClient:
    def __init__(self, server_ip="127.0.0.1", server_port="8000"):
        self.server_ip = server_ip
        self.server_port = server_port
        self.server = None
        # centralizar nombres de métodos para fácil edición
        self.methods = {
            "Inicio": "Inicio",            # rpc.login(user, password)
            "get_status": "get_status",  # rpc.get_status()
            "move_xyz": "move_xyz",      # rpc.move_xyz(x,y,z)
            "home": "home",              # rpc.home()
            "list_commands": "list_commands"  # rpc.list_commands()
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
        return self.call("Inicio", username, password)

    def get_status(self):
        return self.call("get_status")

    def move_xyz(self, x, y, z):
        return self.call("move_xyz", float(x), float(y), float(z))

    def home(self):
        return self.call("home")

    def list_commands(self):
        return self.call("list_commands")