#include "Tarea.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cctype>
#include <ctime>
#include <unistd.h>

using namespace XmlRpc;

Tarea::Tarea(Logger* log, int nivel, XmlRpcServer* S, const std::string& dir)
    : XmlRpcServerMethod("Tarea", S), Log(log), Nivel_de_Acceso(nivel), base_dir(dir) {
    ensure_dir();
}

Tarea::~Tarea() {
    // No destruir Logger aquí: es propiedad compartida con otras clases
}

void Tarea::setLvL(const int nivel) {
    this->Nivel_de_Acceso = nivel;
}

bool Tarea::ensure_dir() {
    try {
        if (base_dir.empty()) base_dir = "tareas";
        std::filesystem::path p(base_dir);
        if (std::filesystem::exists(p)) return true;
        return std::filesystem::create_directories(p);
    } catch(...) {
        return false;
    }
}

std::string Tarea::sanitize_name(const std::string& name) {
    std::string out;
    out.reserve(name.size());
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c=='_' || c=='-' ) out.push_back(c);
        else out.push_back('_');
    }
    if (out.empty()) out = "tarea";
    return out;
}

std::string Tarea::task_path(const std::string& name) const {
    std::string n = sanitize_name(name);
    std::filesystem::path p(base_dir);
    p /= (n + ".task");
    return p.string();
}

