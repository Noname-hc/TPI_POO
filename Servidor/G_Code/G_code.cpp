#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>

#include "G_code.h"
#include "../Logger/Logger.hpp"
#include "../../Libreria_RPC/XmlRpc.h"
#include "../Serial_Com/Serial_Com.h"

G_Code::G_Code(Logger *log, const int Nivel_de_Acceso, XmlRpcServer *S):XmlRpcServerMethod("G_Code",S){
    this->log = log;
    this->Nivel_de_Acceso = Nivel_de_Acceso;
}

G_Code::~G_Code(){
    if(log != nullptr){
        log->~Logger();
    }
    this->log = nullptr;
}
void G_Code::setLog(Logger *log){
    this->log = log;
}
void G_Code::setPath(const std::string &path){
    this->path = path;
}
void G_Code::setLvL(const int Nivel_de_Acceso){
    this->Nivel_de_Acceso = Nivel_de_Acceso;
}
//==============================================================================================
void G_Code::execute(XmlRpcValue& params, XmlRpcValue& result){ // params[0] es un entero params[1] es un string x,y,z (no hace falta poner si no hay que mover nada)
    result = "";

        switch (Nivel_de_Acceso)
    {
        case 0:
            try{
                this->log->log(LogLevel::ERROR, LogDomain::G_Code, "Falta Loguearse");       

            }catch(std::runtime_error &e){
                std::cout << e.what() << std::endl;
            }
            throw XmlRpc::XmlRpcException("Falta logearse");

        break;

        case 1:
            // Se puede acceder al metodo, no se hace nada
        break;

        case 2:
            // Se puede acceder al metodo, no se hace nada
        break;

        default:
            try{
                this->log->log(LogLevel::ERROR, LogDomain::G_Code, "Error al logearse");       

            }catch(std::runtime_error &e){
                std::cout << e.what() << std::endl;
            }
            throw XmlRpc::XmlRpcException("Error al logearse");
            
        break;
    }

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

    std::string str_aux = params[1];
    str_aux.erase(std::remove(str_aux.begin(), str_aux.end(), ' '), str_aux.end());// eliminamos los espacios

    std::string str_aux_G0 = "";
    std::vector<double> valores = {};

    int a = params[0];
    std::string posicion = str_aux;
    size_t pos = 0;

    if(a < 1000){ // Comandos G
        
        switch(a){
            case 0:
                str_aux = posicion;
                if(str_aux.size() == 0){
                    try{
                        log->log(LogLevel::ERROR, LogDomain::G_Code, "No se especifico posicion para G0");
                    } catch(std::runtime_error &e){
                        std::cout << e.what();
                    }
                    throw XmlRpc::XmlRpcException("No se especifico posicion para G0");
                }
                
                for(int i = 0; i<2 ;i++){
                    pos = str_aux.find(",");

                    if(pos == std::string::npos){
                        try{
                            log->log(LogLevel::ERROR, LogDomain::G_Code, "Posicion para G0 invalida");
                        } catch(std::runtime_error &e){
                            std::cout << e.what();
                        }
                        
                        throw XmlRpc::XmlRpcException("Posicion para G0 invalida");
                    }

                    str_aux_G0 = str_aux.substr(0,pos);
                    try{
                        valores.push_back(std::stod(str_aux_G0));

                    }catch(...){

                        try{
                            log->log(LogLevel::ERROR, LogDomain::G_Code, "Posicion para G0 invalida");
                        } catch(std::runtime_error &e){
                            std::cout << e.what();
                        }
                        throw XmlRpc::XmlRpcException("Posicion para G0 invalida");
                    }

                    str_aux_G0 = "";
                    str_aux = str_aux.substr(pos+1);
                }

                try{
                    valores.push_back(std::stod(str_aux));

                }catch(...){
                    try{
                        log->log(LogLevel::ERROR, LogDomain::G_Code, "Ultima posicion para G0 invalida");
                    } catch(std::runtime_error &e){
                        std::cout << e.what();
                    }
                    throw XmlRpc::XmlRpcException("Ultima posicion para G0 invalida");
                }

                str_aux = "";
                str_aux += "G0 X" + std::to_string(valores[0]);
                str_aux += " Y" + std::to_string(valores[1]);
                str_aux += " Z" + std::to_string(valores[2]);

                try{
                    log->log(LogLevel::INFO, LogDomain::G_Code, str_aux);
                } catch(std::runtime_error &e){
                    std::cout << e.what();
                }
                result = str_aux;

                if(path.size() != 0){
                    this->getFs() << str_aux << std::endl;
                }
            break;

            default:
                str_aux = "G" + std::to_string(a);

                try{
                    log->log(LogLevel::INFO, LogDomain::G_Code, str_aux);
                } catch(std::runtime_error &e){
                    std::cout << e.what();
                }
                result = str_aux;
            break;
        }

    }else if(a > 1000){ // Comandos M
        str_aux = std::to_string(a);
        str_aux = "M" + str_aux.substr(4);

        try{
            log->log(LogLevel::INFO, LogDomain::G_Code, str_aux);
        } catch(std::runtime_error &e){
            std::cout << e.what();
        }
        result=str_aux;

    }

    /*if(str_aux.size() != 0){
        char Buffer[128];
        Serial_Com Com;
        Com.T_R_Init(19600, 2, "/dev/ttyACM0");
        Com.ClearInput();

        Com.Transmision(str_aux);
        Com.Recepcion(Buffer, sizeof(Buffer), 3000);

        result = Buffer;
        Com.~Serial_Com();
    }else{
        try{
            this->log->log(LogLevel::ERROR, LogDomain::G_Code, "No se transmite nada");
        }catch(std::runtime_error &e){
            std::cout << e.what();
        }
    }*/
}

std::string G_Code::help(){
    return "uso: (int), string(x,y,z) ";
}

//=============================================================================================
//=============================================================================================

// Modo escritura y se escribe al final, la ruta es "TPI_POO/logs/trajs.txt"
void G_Code::openFile(){
    if(this->trajs.is_open()){
        trajs.close();
    }
    trajs.open(path, std::ios::out | std::ios::app);
}
void G_Code::closeF(){
    if(trajs.is_open()){
        trajs.close();
    }
}

std::fstream& G_Code::getFs(){
    if(!this->trajs.is_open()){
        this->openFile();
    }
    return this->trajs;
}
