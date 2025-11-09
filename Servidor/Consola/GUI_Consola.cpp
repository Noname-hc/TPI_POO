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
#include <cstdio>
#include <cstdlib>

#include "../../Libreria_RPC/XmlRpcClient.h"
using namespace XmlRpc;

// Adapter local que "simula" el servidor generando el literal G-code
class LocalEchoServer : public IServer {
public:
    ServerResult ejecutar(comandos cmd, const std::vector<std::string>& args, const User& /*who*/) override {
        try {
            auto g = GUI_Consola::build_gcode_literal(cmd, args);
            if(g.empty()) return {false, std::string("No se pudo generar G-code para comando")};
            return {true, std::string("G-code: ") + g};
        } catch (const std::exception& e) {
            return {false, std::string("Excepcion: ")+e.what()};
        }
    }
    ServerResult login(const std::string& user, const std::string& /*pass*/) override {
        return {true, std::string("Bienvenido cliente (local) ")+user};
    }
    std::pair<std::string,int> endpoint() const override { return {"localhost", 0}; }
};

// Adapter XML-RPC real: llama al servidor C++ expuesto en Server.cpp
class RPCServerAdapter : public IServer {
public:
    RPCServerAdapter(std::string host, int port)
        : host_(std::move(host)), port_(port), client_(host_.c_str(), port_) {}

    ServerResult ejecutar(comandos cmd, const std::vector<std::string>& args, const User& who) override {
        // Asegura login en el servidor (XmlRpc method: "Inicio")
        if(!ensure_login(who)) {
            return {false, std::string("Login en servidor falló para usuario ") + who.id};
        }

        // Llamada a G_Code según contrato del servidor
        try {
            XmlRpcValue params, result;
            const int a = static_cast<int>(cmd_to_code(cmd));

            if(cmd == comandos::Movimiento_Lineal) {
                // Construir "x,y,z" para G0; ignorar F
                std::string x,y,z;
                for(const auto& t_raw : args){
                    if(t_raw.size() < 2) continue;
                    char k = std::toupper(static_cast<unsigned char>(t_raw[0]));
                    std::string v = t_raw.substr(1);
                    if(k=='X') x=v; else if(k=='Y') y=v; else if(k=='Z') z=v;
                }
                if(x.empty()) { x = "0"; }
                if(y.empty()) { y = "0"; }
                if(z.empty()) { z = "0"; }
                std::string xyz = x + "," + y + "," + z;
                params[0] = a;        // 0
                params[1] = xyz;      // "x,y,z"
            } else {
                // Otros comandos no requieren segundo parámetro
                params = a; // El servidor acepta un entero directo
            }

            bool ok = client_.execute("G_Code", params, result);
            if(!ok) return {false, "Llamada XML-RPC a G_Code falló"};
            std::string msg;
            try { msg = (std::string)result; } catch(...) { msg = "(sin respuesta textual)"; }
            return {true, msg};
        } catch(const std::exception& e) {
            return {false, std::string("Excepcion en ejecutar: ") + e.what()};
        }
    }

    ServerResult login(const std::string& user, const std::string& pass) override {
        try{
            XmlRpcValue params, result;
            params[0] = user;
            params[1] = pass;
            bool ok = client_.execute("Inicio", params, result);
            if(!ok) return {false, "execute(Inicio) fallo"};
            std::string resp;
            try{ resp = (std::string)result; }catch(...){ resp = ""; }
            if(resp.find("Bienvenido") != std::string::npos) {
                logged_in_ = true; logged_user_ = user; last_login_msg_ = resp;
                return {true, resp};
            }
            return {false, resp};
        }catch(const std::exception& e){ return {false, e.what()}; }
    }
    std::pair<std::string,int> endpoint() const override { return {host_, port_}; }

private:
    // Mapea nuestro enum a los códigos numéricos que espera el servidor (igual al valor del enum salvo G0..Gnn)
    static int cmd_to_code(comandos c) {
        switch(c){
            case comandos::Movimiento_Lineal: return 0;  // G0 X/Y/Z
            case comandos::Rampa:             return 4;  // G4 S..
            case comandos::Homing:            return 28; // G28
            case comandos::C_Absolutas:       return 90; // G90
            case comandos::C_Relativas:       return 91; // G91
            case comandos::Set_0:             return 92; // G92
            default:
                return static_cast<int>(c);   // M-codes ya están codificados como 1000xx
        }
    }

