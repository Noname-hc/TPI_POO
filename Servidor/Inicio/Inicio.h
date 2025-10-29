#ifndef _INICIO_
#define _INICIO_

#include <iostream>
#include "../../Libreria_RPC/XmlRpc.h"
#include "../G_Code/G_code.h"
#include "../Reporte/Reporte.h"
#include "../Logger/Logger.hpp"

class Inicio:XmlRpc::XmlRpcServerMethod{
private:
    int Nivel_de_Acceso = 0;
    Reporte* Repo = nullptr;
    G_Code* gcode = nullptr;
    Logger* Log;

public:
    Inicio(const int Nivel_de_Acceso, Logger *Log, XmlRpc::XmlRpcServer *S);
    ~Inicio();

    void setMethod(Reporte *Repo, G_Code *gcode);
    void execute(XmlRpcValue& params, XmlRpcValue& result);

};

#endif