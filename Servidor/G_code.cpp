#include <cstring>
#include <iostream>
#include <vector>

#include "G_code.h"
#include "../Servidor/Logger/Logger.hpp"

G_Code::G_Code(Logger *log){
    this->log = log;
}

G_Code::~G_Code(){
    this->log = nullptr;
}
void G_Code::setLog(Logger *log){
    this->log = log;
}
//==============================================================================================

std::string G_Code::Interpretar(comandos cmnd, const std::string &posicion){ // posicion es x,y,z (no hace falta poner si no hay que mover nada)

    log->abrirLogger();
// como lo que pasamos es la clase comandos, no tenemos que hacer muchas verificaciones de errores
    std::string str_aux = "";
    std::string str_aux_G0 = "";
    std::vector<double> valores = {};

    int a = (int)cmnd;
    size_t pos = 0;
    if(a < 1000){ // Comandos G
        
        switch(a){
            case 0:
                str_aux = posicion;
                if(str_aux.size() == 0){
                    log->log(LogLevel::ERROR, LogDomain::G_Code, "No se especifico posicion para G0");
                    throw "No se especifico posicion para G0";
                }
                
                for(int i = 0; i<2 ;i++){
                    pos = str_aux.find(",");

                    if(pos == std::string::npos){
                        log->log(LogLevel::ERROR, LogDomain::G_Code, "Posicion para G0 invalida");
                        throw "Posicion para G0 invalida";
                    }

                    str_aux_G0 = str_aux.substr(0,pos);
                    try{
                        valores.push_back(std::stod(str_aux_G0));

                    }catch(...){
                        log->log(LogLevel::ERROR, LogDomain::G_Code, "Posicion para G0 invalida");
                        throw "Posicion para G0 invalida";
                    }

                    str_aux_G0 = "";
                    str_aux = str_aux.substr(pos+1);
                }

                try{
                    valores.push_back(std::stod(str_aux));

                }catch(...){
                    log->log(LogLevel::ERROR, LogDomain::G_Code, "Ultima posicion para G0 invalida");
                    throw "Ultima posicion para G0 invalida";
                }

                str_aux = "";
                str_aux += "G0 [X:" + std::to_string(valores[0]);
                str_aux += " Y:" + std::to_string(valores[1]);
                str_aux += " Z:" + std::to_string(valores[2]) + "]";

                /*
                str_aux += "G0 " + std::to_string(valores[0]);
                str_aux += " " + std::to_string(valores[1]);
                str_aux += " " + std::to_string(valores[2]);
                */
                log->log(LogLevel::INFO, LogDomain::G_Code, str_aux);
                return str_aux;

            break;

            default:
                str_aux = "G" + std::to_string((int)cmnd);
                this->log->log(LogLevel::INFO, LogDomain::G_Code, str_aux);

                return str_aux;
            break;
        }

    }else if((int)cmnd > 1000){ // Comandos M
        str_aux = std::to_string((int)cmnd);
        str_aux = "M" + str_aux.substr(4);

        this->log->log(LogLevel::INFO, LogDomain::G_Code, str_aux);
        return str_aux;
    }

    return "error";
}

