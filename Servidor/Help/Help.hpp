#ifndef _HELP_
#define _HELP_

#include <iostream>
#include "../../Libreria_RPC/XmlRpc.h"

using namespace XmlRpc;
class Help:XmlRpcServerMethod{

public:
    Help(XmlRpc::XmlRpcServer *S):XmlRpcServerMethod("Help",S){}
    ~Help(){}

    void execute(XmlRpcValue& params, XmlRpcValue& result){
        std::string str = params[0];
        std::string str_aux = "";

        if(str == "G_Code" || str == "gcode" || str == "g_code" || str == "G_code"){
            result = "params[0] es un entero params[1] es un string x,y,z (no hace falta poner si no hay que mover nada)";
        
        }else if(str == "Reporte" || str == "reporte"){
            str_aux = "Se puede filtrar por 1, 2 o ningun parametro\n";
            str_aux += "  El primero es el nivel\n";
            str_aux += "  0 -> INFO\n";
            str_aux += "  1 -> WARNING\n";
            str_aux += "  2 -> ERROR\n";
            str_aux += "  3 -> DEBUG\n\n";
            str_aux += "  El segundo es el dominio al cual acceder:\n";
            str_aux += "  0 -> main";
            str_aux += "  1 -> G_Code";
            str_aux += "  2 -> Reporte";
            str_aux += "  3 -> Inicio";
            result = str_aux;
        }
    }
};

#endif