#include <iostream>
#include <cstring>

#include "../Libreria_RPC/XmlRpc.h"
#include "../Libreria_RPC/XmlRpcSocket.h"
#include "../Servidor/G_Code/G_code.h"
#include "../Servidor/Logger/Logger.hpp"
#include "Inicio/Inicio.h"
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
    G_Code g_code(&Log, 0,&S);
    g_code.setPath("logs/Trayectorias.log");
    g_code.openFile();

    Reporte Repo(&Log,0,&S);
    Inicio Init(0, &Log, &S);
    Init.setMethod(&Repo, &g_code);
    XmlRpc::setVerbosity(5);

    // Se crea un socket de servidor sobre el puerto indicado
    S.bindAndListen(port);

    // Enable introspection
    S.enableIntrospection(true);
    
    // A la escucha de requerimientos
    S.work(-1.0);

    return 0;
}