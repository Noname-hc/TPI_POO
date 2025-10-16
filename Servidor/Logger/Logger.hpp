#pragma once
#include <string>
#include <fstream>

enum class LogDomain
{
    MAIN,
    G_Code
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
        Logger();
        ~Logger();
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        std::string obtenerHoraActual();
        std::string nivelToString(LogLevel);
        std::string domainToString(LogDomain);

    public:
        static Logger& getInstance();
        void log(LogLevel, LogDomain, const std::string);
        void VerLog(LogLevel);
        void abrirLogger();
};