#pragma once
#include <string>
#include <fstream>

// Evitar colisiones con macros comunes (DEBUG/ERROR/INFO/WARNING) que rompen los enums
#ifdef DEBUG
# pragma push_macro("DEBUG")
# undef DEBUG
# define LOGGER_POP_DEBUG 1
#endif
#ifdef ERROR
# pragma push_macro("ERROR")
# undef ERROR
# define LOGGER_POP_ERROR 1
#endif
#ifdef INFO
# pragma push_macro("INFO")
# undef INFO
# define LOGGER_POP_INFO 1
#endif
#ifdef WARNING
# pragma push_macro("WARNING")
# undef WARNING
# define LOGGER_POP_WARNING 1
#endif

enum class LogDomain
{
    MAIN,
    G_Code,
    Reporte,
    Inicio
    //, editar según sea necesario
};

enum class LogLevel
{
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

class Logger
{
    private:
        std::ofstream logFile;
        std::string path;

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        std::string nivelToString(LogLevel);
        std::string domainToString(LogDomain);

    public:
        Logger(const std::string &path);
        ~Logger();
        std::string obtenerHoraActual();
        void log(LogLevel, LogDomain, const std::string);

        void VerLog();
        void VerLog(LogLevel);
        void VerLog(LogLevel nivel, LogDomain dominio);

        std::string getMsj(LogLevel nivel, LogDomain dominio);
        std::string getMsj(LogLevel nivel);
        std::string getMsj();
        void abrirLogger();
};

// Restaurar macros si estaban definidas
#ifdef LOGGER_POP_DEBUG
# pragma pop_macro("DEBUG")
# undef LOGGER_POP_DEBUG
#endif
#ifdef LOGGER_POP_ERROR
# pragma pop_macro("ERROR")
# undef LOGGER_POP_ERROR
#endif
#ifdef LOGGER_POP_INFO
# pragma pop_macro("INFO")
# undef LOGGER_POP_INFO
#endif
#ifdef LOGGER_POP_WARNING
# pragma pop_macro("WARNING")
# undef LOGGER_POP_WARNING
#endif
