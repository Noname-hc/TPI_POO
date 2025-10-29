/* test_client.cpp
   Cliente de prueba para el servidor del TP.
   - Envía un mensaje inicial: <Permisos>user</Permisos>
   - Luego llama al método remoto "G_Code" por XML-RPC.
   - Interactivo: permite escribir comandos desde stdin, incluye 'help'.
   Uso: ./test_client HOST PORT
*/

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <unistd.h>     // close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "../../Libreria_RPC/XmlRpcClient.h"
#include "../../Libreria_RPC/XmlRpcSocket.h"
#include "../../Servidor/G_Code/G_code.h"

using namespace std;
using namespace XmlRpc;

static void printHelp() {
    cout << "Comandos disponibles:\n";
    cout << "  help               : muestra esta ayuda\n";
    cout << "  gcode <param>      : llama al metodo remoto G_Code con un parametro (ej: gcode M104)\n";
    cout << "  exit               : cierra el cliente\n";
    cout << "\nEjemplo de uso:\n";
    cout << "  gcode M101\n\n";
    cout << "Este cliente establece la conexion TCP, envia el mensaje inicial\n";
    cout << "  <Permisos>user</Permisos>\n";
    cout << "y luego reutiliza el mismo socket para hacer llamadas XML-RPC.\n";
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " HOST PORT\n";
        return 1;
    }

    const char* host = argv[1];
    int port = atoi(argv[2]);

    // Crear socket POSIX
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    // Usamos la utilidad de la libreria para conectar (devuelve true/false)
    string shost(host);
    if (!XmlRpcSocket::connect(fd, shost, port)) {
        cerr << "Error: no se pudo conectar a " << host << ":" << port << " -> " << XmlRpcSocket::getErrorMsg() << "\n";
        ::close(fd);
        return 1;
    }
    cout << "Conectado a " << host << ":" << port << " (fd=" << fd << ")\n";

    // Creamos el cliente XML-RPC y le asignamos el fd ya conectado.
    XmlRpcClient client(host, port);
    client.setfd(fd); // reutilizamos el fd que ya conectamos
    // NOTA: al setear el fd evitamos que el cliente abra otra conexion distinta; 
    // asi el servidor recibira primero nuestro mensaje inicial y luego las llamadas RPC.

    // Mensaje inicial requerido por el servidor
    string mensaje_inicial = "<Permisos>user</Permisos>";
    int bytesWritten = 0;
    if (!XmlRpcSocket::nbWrite(fd, mensaje_inicial, &bytesWritten)) {
        cerr << "Error al enviar mensaje inicial: " << XmlRpcSocket::getErrorMsg() << "\n";
        client.close();
        return 1;
    }
    cout << "Mensaje inicial enviado: " << mensaje_inicial << "\n";

    // Interfaz interactiva
    printHelp();
    string line;
    while (true) {
        cout << "> ";
        if (!std::getline(cin, line)) break;
        if (line.size() == 0) continue;

        // parse simple commands
        if (line == "help") {
            printHelp();
            continue;
        } else if (line == "exit" || line == "quit") {
            break;
        } else if (line.rfind("gcode", 0) == 0) {
            // obtener parametro despues de 'gcode'
            string param;
            size_t pos = line.find(' ');
            if (pos != string::npos) param = line.substr(pos+1);
            // Construir parametro para el metodo: G_Code espera un arreglo con 2 elementos segun ejemplo
            XmlRpcValue parametros;
            parametros.setSize(2);
            parametros[0] = "G_CODE"; // nombre o tipo de peticion; ajustar si su servidor espera otra cosa
            parametros[1] = param;
            XmlRpcValue result;
            cout << "Llamando a G_Code con parametro: '" << param << "'\n";
            if (client.execute("G_Code", parametros, result)) {
                if (client.isFault()) {
                    cout << "Respuesta de fallo: " << result << "\n";
                } else {
                    cout << "Resultado: " << result << "\n";
                }
            } else {
                cout << "Error en la llamada a 'G_Code'\n";
            }
            continue;
        } else {
            cout << "Comando no reconocido. Escriba 'help' para ver opciones.\n";
        }
    }

    cout << "Cerrando cliente...\n";
    client.close();
    // Si setfd transfirio la propiedad, close() deberia cerrar fd; por si acaso:
    ::close(fd);
    return 0;
}