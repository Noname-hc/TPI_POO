#include <iostream>
#include <cstring>

#include "../Libreria_RPC/XmlRpc.h"
#include "../Servidor/G_Code/G_code.h"
#include "../Servidor/Logger/Logger.hpp"

int main(int argc, char* argv[]){

    if (argc != 1) {
        std::cerr << "Uso: N_Port\n";
        return 1;
    }
    int port = atoi(argv[1]);

    // S es el servidor
    XmlRpcServer S;
    Logger &log = Logger::getInstance();
    // Registro de metodos en el servidor
    // mediante el uso del constructor heredado.
    G_Code g_code(&log,&S);

    XmlRpc::setVerbosity(5);

    // Se crea un socket de servidor sobre el puerto indicado
    S.bindAndListen(port);

    // Enable introspection
    S.enableIntrospection(true);

    // A la escucha de requerimientos
    S.work(-1.0);

    return 0;
}