#ifndef _TAREA_H
#define _TAREA_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <filesystem>

#include "Logger/Logger.hpp"
#include "../Libreria_RPC/XmlRpc.h"
#include "Serial_Com/Serial_Com.h"

// Clase Tarea: administra tareas preestablecidas compuestas por líneas G/M-code.
// Expone el método XML-RPC "Tarea" con operaciones:
//   - ["add", nombre, linea]   -> agrega una línea a la tarea
//   - ["run", nombre]          -> ejecuta la tarea (envía cada línea por serie)
//   - ["show", nombre]         -> devuelve el contenido de la tarea
//   - ["list"]                 -> lista tareas disponibles
//   - ["clear", nombre]        -> borra la tarea

class Tarea : public XmlRpc::XmlRpcServerMethod {
public:
    Tarea(Logger* log, int nivel, XmlRpc::XmlRpcServer* S, const std::string& dir = "tareas");
    ~Tarea();

    void setLvL(const int nivel);
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;

private:
    Logger* Log;
    int Nivel_de_Acceso = 0;
    std::string base_dir; // carpeta donde se almacenan las tareas (relative a Servidor/)

    bool ensure_dir();
    static std::string sanitize_name(const std::string& name);
    std::string task_path(const std::string& name) const;

    // Valida formato: "<ID><num> <param><value> ..."
    // ID = 'G'|'M'; num = 1-3 dígitos; param = [X,Y,Z,E,F]; value = entero o float con signo
    bool validate_and_normalize(const std::string& line, std::string& normalized, std::string& error) const;

    // Helpers de operaciones
    std::string op_add(const std::string& name, const std::string& line);
    std::string op_run(const std::string& name);
    std::string op_show(const std::string& name) const;
    std::string op_clear(const std::string& name);
    std::string op_list() const;
};

#endif // _TAREA_H

