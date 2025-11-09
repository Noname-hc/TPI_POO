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
    // SQLite: usa base en carpeta db (p.ej. ../db/users.db)
    bool load_sqlite(const std::string& db_path);  // configura origen como SQLite
private:
    std::unordered_map<std::string, User> by_id_;
    bool use_sqlite_ = false;
    std::string sqlite_db_path_;
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
    // Login remoto contra metodo Inicio; devuelve msg del servidor
    virtual ServerResult login(const std::string& user, const std::string& pass) = 0;
    // Endpoint al que está conectado (para lanzar GUI externa)
    virtual std::pair<std::string,int> endpoint() const = 0;
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
    static std::string build_gcode_literal(comandos c, const std::vector<std::string>& args);

private:

    // presentación
    void imprimir_menu() const;
    std::optional<std::pair<comandos, std::vector<std::string>>>
    parse_command(const std::string& line) const; //parsing comando+args 
    //separa la línea en palabras, busca el comando y devuelve enum+args

    // UI modo "botones" (menu numerado)
    void mostrar_menu_botones(const User& current);
    // Lanzar GUI Tkinter que muestra botones y llama XML-RPC
    void lanzar_gui_botones(const User& current);

    // estado
    std::string usuarios_csv_ = "usuarios.csv";
    std::string log_csv_      = "log_operaciones.csv";
    UserDB db_;
    std::unique_ptr<IServer> server_;
    std::unique_ptr<CsvLogger> logger_;
};

#endif // GUI_CONSOLA_H
