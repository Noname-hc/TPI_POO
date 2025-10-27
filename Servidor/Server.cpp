#include <iostream>
#include <cstring>

#include "../Libreria_RPC/XmlRpc.h"
#include "../Libreria_RPC/XmlRpcSocket.h"
#include "../Servidor/G_Code/G_code.h"
#include "../Servidor/Logger/Logger.hpp"

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
    XmlRpc::setVerbosity(5);

    // Se crea un socket de servidor sobre el puerto indicado
    S.bindAndListen(port);

    // Enable introspection
    S.enableIntrospection(true);

    // A la escucha de requerimientos
    S.work(-1.0);
    g_code.closeF();

    /* 
    std::string mensaje_crudo ="";
    bool eof;
    int fd = S.getfd();
    XmlRpc::XmlRpcSocket::nbRead(fd, mensaje_crudo, &eof);

    size_t pos = 0;
    pos = mensaje_crudo.find("<lvl>");

    if(pos != std::string::npos){
        std::cout << mensaje_crudo.substr(pos+1+5, mensaje_crudo.find("</lvl>")-(pos+1+5)) << std::endl;
    }
    */

    return 0;
}