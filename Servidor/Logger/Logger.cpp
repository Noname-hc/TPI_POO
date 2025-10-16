#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#include "Logger.hpp"

Logger::Logger()
{
    std::cout<< "Creando Logger/Logger.log" <<std::endl;
    logFile.open("../logs/Logger.log", std::ios::app);
}

Logger::~Logger()
{
    std::cout<< "Destruyendo Logger" <<std::endl;
    if(logFile.is_open())
        {
            logFile.close();
        }
}

void Logger::abrirLogger(){
    const std::string path = "logs/Logger.log";
    if(logFile.is_open()){
        logFile.close();
    }
    logFile.clear(); // limpiar flags
    logFile.open(path, std::ios::app);
    if(!logFile.is_open()){
        std::cerr << "No se pudo abrir " << path << std::endl;
    }
}

std::string Logger::obtenerHoraActual() 
{
    std::time_t ahora = std::time(nullptr);
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&ahora));
    return std::string(buffer);
}

std::string Logger::nivelToString(LogLevel nivel) 
{
    switch (nivel) 
    {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

std::string Logger::domainToString(LogDomain dom) 
{
    switch (dom) 
    {
        case LogDomain::MAIN: return "MAIN";
        case LogDomain::G_Code: return "G_Code";
        //case domain::...: return "..."; // editar según sea necesario
        default: return "UNKNOWN";
    }
}


Logger& Logger::getInstance() 
{
    static Logger instance; 
    return instance;
}

void Logger::log(LogLevel nivel,LogDomain dominio, const std::string mensaje)
{
    abrirLogger();
    std::string hora = obtenerHoraActual();
    std::string status = nivelToString(nivel);
    std::string domain = domainToString(dominio);


    if(logFile.is_open())
    {
        logFile << "[" << hora << "] [" << status << "] [" << domain <<"] " << mensaje << std::endl;
        logFile.flush();
    }
    else
    {
        std::cerr << "No se pudo escribir en el archivo de log." << std::endl;
    }
}


void Logger::VerLog(LogLevel nivel){
    if(logFile.is_open()){
        logFile.close();
    }
    std::fstream fs("../logs/Logger.log", std::ios::in);

    std::string lvl = nivelToString(nivel);
    size_t pos = 0;

    std::string str_aux = "";
    while (getline(fs, str_aux)){
        pos = str_aux.find("["+lvl+"]");

        if(pos != std::string::npos){
            std::cout << str_aux << std::endl;
        }
    }
    
    fs.close();
}