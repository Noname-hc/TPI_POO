#include <iostream>
#include <cstring>

#include "../Libreria_RPC/XmlRpc.h"
#include "../Libreria_RPC/XmlRpcSocket.h"
#include "../Servidor/G_Code/G_code.h"
#include "../Servidor/Logger/Logger.hpp"
#include "Reporte/Reporte.h"

int main(int argc, char* argv[]){

    if (argc != 2) {
        std::cerr << "Uso: miserver N_Port\n";
        return 1;
    }
    int port = atoi(argv[1]);

    // S es el servidor
    XmlRpcServer S;
    Logger Log("logs/Logger.log");
    // Registro de metodos en el servidor
    // mediante el uso del constructor heredado.
    Reporte reporte(&Log, &S);
    G_Code g_code(&Log,&S);
    g_code.setPath("logs/Trayectorias.log");
    g_code.openFile();
    // XmlRpc::setVerbosity(5);

    // Se crea un socket de servidor sobre el puerto indicado
    S.bindAndListen(port);

    XmlRpcSocket sock;
    bool eof = true;
    std::string Mensaje_inicial = "";
    sock.nbRead(S.getfd(), Mensaje_inicial, &eof);

    std::cout << "Mensaje inicial: " << std::endl << Mensaje_inicial;
    // La idea es que el mensaje inicial especifique nivel de acceso y con eso asociemos mas o menos funciones a S


    // Enable introspection
    S.enableIntrospection(true);
    
    // A la escucha de requerimientos
    S.work(-1.0);
    g_code.closeF();

    return 0;
}