    bool ensure_login(const User& who) {
        if(logged_in_ && who.id == logged_user_) return true;
        try{
            XmlRpcValue params, result;
            params[0] = who.id;
            params[1] = who.clave;
            bool ok = client_.execute("Inicio", params, result);
            if(!ok) { last_login_msg_ = "execute(Inicio) fallo"; return false; }
            std::string resp;
            try{ resp = (std::string)result; }catch(...){ resp = ""; }
            // Aceptamos cualquier mensaje "Bienvenido ..."
            if(resp.find("Bienvenido") != std::string::npos) {
                logged_in_ = true;
                logged_user_ = who.id;
                last_login_msg_ = resp;
                return true;
            }
            last_login_msg_ = resp;
            return false;
        }catch(const std::exception& e){ last_login_msg_ = e.what(); return false; }
    }

private:
    std::string host_;
    int port_;
    XmlRpcClient client_;
    bool logged_in_ = false;
    std::string logged_user_;
    std::string last_login_msg_;
};

// ---------------- CSV helpers ----------------
static std::string esc_csv(std::string s) {  // escapa si tiene comas o comillas
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
    if(use_sqlite_) {
        // Consulta simple a SQLite via CLI (similar a Servidor/Inicio/Inicio.cpp)
        if(sqlite_db_path_.empty()) return std::nullopt;
        auto esc = [](const std::string& s){
            std::string out; out.reserve(s.size());
            for(char c: s){ if(c=='\'') out.push_back('\''); out.push_back(c);} return out; };
        std::string sql = "SELECT level FROM users WHERE username='" + esc(id) + "' AND password='" + esc(clave) + "' LIMIT 1;";
        std::string cmd = "sqlite3 -noheader -separator '|' '" + sqlite_db_path_ + "' \"" + sql + "\"";
        FILE* fp = popen(cmd.c_str(), "r");
        if(!fp) return std::nullopt;
        char buf[256]; std::string out;
        if(fgets(buf, sizeof(buf), fp) != nullptr){ out = buf; while(!out.empty() && (out.back()=='\n'||out.back()=='\r'||out.back()==' ')) out.pop_back(); }
        pclose(fp);
        if(out.empty()) return std::nullopt;
        int level = 0; try{ level = std::stoi(out); }catch(...){ return std::nullopt; }
        User u; u.id=id; u.nombre=id; u.clave=clave; u.rol = (level==777?Rol::Admin:Rol::Operador);
        return u;
    } else {
        auto it = by_id_.find(id);
        if(it==by_id_.end()) return std::nullopt;
        if(it->second.clave != clave) return std::nullopt;
        return it->second;
    }
}

