#include "GUI_Consola.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <cctype>
#include <iomanip>

// ---------------- CSV helpers ----------------
static std::string esc_csv(std::string s) {
    if(s.find(',')!=std::string::npos || s.find('"')!=std::string::npos){
        std::string t; t.reserve(s.size()+2);
        t.push_back('"');
        for(char c: s){ if(c=='"') t.push_back('"'); t.push_back(c); }
        t.push_back('"');
        return t;
    }
    return s;
}

// ---------------- UserDB ----------------
bool UserDB::load(const std::string& csv_path) {
    std::ifstream in(csv_path);
    if(!in) return false;

    auto parse_line = [&](const std::string& ln){
        std::vector<std::string> cols;
        std::string cur; bool inq=false;
        for(char c: ln){
            if(c=='"') inq=!inq;
            else if(c==',' && !inq){ cols.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        cols.push_back(cur);
        return cols;
    };

    std::string line;
    while(std::getline(in, line)) {
        if(line.empty()) continue;
        auto cols = parse_line(line);
        if(cols.size() < 4) continue;
        User u;
        u.id     = cols[0];
        u.nombre = cols[1];
        u.clave  = cols[2];
        std::string rol = cols[3];
        std::transform(rol.begin(), rol.end(), rol.begin(), ::tolower);
        u.rol = (rol.find("admin")!=std::string::npos) ? Rol::Admin : Rol::Operador;
        by_id_[u.id] = u;
    }
    return !by_id_.empty();
}

std::optional<User> UserDB::authenticate(const std::string& id, const std::string& clave) const {
    auto it = by_id_.find(id);
    if(it==by_id_.end()) return std::nullopt;
    if(it->second.clave != clave) return std::nullopt;
    return it->second;
}

// ---------------- CsvLogger ----------------
CsvLogger::CsvLogger(std::string path) : path_(std::move(path)) {}

void CsvLogger::ensure_header() {
    if(initialized_) return;
    bool exists = std::filesystem::exists(path_);
    std::ofstream out(path_, std::ios::app);
    if(!exists) {
        out << "timestamp,user_id,user_name,rol,command,args,status,msg\n";
    }
    initialized_ = true;
}

void CsvLogger::log(const User& u, const std::string& cmd_literal, const std::vector<std::string>& args,
                    const std::string& status, const std::string& msg) {
    ensure_header();
    std::ofstream out(path_, std::ios::app);
    auto rol_s = (u.rol==Rol::Admin ? "admin" : "operador");
    out << GUI_Consola::now_iso() << ','
        << esc_csv(u.id) << ','
        << esc_csv(u.nombre) << ','
        << rol_s << ','
        << esc_csv(cmd_literal) << ','
        << esc_csv(GUI_Consola::join(args," ")) << ','
        << status << ','
        << esc_csv(msg)
        << '\n';
}

// ---------------- GUI_Consola ----------------
GUI_Consola::GUI_Consola() = default;

void GUI_Consola::setUserCsv(const std::string& usuarios_csv) { usuarios_csv_ = usuarios_csv; }
void GUI_Consola::setLogCsv(const std::string& log_csv)       { log_csv_ = log_csv; }
void GUI_Consola::setServer(std::unique_ptr<IServer> server)  { server_ = std::move(server); }

int GUI_Consola::run() {
    if(!db_.load(usuarios_csv_)) {
        std::cerr << "ERROR: No pude cargar usuarios desde " << usuarios_csv_ << "\n";
        std::cerr << "Formato: id,nombre,clave,rol\n";
        return 1;
    }
    logger_ = std::make_unique<CsvLogger>(log_csv_);

    std::cout << "=== Consola Servidor - Control Robot ===\n";

    while(true) {
        // Login
        std::cout << "\nIniciar sesion\n";
        std::cout << "Usuario: ";
        std::string uid; if(!std::getline(std::cin, uid)) break;
        std::cout << "Clave  : ";
        std::string pwd; if(!std::getline(std::cin, pwd)) break;

        auto auth = db_.authenticate(uid, pwd);
        if(!auth) {
            std::cout << "Credenciales invalidas.\n";
            continue;
        }
        User current = *auth;
        std::cout << "Bienvenido/a, " << current.nombre
                  << " (" << (current.rol==Rol::Admin ? "admin" : "operador") << ")\n";

        imprimir_menu();

        // Loop comandos
        while(true) {
            std::cout << "\n> ";
            std::string line; if(!std::getline(std::cin, line)) { std::cout << "\n"; return 0; }
            if(line.empty()) continue;

            auto parsed = parse_command(line);
            if(!parsed) {
                std::cout << "Comando no reconocido. Escribi 'help' para ver opciones.\n";
                continue;
            }

            comandos cmd = parsed->first;
            auto args    = parsed->second;

            // comandos de control local
            if(cmd == comandos::C_Absolutas && line == "help") { imprimir_menu(); continue; } // por si escriben "help"
            if(line == "help" || line == "ayuda") { imprimir_menu(); continue; }
            if(line == "menu") { imprimir_menu(); continue; }
            if(line == "logout") { std::cout << "Sesion cerrada.\n"; break; }
            if(line == "exit" || line == "salir") { std::cout << "Saliendo...\n"; return 0; }

            // permisos
            if(requiere_admin(cmd) && current.rol != Rol::Admin) {
                std::cout << "Permiso denegado: requiere rol admin.\n";
                logger_->log(current, "PERMISO_DENEGADO", args, "DENY", "admin_required");
                continue;
            }

            if(!server_) {
                std::cout << "[ERROR] Servidor no configurado.\n";
                logger_->log(current, "NO_SERVER", args, "ERR", "no_server");
                continue;
            }

            // Ejecutar
            auto res = server_->ejecutar(cmd, args, current);
            if(res.ok) {
                std::cout << "[OK] " << res.msg << "\n";
                // guardamos el literal igual a lo que tipeó (primera palabra)
                auto first = split_ws(line);
                logger_->log(current, first.empty()?"":first[0], args, "OK", res.msg);
            } else {
                std::cout << "[ERROR] " << res.msg << "\n";
                auto first = split_ws(line);
                logger_->log(current, first.empty()?"":first[0], args, "ERR", res.msg);
            }
        }
    }
    return 0;
}

// -------- utilidades ----------
std::string GUI_Consola::now_iso() {
    using namespace std::chrono;
    auto t = system_clock::to_time_t(system_clock::now());
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t); 
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}

std::vector<std::string> GUI_Consola::split_ws(const std::string& s) {
    std::vector<std::string> out;
    std::string token;
    std::istringstream iss(s);
    while (iss >> token) out.push_back(token);
    return out;
}

std::string GUI_Consola::join(const std::vector<std::string>& v, const std::string& sep) {
    std::string r;
    for (size_t i=0;i<v.size();++i){ r+=v[i]; if(i+1<v.size()) r+=sep; }
    return r;
}

bool GUI_Consola::requiere_admin(comandos c) {
    switch(c){
        case comandos::Set_0:
        case comandos::Reporte_finales:
            return true;
        default:
            return false;
    }
}

// ----- Mapa literal->comandos y parser -----
static const std::unordered_map<std::string, comandos> kMap = {
    // NOMBRES EXACTOS DEL ENUM:
    {"Movimiento_Lineal", comandos::Movimiento_Lineal},
    {"Rampa",             comandos::Rampa},
    {"Homing",            comandos::Homing},
    {"C_Absolutas",       comandos::C_Absolutas},
    {"C_Relativas",       comandos::C_Relativas},
    {"Set_0",             comandos::Set_0},
    {"Bomba_ON",          comandos::Bomba_ON},
    {"Bomba_OFF",         comandos::Bomba_OFF},
    {"Griper_ON",         comandos::Griper_ON},
    {"Griper_OFF",        comandos::Griper_OFF},
    {"Laser_ON",          comandos::Laser_ON},
    {"Laser_OFF",         comandos::Laser_OFF},
    {"Motor_ON",          comandos::Motor_ON},
    {"Motor_OFF",         comandos::Motor_OFF},
    {"Ventiladores_ON",   comandos::Ventiladores_ON},
    {"Ventiladores_OFF",  comandos::Ventiladores_OFF},
    {"Reporte_General",   comandos::Reporte_General},
    {"Reporte_finales",   comandos::Reporte_finales},

    // --- Opcional: ALIAS útiles (podés borrar si querés “solo enum”) ---
    {"G1",  comandos::Movimiento_Lineal},
    {"g1",  comandos::Movimiento_Lineal},
    {"G90", comandos::C_Absolutas},
    {"g90", comandos::C_Absolutas},
    {"G91", comandos::C_Relativas},
    {"g91", comandos::C_Relativas},
    {"G92", comandos::Set_0},
    {"g92", comandos::Set_0},
    {"G28", comandos::Homing},
    {"g28", comandos::Homing},
};

void GUI_Consola::imprimir_menu() const {
    std::cout <<
    "\n=== COMANDOS (nombres EXACTOS del enum `comandos`) ===\n"
    "C_Absolutas             (G90)\n"
    "C_Relativas             (G91)\n"
    "Set_0                   (G92)\n"
    "Movimiento_Lineal X.. Y.. Z.. [F..]\n"
    "Rampa S..\n"
    "Homing                  (G28)\n"
    "Bomba_ON / Bomba_OFF\n"
    "Griper_ON / Griper_OFF\n"
    "Laser_ON / Laser_OFF\n"
    "Motor_ON / Motor_OFF\n"
    "Ventiladores_ON / Ventiladores_OFF\n"
    "Reporte_General\n"
    "Reporte_finales         [admin]\n"
    "\nComandos locales: help | menu | logout | exit/salir\n";
}

std::optional<std::pair<comandos, std::vector<std::string>>>
GUI_Consola::parse_command(const std::string& line) const {
    auto toks = split_ws(line);
    if(toks.empty()) return std::nullopt;
    auto it = kMap.find(toks[0]);
    if(it == kMap.end()) return std::nullopt;
    std::vector<std::string> args(toks.begin()+1, toks.end());

    // Validaciones mínimas de parámetros según comando:
    switch(it->second) {
        case comandos::Movimiento_Lineal: {
            // Requiere al menos X/Y/Z en formato X.. Y.. Z.. (F.. opcional)
            bool okX=false, okY=false, okZ=false;
            for (auto& t : args) {
                if(t.size()<2) continue;
                char k = std::toupper(static_cast<unsigned char>(t[0]));
                if(k=='X') okX=true; else if(k=='Y') okY=true; else if(k=='Z') okZ=true;
            }
            if(!(okX||okY||okZ)) {
                std::cout << "[WARN] Movimiento_Lineal requiere al menos X/Y/Z.\n";
            }
        } break;
        case comandos::Rampa: {
            bool hasS=false;
            for (auto& t : args) if(!t.empty() && (t[0]=='S' || t[0]=='s')) hasS=true;
            if(!hasS) std::cout << "[WARN] Rampa requiere S<valor>.\n";
        } break;
        default: break;
    }

    return std::make_pair(it->second, std::move(args));
}
