# api.py
import xmlrpc.client
PORT = "8000"
SERVER_URL = "http://localhost:" + PORT

def llamar_xmlrpc(metodo, *args):
    server = xmlrpc.client.ServerProxy(SERVER_URL)
    func = getattr(server, metodo)
    return func(*args)

