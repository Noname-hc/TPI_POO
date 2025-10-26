#include <iostream>

#include "../Logger/Logger.hpp"
#include "../../Libreria_RPC/XmlRpc.h"
#include "Reporte.h"

using namespace XmlRpc;

Reporte::Reporte(Logger *Log = nullptr, XmlRpcServer *S):XmlRpcServerMethod("Reporte", S){
    this->Log = Log;
}
Reporte::~Reporte(){
    if(this->Log != nullptr){
        this->Log->~Logger();
    }
}

// Se entregan dos enteros corrspondientes al enum de log domain y log level respectivamente
void Reporte::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result){
    bool un_param = false;

    if(params.size() == 0){
        result = Log->getMsj();
        return;

    }else if(params.size() == 1){
        if(params.getType() != XmlRpcValue::TypeInt){
            throw XmlRpc::XmlRpcException("error: Valor no entero");
        }
        un_param = true;

    }else if(params.getType() != XmlRpcValue::TypeArray){
        throw XmlRpc::XmlRpcException("Los parametros no conforman un arreglo");

    }else if(params[0].getType() != XmlRpcValue::TypeInt && params[1].getType() != XmlRpcValue::TypeInt){
        throw XmlRpc::XmlRpcException("Hay valores no enteros en los parametros");
    }
    Log->abrirLogger();
    

    if(un_param){
        int a = params;
        LogLevel lvl = static_cast<LogLevel>(a);
        result = Log->getMsj(lvl);

    }else{
        int a = params[0];
        if( !(a >= 0 && a <= 3)){
            throw XmlRpc::XmlRpcException("el primer parametro no esta en rango");
        }
        int b = params[1];
        if( !(b >= 0 && b <= 1)){
            throw XmlRpc::XmlRpcException("el segundo parametro no esta en rango");
        }

        LogDomain dom = static_cast<LogDomain>(a);
        LogLevel lvl = static_cast<LogLevel>(b);
        result = Log->getMsj(lvl, dom);
    }

    Log->~Logger();
}
std::string Reporte::help(){

}