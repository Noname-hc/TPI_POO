#include "Logger.hpp"
#include <iostream>

int main(){
    // Ruta relativa correcta desde Servidor/Logger hacia Servidor/logs/Logger.log
    const std::string path = "../logs/Logger.log";
    Logger Log(path);

    // Intentar escribir y reportar en stderr si falla
    try {
        Log.log(LogLevel::DEBUG, LogDomain::MAIN, "Probando rutas");
        std::cout << "Escrito en: " << path << std::endl;
    } catch (...) {
        std::cerr << "Fallo al escribir en: " << path << std::endl;
        return 1;
    }

    return 0;
}