#include <iostream>

#include "Inicio.h"
#include "../../Libreria_RPC/XmlRpc.h"
#include "../G_Code/G_code.h"
#include "../Reporte/Reporte.h"
#include "../Logger/Logger.hpp"

using namespace XmlRpc;



Inicio::Inicio(const int Nivel_de_Acceso, Logger* Log, XmlRpcServer *S):XmlRpcServerMethod("Inicio" ,S){
    this->Nivel_de_Acceso = Nivel_de_Acceso;
}

Inicio::~Inicio(){

}

void Inicio::setMethod(Reporte *Repo, G_Code *gcode){
    this->Repo = Repo;
    this->gcode = gcode;
}

void Inicio::execute(XmlRpcValue& params, XmlRpcValue& result){
    std::string usuario = params[0];
    std::string contraseña = params[0];

    bool valido = false;

    if(usuario == "Nacho" && contraseña == "123"){
        valido == true;
    }


    if(valido){
        result = true; 

        if(this->Repo == nullptr || this->gcode == nullptr){
            throw std::runtime_error("Falta inicializar los objetos");
        
            try{
            Log->log(LogLevel::ERROR, LogDomain::Inicio, "Falta inicializar los objetos");       

            }catch(std::runtime_error &e){
                std::cout << e.what() << std::endl;
            }
        }else{
            Repo->setLvL(2);
            gcode->setLvL(2);
        }

    }
}