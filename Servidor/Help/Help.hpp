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
            str_aux += "  0 -> main\n";
            str_aux += "  1 -> G_Code\n";
            str_aux += "  2 -> Reporte\n";
            str_aux += "  3 -> Inicio\n";
            result = str_aux;
        }else if(str == "Tarea" || str == "tarea"){
            str_aux = "Uso del metodo Tarea (RPC):\n";
            str_aux += "  ['add', nombre, linea]   -> agrega linea a una tarea.\n";
            str_aux += "  ['run', nombre]          -> ejecuta la tarea por serie.\n";
            str_aux += "  ['show', nombre]         -> devuelve el contenido de la tarea.\n";
            str_aux += "  ['list']                 -> lista las tareas.\n";
            str_aux += "  ['clear', nombre]        -> borra el contenido de la tarea.\n\n";
            str_aux += "Formato de linea: <ID><num> <param><value> <param><value> ...\n";
            str_aux += "  <ID> = 'G' o 'M'\n";
            str_aux += "  <num> = entero (1–3 digitos)\n";
            str_aux += "  <param> = letra en {X,Y,Z,E,F}\n";
            str_aux += "  <value> = numero entero o flotante con signo\n";
            result = str_aux;
        }
    }
};

#endif
