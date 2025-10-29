#ifndef _Reporte_H
#define _Reporte_H

#include <iostream>

#include "../Logger/Logger.hpp"
#include "../../Libreria_RPC/XmlRpc.h"


class Reporte: public XmlRpc::XmlRpcServerMethod{
    public:
        Reporte(Logger *Log, const int Nivel_de_Acceso, XmlRpc::XmlRpcServer *S);
        ~Reporte();
        
        void setLvL(const int Nivel_de_Acceso);
        void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);
        std::string help();

    private:
        Logger *Log;
        int Nivel_de_Acceso = 0;

};

#endif