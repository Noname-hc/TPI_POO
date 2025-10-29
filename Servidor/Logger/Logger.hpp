#pragma once
#include <string>
#include <fstream>

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