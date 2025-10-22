#include <cstring>
#include <iostream>
#include <vector>

#include "G_code.h"
#include "../Logger/Logger.hpp"
#include "../../Libreria_RPC/XmlRpc.h"

G_Code::G_Code(Logger *log, XmlRpcServer *S):XmlRpcServerMethod("G_Code",S){
    this->log = log;
}

G_Code::~G_Code(){
    this->log = nullptr;
}
void G_Code::setLog(Logger *log){
    this->log = log;
}
//==============================================================================================
void G_Code::execute(XmlRpcValue& params, XmlRpcValue& result){ // params[0] es un entero params[1] es un string x,y,z (no hace falta poner si no hay que mover nada)
    log->abrirLogger();

    if(!params.valid()){
        log->log(LogLevel::ERROR, LogDomain::G_Code, "Los parametros no son de ningun tipo");
        throw XmlRpc::XmlRpcException("Los parametros no son de ningun tipo");

    }else if(params.getType() != XmlRpcValue::TypeArray){ // Verificamos si es arreglo

        if(params.getType() != XmlRpcValue::TypeInt){ // verificamos si es entero ya que no es arreglo
            log->log(LogLevel::ERROR, LogDomain::G_Code, "Se debe pasar un arreglo o un entero");
            throw XmlRpc::XmlRpcException("Se debe pasar un arreglo o un entero");
        }
        // si llego hasta aca es porque es entero

    }else if(params.size() != 2){ // si es arreglo el tamaño debe ser igual a 2
        log->log(LogLevel::ERROR, LogDomain::G_Code, "Parametros esperados para array = 2");
        throw XmlRpc::XmlRpcException("Parametros esperados para array = 2");

    }else if(params[0].getType() != XmlRpcValue::TypeInt || params[1].getType() != XmlRpcValue::TypeString){ // verificamos que el arreglo contenga int, string
        log->log(LogLevel::ERROR, LogDomain::G_Code, "parametros debe contener entero, string");
        throw XmlRpc::XmlRpcException("parametros debe contener entero, string");
    }

    std::string str_aux = "";
    std::string str_aux_G0 = "";
    std::vector<double> valores = {};

    int a = params[0];
    std::string posicion = params[1];
    size_t pos = 0;

    if(a < 1000){ // Comandos G
        
        switch(a){
            case 0:
                str_aux = posicion;
                if(str_aux.size() == 0){
                    log->log(LogLevel::ERROR, LogDomain::G_Code, "No se especifico posicion para G0");
                    throw XmlRpc::XmlRpcException("No se especifico posicion para G0");
                }
                
                for(int i = 0; i<2 ;i++){
                    pos = str_aux.find(",");

                    if(pos == std::string::npos){
                        log->log(LogLevel::ERROR, LogDomain::G_Code, "Posicion para G0 invalida");
                        throw XmlRpc::XmlRpcException("Posicion para G0 invalida");
                    }

                    str_aux_G0 = str_aux.substr(0,pos);
                    try{
                        valores.push_back(std::stod(str_aux_G0));

                    }catch(...){
                        log->log(LogLevel::ERROR, LogDomain::G_Code, "Posicion para G0 invalida");
                        throw XmlRpc::XmlRpcException("Posicion para G0 invalida");
                    }

                    str_aux_G0 = "";
                    str_aux = str_aux.substr(pos+1);
                }

                try{
                    valores.push_back(std::stod(str_aux));

                }catch(...){
                    log->log(LogLevel::ERROR, LogDomain::G_Code, "Ultima posicion para G0 invalida");
                    throw XmlRpc::XmlRpcException("Ultima posicion para G0 invalida");
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
                result = str_aux;

            break;

            default:
                str_aux = "G" + std::to_string(a);
                this->log->log(LogLevel::INFO, LogDomain::G_Code, str_aux);

                result = str_aux;
            break;
        }

    }else if(a > 1000){ // Comandos M
        str_aux = std::to_string(a);
        str_aux = "M" + str_aux.substr(4);

        this->log->log(LogLevel::INFO, LogDomain::G_Code, str_aux);
        result = str_aux;
    }
    result = "";
}

std::string G_Code::help(){
    return "uso: (int), string(x,y,z) ";
}