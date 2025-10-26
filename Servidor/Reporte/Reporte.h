#ifndef _Reporte_H
#define _Reporte_H

#include <iostream>

#include "../Logger/Logger.hpp"
#include "../../Libreria_RPC/XmlRpc.h"


class Reporte: public XmlRpc::XmlRpcServerMethod{
    public:
        Reporte(Logger *Log = nullptr, XmlRpc::XmlRpcServer *S);
        ~Reporte();
        void setLog(Logger *Log);
        void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);
        std::string help();

    private:
        Logger *Log;

};

#endif