static inline std::string trim_copy(const std::string& s){
    size_t i=0, j=s.size();
    while(i<j && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    while(j>i && std::isspace(static_cast<unsigned char>(s[j-1]))) --j;
    return s.substr(i, j-i);
}

bool Tarea::validate_and_normalize(const std::string& line, std::string& normalized, std::string& error) const {
    error.clear();
    normalized.clear();
    std::string s = trim_copy(line);
    if (s.empty()) { error = "Linea vacia"; return false; }

    // separar por espacios
    std::vector<std::string> toks;
    {
        std::istringstream iss(s);
        std::string tok;
        while (iss >> tok) toks.push_back(tok);
    }
    if (toks.empty()) { error = "Sin tokens"; return false; }

    // token 0: <ID><num>
    std::string t0 = toks[0];
    if (t0.size() < 2) { error = "Cabecera invalida (se espera G/M + numero)"; return false; }
    char id = static_cast<char>(std::toupper(static_cast<unsigned char>(t0[0])));
    if (id!='G' && id!='M') { error = "ID invalido (debe ser G o M)"; return false; }
    std::string numstr = t0.substr(1);
    if (numstr.size() < 1 || numstr.size() > 3) { error = "Numero fuera de rango (1-3 digitos)"; return false; }
    for (char c : numstr) if (!std::isdigit(static_cast<unsigned char>(c))) { error = "Numero no es digito"; return false; }
    int num = 0;
    try { num = std::stoi(numstr); } catch(...) { error = "Numero invalido"; return false; }

    // construir normalizado base
    normalized = std::string(1,id) + std::to_string(num);

    // resto de tokens: <param><value>
    auto is_param = [](char c){ c=static_cast<char>(std::toupper(static_cast<unsigned char>(c))); return c=='X'||c=='Y'||c=='Z'||c=='E'||c=='F'; };
    for (size_t i=1;i<toks.size();++i){
        const std::string& tk = toks[i];
        if (tk.size()<2){ error = "Parametro invalido (muy corto)"; return false; }
        char p = static_cast<char>(std::toupper(static_cast<unsigned char>(tk[0])));
        if (!is_param(p)) { error = "Parametro invalido (no es X/Y/Z/E/F)"; return false; }
        std::string v = tk.substr(1);
        if (v.empty()) { error = "Valor vacio"; return false; }
        // validar numero con signo/decimal
        try {
            size_t idx=0; (void)idx; 
            std::stod(v, &idx);
            if (idx != v.size()) { error = "Valor con caracteres extra"; return false; }
        } catch(...) { error = "Valor numerico invalido"; return false; }
        normalized += std::string(" ") + p + v;
    }
    return true;
}

std::string Tarea::op_add(const std::string& name, const std::string& line){
    std::string norm, err;
    if (!validate_and_normalize(line, norm, err)) {
        if (Log) try{ Log->log(LogLevel::ERROR, LogDomain::MAIN, std::string("Tarea add invalida: ")+err);}catch(...){ }
        throw XmlRpc::XmlRpcException((std::string)"Linea invalida: " + err);
    }
    if (!ensure_dir()) throw XmlRpc::XmlRpcException("No se puede crear directorio de tareas");
    std::string path = task_path(name);
    std::ofstream f(path, std::ios::out | std::ios::app);
    if (!f) throw XmlRpc::XmlRpcException("No se pudo abrir la tarea para escribir");
    f << norm << '\n';
    f.close();
    if (Log) try{ Log->log(LogLevel::INFO, LogDomain::MAIN, std::string("Tarea[ ")+name+
        "] + " + norm);}catch(...){ }
    return std::string("OK: added to '") + name + "'";
}

std::string Tarea::op_show(const std::string& name) const {
    std::string path = task_path(name);
    std::ifstream f(path);
    if (!f) throw XmlRpc::XmlRpcException("Tarea no encontrada");
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::string Tarea::op_clear(const std::string& name){
    std::string path = task_path(name);
    std::ofstream f(path, std::ios::out | std::ios::trunc);
    if (!f) throw XmlRpc::XmlRpcException("No se pudo truncar tarea");
    f.close();
    if (Log) try{ Log->log(LogLevel::INFO, LogDomain::MAIN, std::string("Tarea[ ")+name+
        "] cleared"); }catch(...){ }
    return std::string("OK: cleared '") + name + "'";
}

std::string Tarea::op_list() const {
    std::ostringstream out;
    try{
        for (auto& de : std::filesystem::directory_iterator(base_dir)){
            if (!de.is_regular_file()) continue;
            auto p = de.path();
            if (p.extension()==".task"){
                out << p.stem().string() << '\n';
            }
        }
    }catch(...){ }
    return out.str();
}

std::string Tarea::op_run(const std::string& name){
    std::string path = task_path(name);
    std::ifstream f(path);
    if (!f) throw XmlRpc::XmlRpcException("Tarea no encontrada");
    std::vector<std::string> lines;
    for (std::string ln; std::getline(f, ln);){
        ln = trim_copy(ln);
        if (!ln.empty()) lines.push_back(ln);
    }
    if (lines.empty()) return "(tarea vacia)";

    Serial_Com Com;
    char Buffer[512];
    Com.T_R_Init(9600, 2, "/dev/ttyACM0");
    if (Log) try{ Log->log(LogLevel::DEBUG, LogDomain::MAIN, "Tarea: iniciando comunicacion"); }catch(...){ }
    sleep(2);
    Com.ClearInput();
    Com.Recepcion(Buffer, sizeof(Buffer), 1000);

    std::ostringstream rep;
    for (auto& cmd : lines){
        std::string msg = cmd; // ya normalizado G/M y parametros
        Com.Transmision(msg);
        if (Log) try{ Log->log(LogLevel::DEBUG, LogDomain::MAIN, std::string("TX: ")+msg); }catch(...){ }
        Buffer[0] = '\0';
        Com.Recepcion(Buffer, sizeof(Buffer), 3000);
        rep << cmd << " -> " << Buffer << '\n';
    }

    // intento de lectura final
    Buffer[0] = '\0';
    Com.Recepcion(Buffer, sizeof(Buffer), 1000);
    std::string tail = Buffer;
    if (!tail.empty()) rep << tail << '\n';

    if (Log) try{ Log->log(LogLevel::INFO, LogDomain::MAIN, std::string("Tarea[ ")+name+
        "] ejecutada"); }catch(...){ }
    return rep.str();
}

void Tarea::execute(XmlRpcValue& params, XmlRpcValue& result){
    // Acceso: requiere estar logueado (nivel 1 o 2). Cualquier rol puede usar Tarea
    if (Nivel_de_Acceso == 0){
        if (Log) try{ Log->log(LogLevel::ERROR, LogDomain::MAIN, "Falta logearse (Tarea)"); }catch(...){ }
        throw XmlRpc::XmlRpcException("Falta logearse");
    }

    // Parseo de parametros
    if (!params.valid() || (params.getType()!=XmlRpcValue::TypeArray) || params.size()<1 || params[0].getType()!=XmlRpcValue::TypeString){
        throw XmlRpc::XmlRpcException("Uso: [op, ...]; op in {add,run,show,list,clear}");
    }
    std::string op = (std::string)params[0];
    std::transform(op.begin(), op.end(), op.begin(), [](unsigned char c){ return std::tolower(c); });

    std::string out;
    if (op == "add"){
        if (params.size() != 3 || params[1].getType()!=XmlRpcValue::TypeString || params[2].getType()!=XmlRpcValue::TypeString){
            throw XmlRpc::XmlRpcException("Uso: ['add', nombre, linea]");
        }
        out = op_add((std::string)params[1], (std::string)params[2]);
    } else if (op == "run"){
        if (params.size() != 2 || params[1].getType()!=XmlRpcValue::TypeString){
            throw XmlRpc::XmlRpcException("Uso: ['run', nombre]");
        }
        out = op_run((std::string)params[1]);
    } else if (op == "show"){
        if (params.size() != 2 || params[1].getType()!=XmlRpcValue::TypeString){
            throw XmlRpc::XmlRpcException("Uso: ['show', nombre]");
        }
        out = op_show((std::string)params[1]);
    } else if (op == "list"){
        if (params.size() != 1){
            throw XmlRpc::XmlRpcException("Uso: ['list']");
        }
        out = op_list();
    } else if (op == "clear"){
        if (params.size() != 2 || params[1].getType()!=XmlRpcValue::TypeString){
            throw XmlRpc::XmlRpcException("Uso: ['clear', nombre]");
        }
        out = op_clear((std::string)params[1]);
    } else {
        throw XmlRpc::XmlRpcException("Operacion desconocida (add|run|show|list|clear)");
    }

    // reset nivel de acceso como en otros métodos
    this->Nivel_de_Acceso = 0;
    result = out;
}
