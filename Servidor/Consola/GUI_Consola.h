#ifndef GUI_CONSOLA_H
#define GUI_CONSOLA_H

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <memory>

// Ajustá la ruta según tu repo:
//   p.ej.  #include "../Servidor/G_Code/G_code.h"
#include "G_code.h"   // Debe definir: enum class comandos { ... }

enum class Rol { Operador, Admin }; // roles de usuario

struct User {   // datos del usuario logeado
    std::string id;
    std::string nombre;
    std::string clave;
    Rol rol{Rol::Operador};
};

class UserDB {  
public:
    // CSV: id,nombre,clave,rol  (rol: operador|admin)
    bool load(const std::string& csv_path);  // carga usuarios desde CSV
    std::optional<User> authenticate(const std::string& id, const std::string& clave) const; // verifica clave
private:
    std::unordered_map<std::string, User> by_id_;
};

class CsvLogger {
public:
    explicit CsvLogger(std::string path);
    void log(const User& u, const std::string& cmd_literal, const std::vector<std::string>& args,
             const std::string& status, const std::string& msg = "");
private:
    void ensure_header();
    std::string path_;
    bool initialized_ = false;
};

struct ServerResult {
    bool ok;
    std::string msg;
};

struct IServer {
    virtual ~IServer() = default;
    // Ejecuta el comando EXACTO del enum `comandos`
    virtual ServerResult ejecutar(comandos cmd, const std::vector<std::string>& args, const User& who) = 0;
};

class GUI_Consola {
public:
    GUI_Consola();

    // Config
    void setUserCsv(const std::string& usuarios_csv);
    void setLogCsv(const std::string& log_csv);
    void setServer(std::unique_ptr<IServer> server);

    // Run
    int run(); // 0 OK, !=0 error de setup

    // utilidades
    static std::string now_iso(); //fecha/hora ISO 8601
    static std::vector<std::string> split_ws(const std::string& s);
    static std::string join(const std::vector<std::string>& v, const std::string& sep); //para formatear argumentos
    static bool requiere_admin(comandos c); //marca cuales comandos piden admin

private:

    // presentación
    void imprimir_menu() const;
    std::optional<std::pair<comandos, std::vector<std::string>>>
    parse_command(const std::string& line) const; //parsing comando+args 
    //separa la línea en palabras, busca el comando y devuelve enum+args

    // estado
    std::string usuarios_csv_ = "usuarios.csv";
    std::string log_csv_      = "log_operaciones.csv";
    UserDB db_;
    std::unique_ptr<IServer> server_;
    std::unique_ptr<CsvLogger> logger_;
};

#endif // GUI_CONSOLA_H

