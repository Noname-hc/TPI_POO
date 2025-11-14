#include <iostream>

#include "../Logger/Logger.hpp"
#include "../../Libreria_RPC/XmlRpc.h"
#include "Reporte.h"

using namespace XmlRpc;

Reporte::Reporte(Logger *Log, const int Nivel_de_Acceso, XmlRpcServer *S):XmlRpcServerMethod("Reporte", S){
    this->Log = Log;
    this->Nivel_de_Acceso = Nivel_de_Acceso;
}
Reporte::~Reporte(){
    if(this->Log != nullptr){
        this->Log->~Logger();
    }
}
void Reporte::setLvL(const int Nivel_de_Acceso){
    this->Nivel_de_Acceso = Nivel_de_Acceso;
}

// Se entregan dos enteros corrspondientes al enum de log domain y log level respectivamente
/*void Reporte::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result){
    bool un_param = false;
    
    switch (Nivel_de_Acceso)
    {
        case 0:
        try{
            Log->log(LogLevel::ERROR, LogDomain::Reporte, "Falta logearse");

        }catch(std::runtime_error &e){
            std::cout << e.what() << std::endl;
        }
            throw XmlRpc::XmlRpcException("Falta logearse");
        break;

        case 1:
            // Un usuario no puede acceder a este metodo
            return;
        break;

        case 2:
            // Se puede acceder al metodo, no se hace nada
        break;

        default:
            try{
                Log->log(LogLevel::ERROR, LogDomain::Reporte, "Error al logearse");

            }catch(std::runtime_error &e){
                std::cout << e.what() << std::endl;
            }

            throw XmlRpc::XmlRpcException("Error al logearse");

        break;
    }

    if(params.size() == 0 || (int)params == 0){
        result = Log->getMsj();
        return;

    }else if(params.size() == 1){
        if(params.getType() != XmlRpcValue::TypeInt){
            Log->log(LogLevel::ERROR, LogDomain::Reporte, "error: Valor no entero");
            throw XmlRpc::XmlRpcException("error: Valor no entero");
        }
        un_param = true;

    }else if(params.getType() != XmlRpcValue::TypeArray){
        Log->log(LogLevel::ERROR, LogDomain::Reporte, "Los parametros no conforman un arreglo");
        throw XmlRpc::XmlRpcException("Los parametros no conforman un arreglo");

    }else if(params[0].getType() != XmlRpcValue::TypeInt && params[1].getType() != XmlRpcValue::TypeInt){
        Log->log(LogLevel::ERROR, LogDomain::Reporte, "Hay valores no enteros en los parametros");
        throw XmlRpc::XmlRpcException("Hay valores no enteros en los parametros");
    }
    Log->abrirLogger();
    

    int a = params[0];
    if( !(a >= 0 && a <= 3)){
        Log->log(LogLevel::ERROR, LogDomain::Reporte, "el primer parametro no esta en rango");
        throw XmlRpc::XmlRpcException("el primer parametro no esta en rango");
    }
    int b = params[1];
    if( !(b >= 0 && b <= 4)){
        Log->log(LogLevel::ERROR, LogDomain::Reporte, "el segundo parametro no esta en rango");
        throw XmlRpc::XmlRpcException("el segundo parametro no esta en rango");
    }

    LogDomain dom = static_cast<LogDomain>(a);
    LogLevel lvl = static_cast<LogLevel>(b);
    result = Log->getMsj(lvl, dom);

    
    std::cout << Log->obtenerHoraActual() << result << std::endl;

    Log->~Logger();
}*/
void Reporte::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) {
    using XmlRpc::XmlRpcValue;

    
    // Control de acceso
    if (Nivel_de_Acceso == 1) {
        // usuario sin permiso
        throw XmlRpc::XmlRpcException("No autorizado");
    } // Nivel 0 (sin login) y 2 (admin) -> permitido para evitar pérdida de estado entre llamadas
    // Mantener el nivel de acceso para permitir múltiples consultas consecutivas

    // Si params no es válido -> devolver todo el log
    if (!params.valid()) {
        result = Log->getMsj();
        return;
    }

    // Caso: un entero (filtrar por nivel)
    if (params.getType() == XmlRpcValue::TypeInt) {
        int lvl = (int)params; // seguro porque ya comprobamos el tipo
        if (lvl < 0 || lvl > 3) {
            Log->log(LogLevel::ERROR, LogDomain::Reporte, "Nivel fuera de rango");
            throw XmlRpc::XmlRpcException("Nivel fuera de rango");
        }
        result = Log->getMsj(static_cast<LogLevel>(lvl));
        return;
    }

    // Caso: arreglo
    if (params.getType() == XmlRpcValue::TypeArray) {
        int sz = params.size();

        if (sz == 0) {
            result = Log->getMsj();
            return;
        }
        if (sz == 1) {
            if (params[0].getType() != XmlRpcValue::TypeInt) {
                Log->log(LogLevel::ERROR, LogDomain::Reporte, "param[0] no entero");
                throw XmlRpc::XmlRpcException("param[0] no entero");
            }
            int lvl = (int)params[0];
            if (lvl < 0 || lvl > 3) throw XmlRpc::XmlRpcException("Nivel fuera de rango");

            result = Log->getMsj(static_cast<LogLevel>(lvl));
            return;
        }
        if (sz >= 2) {
            if (params[0].getType() != XmlRpcValue::TypeInt || params[1].getType() != XmlRpcValue::TypeInt) {
                Log->log(LogLevel::ERROR, LogDomain::Reporte, "parametros deben ser enteros [domain, level]");
                throw XmlRpc::XmlRpcException("parametros deben ser enteros [domain, level]");
            }
            int a = (int)params[0]; // domain
            int b = (int)params[1]; // level
            // Comprobar rangos válidos (ajusta según enum)
            if (a < 0 || a > 3) {
                Log->log(LogLevel::ERROR, LogDomain::Reporte, "domain fuera de rango");
                throw XmlRpc::XmlRpcException("domain fuera de rango");
            }
            if (b < 0 || b > 3) {
                Log->log(LogLevel::ERROR, LogDomain::Reporte, "level fuera de rango");
                throw XmlRpc::XmlRpcException("level fuera de rango");
            }
            LogLevel lvl = static_cast<LogLevel>(a);
            LogDomain dom = static_cast<LogDomain>(b);


            result = Log->getMsj(lvl, dom);
            return;
        }
    }

    // Cualquier otro caso inválido
    Log->log(LogLevel::ERROR, LogDomain::Reporte, "Parametros invalidos para Reporte::execute");
    throw XmlRpc::XmlRpcException("Parametros invalidos para Reporte::execute");
}
std::string Reporte::help(){
    return "Se entregan dos enteros corrspondientes al enum de log domain y log level respectivamente";
}
