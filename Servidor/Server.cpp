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
    G_Code g_code(&Log,&S);
    g_code.setPath("logs/Trayectorias.log");
    g_code.openFile();
    // XmlRpc::setVerbosity(5);

    // Se crea un socket de servidor sobre el puerto indicado
    S.bindAndListen(port);

    XmlRpcSocket sock;
    bool eof = true;
    std::string Mensaje_inicial = "";

    if(sock.nbRead(S.getfd(), Mensaje_inicial, &eof)){
        Log.log(LogLevel::DEBUG, LogDomain::MAIN, "Mensaje inicial: " + Mensaje_inicial);

        std::string str_aux = "";
        size_t pos = Mensaje_inicial.find("<Permisos>");

        str_aux = Mensaje_inicial.substr(pos+11, (Mensaje_inicial.find("</Permisos>") - (pos+11)) );
        Log.log(LogLevel::DEBUG, LogDomain::MAIN, "str_aux: " + str_aux);

        if(str_aux == "user"){
            Log.log(LogLevel::INFO, LogDomain::MAIN, "Logeado como user");

        }else if(str_aux == "admin"){
            Reporte reporte(&Log, &S);
            Log.log(LogLevel::INFO, LogDomain::MAIN, "Logeado como admin");
        }

    }else{
        Log.log(LogLevel::ERROR, LogDomain::MAIN, "El mensaje inicial no se recibio correctamente");
    }

    // Enable introspection
    S.enableIntrospection(true);
    
    // A la escucha de requerimientos
    S.work(-1.0);
    g_code.closeF();

    return 0;
}