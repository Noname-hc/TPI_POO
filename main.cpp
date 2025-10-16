#include <iostream>
#include "Servidor/Logger/Logger.hpp"
#include "Servidor/G_code.h"
#include <cstring>

int main(int argc, char *argv[]){

    Logger &log = Logger::getInstance();

    G_Code G;
    G.setLog(&log); 

    std::cout << G.Interpretar(comandos::Bomba_OFF) << std::endl;
    std::cout << G.Interpretar(comandos::Laser_OFF) << std::endl;
    std::cout << G.Interpretar(comandos::Movimiento_Lineal,"2.43,123.43,0.231") << std::endl;
    std::cout << G.Interpretar(comandos::Reporte_finales) << std::endl;

    return 0;
}