bool UserDB::load_sqlite(const std::string& db_path) {
    if(!std::filesystem::exists(db_path)) return false;
    use_sqlite_ = true;
    sqlite_db_path_ = db_path;
    by_id_.clear();
    return true;
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
    // Autenticación por XML-RPC (Inicio); sin dependencia de CSV/SQLite
    logger_ = std::make_unique<CsvLogger>(log_csv_);

    std::cout << "=== Consola Servidor - Control Robot ===\n";

    while(true) {
        // Login
        std::cout << "\nIniciar sesion\n";
        std::cout << "Usuario: ";
        std::string uid; if(!std::getline(std::cin, uid)) break;
        std::cout << "Clave  : ";
        std::string pwd; if(!std::getline(std::cin, pwd)) break;

        if(!server_) { std::cout << "[ERROR] Servidor no configurado.\n"; return 1; }
        auto lr = server_->login(uid, pwd);
        if(!lr.ok) { std::cout << "Login fallido: " << lr.msg << "\n"; continue; }

        User current; current.id=uid; current.nombre=uid; current.clave=pwd;
        std::string msg = lr.msg; std::string low=msg; std::transform(low.begin(),low.end(),low.begin(),::tolower);
        current.rol = (low.find("administrador")!=std::string::npos)?Rol::Admin:Rol::Operador;
        std::cout << msg << "\n";

        imprimir_menu();
        std::cout << "\nTip: escribi 'ui' o 'botones' para abrir el menu de botones." << '\n';

        // Lanzar GUI gráfica de botones (Tkinter)
        lanzar_gui_botones(current);
        std::cout << "\nSesion finalizada.\n";
        continue; // volver al login
        
        // (Opcional) Loop de comandos por consola
        while(true) {
            std::cout << "\n> ";
            std::string line; if(!std::getline(std::cin, line)) { std::cout << "\n"; return 0; }
            if(line.empty()) continue;

            // comandos de control local (chequear antes de parsear)
            if(line == "help" || line == "ayuda") { imprimir_menu(); continue; }
            if(line == "menu") { imprimir_menu(); continue; }
            if(line == "ui" || line == "botones") { mostrar_menu_botones(current); continue; }
            if(line == "logout") { std::cout << "Sesion cerrada.\n"; break; }
            if(line == "exit" || line == "salir") { std::cout << "Saliendo...\n"; return 0; }

            auto parsed = parse_command(line);
            if(!parsed) {
                std::cout << "Comando no reconocido. Escribi 'help' para ver opciones.\n";
                continue;
            }

            comandos cmd = parsed->first;
            auto args    = parsed->second;

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

// ---------------- Construccion literal G-code ----------------
static std::string to_upper_copy(std::string s){
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::toupper(c); });
    return s;
}

std::string GUI_Consola::build_gcode_literal(comandos c, const std::vector<std::string>& args) {
    switch(c) {
        case comandos::C_Absolutas: return "G90";
        case comandos::C_Relativas: return "G91";
        case comandos::Set_0:       return "G92";
        case comandos::Homing:      return "G28";
        case comandos::Rampa: {
            // G4 S..
            std::string sparam;
            for(auto a: args){ auto t=to_upper_copy(a); if(!t.empty() && t[0]=='S'){ sparam=t; break; } }
            if(sparam.empty()) return "G4"; // sin parametro
            return std::string("G4 ")+sparam;
        }
        case comandos::Movimiento_Lineal: {
            // En servidor, a==0 => G0 X.. Y.. Z.. (sin manejo de F). Respetamos eso.
            std::string out = "G0";
            std::string x,y,z,f;
            for(auto a: args){
                auto t=to_upper_copy(a);
                if(t.size()<2) continue;
                if(t[0]=='X') x=t;
                else if(t[0]=='Y') y=t;
                else if(t[0]=='Z') z=t;
                else if(t[0]=='F') f=t; // opcional
            }
            if(!x.empty()) out += ' '+x;
            if(!y.empty()) out += ' '+y;
            if(!z.empty()) out += ' '+z;
            if(!f.empty()) out += ' '+f;
            return out;
        }
        default: {
            // M-codes: enum usa valores tipo 10001 -> M1, 100017 -> M17, 1000106 -> M106, etc.
            int val = static_cast<int>(c);
            if(val >= 1000) {
                auto s = std::to_string(val);
                if(s.size() >= 5) {
                    auto tail = s.substr(4);
                    return std::string("M") + tail;
                }
            }
            // Fallback G-code generico para Gn
            int g = static_cast<int>(c);
            return std::string("G") + std::to_string(g);
        }
    }
}

// ---------------- Menu de "botones" ----------------
void GUI_Consola::mostrar_menu_botones(const User& current) {
    struct Item { const char* label; comandos c; bool admin=false; };
    const std::vector<Item> items = {
        {"C_Absolutas (G90)",           comandos::C_Absolutas, false},
        {"C_Relativas (G91)",           comandos::C_Relativas, false},
        {"Set_0 (G92)",                 comandos::Set_0,       true },
        {"Homing (G28)",                 comandos::Homing,      false},
        {"Movimiento Lineal (G0 X/Y/Z)", comandos::Movimiento_Lineal, false},
        {"Rampa (G4 S..)",               comandos::Rampa,       false},
        {"Bomba ON (M1)",                comandos::Bomba_ON,    false},
        {"Bomba OFF (M2)",               comandos::Bomba_OFF,   false},
        {"Griper ON (M3)",               comandos::Griper_ON,   false},
        {"Griper OFF (M5)",              comandos::Griper_OFF,  false},
        {"Laser ON (M6)",                comandos::Laser_ON,    false},
        {"Laser OFF (M7)",               comandos::Laser_OFF,   false},
        {"Motor ON (M17)",               comandos::Motor_ON,    false},
        {"Motor OFF (M18)",              comandos::Motor_OFF,   false},
        {"Ventiladores ON (M106)",       comandos::Ventiladores_ON,  false},
        {"Ventiladores OFF (M107)",      comandos::Ventiladores_OFF, false},
        {"Reporte General (M114)",       comandos::Reporte_General,  false},
        {"Reporte finales (M119)",       comandos::Reporte_finales,  true },
    };

    while(true) {
        std::cout << "\n=== Menu de botones (numeros) ===\n";
        for(size_t i=0;i<items.size();++i){
            std::cout << (i+1) << ") " << items[i].label;
            if(items[i].admin) std::cout << " [admin]";
            std::cout << "\n";
        }
        std::cout << "0) Volver\n> ";
        std::string sel;
        if(!std::getline(std::cin, sel)) return;
        if(sel=="0" || to_upper_copy(sel)=="VOLVER") return;
        int idx=-1;
        try{ idx = std::stoi(sel); }catch(...){ idx=-1; }
        if(idx < 1 || (size_t)idx > items.size()) { std::cout << "Opcion invalida\n"; continue; }
        const auto& it = items[idx-1];

        if(it.admin && current.rol != Rol::Admin){
            std::cout << "Permiso denegado: requiere rol admin.\n";
            if(logger_) logger_->log(current, it.label, {}, "DENY", "admin_required");
            continue;
        }

        std::vector<std::string> args;
        if(it.c == comandos::Movimiento_Lineal) {
            std::string x,y,z,f;
            std::cout << "X (vacio para omitir): "; std::getline(std::cin, x);
            std::cout << "Y (vacio para omitir): "; std::getline(std::cin, y);
            std::cout << "Z (vacio para omitir): "; std::getline(std::cin, z);
            std::cout << "F (opcional, vacio para omitir): "; std::getline(std::cin, f);
            if(!x.empty()) args.push_back("X"+x);
            if(!y.empty()) args.push_back("Y"+y);
            if(!z.empty()) args.push_back("Z"+z);
            if(!f.empty()) args.push_back("F"+f);
        } else if(it.c == comandos::Rampa) {
            std::string s;
            std::cout << "S (tiempo/valor): "; std::getline(std::cin, s);
            if(!s.empty()) args.push_back("S"+s);
        }

        ServerResult res{false, ""};
        if(server_) res = server_->ejecutar(it.c, args, current);
        else {
            // Fallback local por si no hay server
            try { auto lit = build_gcode_literal(it.c, args); res = {true, std::string("G-code: ")+lit}; }
            catch(const std::exception& e){ res = {false, e.what()}; }
        }

        if(res.ok) {
            std::cout << "[OK] " << res.msg << "\n";
            if(logger_) logger_->log(current, it.label, args, "OK", res.msg);
        } else {
            std::cout << "[ERROR] " << res.msg << "\n";
            if(logger_) logger_->log(current, it.label, args, "ERR", res.msg);
        }
    }
}

static std::string sh_quote(const std::string& s){
    // Simple shell single-quote escaping: ' -> '\''
    std::string out; out.reserve(s.size()+2);
    out.push_back('\'');
    for(char c: s){ if(c=='\'') out += "'\\''"; else out.push_back(c); }
    out.push_back('\'');
    return out;
}

void GUI_Consola::lanzar_gui_botones(const User& current) {
    if(!server_) { std::cout << "[ERROR] servidor no configurado\n"; return; }
    auto ep = server_->endpoint();
    std::string host = ep.first; int port = ep.second;
    // Verificar disponibilidad de Tkinter
    int has_tk = std::system("python3 -c 'import tkinter' > /dev/null 2>&1");
    if(has_tk != 0) {
        std::cout << "Tkinter no está disponible en el sistema. Abriendo menú por consola...\n";
        mostrar_menu_botones(current);
        return;
    }
    // Ejecutar script Tkinter
    std::string cmd = std::string("python3 gui_botones.py ")
        + sh_quote(host) + " " + std::to_string(port) + " "
        + sh_quote(current.id) + " " + sh_quote(current.clave);
    std::cout << "Lanzando GUI de botones...\n";
    int rc = std::system(cmd.c_str());
    if(rc != 0){ std::cout << "GUI terminó con codigo: " << rc << "\n"; }
}

#if !defined(NO_GUI_CONSOLA_MAIN)
// Pequeño main: si se pasan host y puerto, usa RPC real; si no, usa eco local.
// Uso: ./consola_servidor usuarios.csv log.csv [HOST] [PORT]
int main(int argc, char* argv[]) {
    GUI_Consola app;
    if(argc >= 2) app.setUserCsv(argv[1]);
    if(argc >= 3) app.setLogCsv(argv[2]);
    if(argc >= 5) {
        std::string host = argv[3];
        int port = std::atoi(argv[4]);
        app.setServer(std::make_unique<RPCServerAdapter>(host, port));
        std::cout << "[RPC] Conectando a " << host << ":" << port << "\n";
    } else {
        app.setServer(std::make_unique<LocalEchoServer>());
        std::cout << "[LOCAL] Sin HOST/PORT: usando servidor eco local.\n";
    }
    return app.run();
}
#endif
