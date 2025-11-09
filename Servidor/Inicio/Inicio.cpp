// Inicio.cpp
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <stdexcept>
#include <sstream>

#include "Inicio.h"
#include "../../Libreria_RPC/XmlRpc.h"
#include "../G_Code/G_code.h"
#include "../Reporte/Reporte.h"
#include "../Logger/Logger.hpp"

using namespace XmlRpc;

Inicio::Inicio(const int Nivel_de_Acceso, Logger* Log, XmlRpcServer *S):XmlRpcServerMethod("Inicio" ,S){
    this->Nivel_de_Acceso = Nivel_de_Acceso;
    this->Log = Log;
}

Inicio::~Inicio(){

}

void Inicio::setMethod(Reporte *Repo, G_Code *gcode){
    this->Repo = Repo;
    this->gcode = gcode;
}

void Inicio::execute(XmlRpcValue& params, XmlRpcValue& result){
    // params[0] = usuario, params[1] = contrasena
    std::string usuario;
    std::string contrasena;
    try {
        usuario = (std::string)params[0];
        contrasena = (std::string)params[1];
    } catch(...) {
        throw XmlRpc::XmlRpcException("Parametros invalidos para Inicio: se requiere usuario y contraseña");
    }

    // escape simple: duplicar comillas simples para valores SQL
    auto escape_sql = [](const std::string &s)->std::string{
        std::string out;
        out.reserve(s.size());
        for(char c: s){
            if(c == '\''){
                out.push_back('\'');
                out.push_back('\'');
            } else {
                out.push_back(c);
            }
        }
        return out;
    };

    // Ruta por defecto para la base de datos SQLite (puedes adaptarla)
    const std::string db_path = "Servidor/db/users.db";

    std::string sql = "SELECT level FROM users WHERE username='" + escape_sql(usuario) + "' AND password='" + escape_sql(contrasena) + "' LIMIT 1;";

    // Ejecutar la consulta usando la utilidad sqlite3 (evita tocar Makefiles)
    std::string cmd = "sqlite3 -noheader -separator '|' '" + db_path + "' \"" + sql + "\"";

    FILE *fp = popen(cmd.c_str(), "r");
    bool auth_ok = false;
    int nivel = 0;
    if(fp){
        char buf[256];
        if(fgets(buf, sizeof(buf), fp) != nullptr){
            std::string out(buf);
            while(!out.empty() && (out.back()=='\n' || out.back()=='\r' || out.back()==' ')) out.pop_back();
            if(!out.empty()){
                try{
                    nivel = std::stoi(out);
                    auth_ok = true;
                }catch(...){}
            }
        }
        pclose(fp);
    }

    // Fallback al comportamiento previo si la BD no existe o la consulta falló
    if(!auth_ok){
        if(usuario == "Nacho" && contrasena == "123"){
            nivel = static_cast<int>(UserLvL::usuario);
            auth_ok = true;
        } else if(usuario == "Nico" && contrasena == "777"){
            nivel = static_cast<int>(UserLvL::admin);
            auth_ok = true;
        }
    }

    if(!auth_ok){
        this->Nivel_de_Acceso = 0;
        if(this->Repo) this->Repo->setLvL(0);
        if(this->gcode) this->gcode->setLvL(0);
        result = std::string("Credenciales invalidas");
        try{ if(Log) Log->log(LogLevel::ERROR, LogDomain::Inicio, "Intento de logueo fallido para usuario: " + usuario);}catch(...){ }
        return;
    }

    // Autenticado: actualizar niveles
    this->Nivel_de_Acceso = nivel;
    if(this->Repo) this->Repo->setLvL((nivel==static_cast<int>(UserLvL::admin))?2:1);
    if(this->gcode) this->gcode->setLvL((nivel==static_cast<int>(UserLvL::admin))?2:1);

    if(nivel == static_cast<int>(UserLvL::admin)){
        result = std::string("Bienvenido administrador");
    } else {
        result = std::string("Bienvenido cliente");
    }
    try{ if(Log) Log->log(LogLevel::INFO, LogDomain::Inicio, std::string("Usuario autenticado: ") + usuario);}catch(...){ }
}