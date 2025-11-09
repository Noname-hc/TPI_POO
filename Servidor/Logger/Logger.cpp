#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <filesystem>
#include <cerrno>
#include <cstring>

#include "Logger.hpp"

Logger::Logger(const std::string &path)
{
    this->path = path;
    // Asegurar que el directorio exista
    std::filesystem::path p(path);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }
    logFile.open(path, std::ios::app);
    if(!logFile.is_open()){
        throw std::runtime_error(std::string("No se pudo abrir ") + path + ": " + std::strerror(errno));
    }
}

Logger::~Logger()
{
    if(logFile.is_open())
        {
            logFile.close();
        }
}

void Logger::abrirLogger(){
    if(logFile.is_open()){
        logFile.close();
    }
    logFile.clear(); // limpiar flags
    // Asegurar directorio antes de abrir
    std::filesystem::path p(path);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }
    logFile.open(path, std::ios::app);
    if(!logFile.is_open()){
        throw std::runtime_error(std::string("No se pudo abrir ") + path + ": " + std::strerror(errno));
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
        case LogDomain::Inicio: return "Inicio";
        case LogDomain::Reporte: return "Reporte";
        //case domain::...: return "..."; // editar según sea necesario
        default: return "UNKNOWN";
    }
}

void Logger::log(LogLevel nivel,LogDomain dominio, const std::string mensaje)
{
    abrirLogger();
    std::string hora = obtenerHoraActual();
    std::string status = nivelToString(nivel);
    std::string domain = domainToString(dominio);


    if(!logFile.is_open()){
        throw std::runtime_error(std::string("No se pudo escribir en el archivo de log: ") + path);
    }
    logFile << "[" << hora << "],[" << status << "],[" << domain <<"]," << mensaje << std::endl;
    logFile.flush();
}

// ===================== Visualizacion =========================================================
void Logger::VerLog(){
    if(logFile.is_open()){
        logFile.close();
    }
    std::ifstream fs(path, std::ios::in);
    std::string str_aux = "";

    while (getline(fs, str_aux)){
        std::cout << str_aux << std::endl;
    }
    
    fs.close();
}

void Logger::VerLog(LogLevel nivel){
    if(logFile.is_open()){
        logFile.close();
    }
    std::ifstream fs(path, std::ios::in);

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

void Logger::VerLog(LogLevel nivel, LogDomain dominio){
    if(logFile.is_open()){
        logFile.close();
    }
    std::ifstream fs(path, std::ios::in);

    std::string lvl = nivelToString(nivel);
    std::string dom = domainToString(dominio);
    size_t pos = 0;
    size_t pos_2 = 0;

    std::string str_aux = "";
    while (getline(fs, str_aux)){
        pos = str_aux.find("["+lvl+"]");
        pos_2 = str_aux.find("["+dom+"]");

        if(pos != std::string::npos && pos_2 != std::string::npos){
            std::cout << str_aux << std::endl;
        }
    }
    
    fs.close();
}

std::string Logger::getMsj(){
    if(logFile.is_open()){
        logFile.close();
    }
    std::ifstream fs(path, std::ios::in);

    std::string str_aux = "";
    std::string resultado = "";

    while (getline(fs, str_aux)){
        resultado += str_aux + "\n";
    }
    
    fs.close();
    return resultado;
}

std::string Logger::getMsj(LogLevel nivel){
    if(logFile.is_open()){
        logFile.close();
    }
    std::ifstream fs(path, std::ios::in);

    std::string lvl = nivelToString(nivel);
    size_t pos = 0;

    std::string str_aux = "";
    std::string resultado = "";

    while (getline(fs, str_aux)){
        pos = str_aux.find("["+lvl+"]");

        if(pos != std::string::npos){
            resultado += str_aux + "\n";
        }
    }
    
    fs.close();
    return resultado;
}

std::string Logger::getMsj(LogLevel nivel, LogDomain dominio){
    if(logFile.is_open()){
        logFile.close();
    }
    std::ifstream fs(path, std::ios::in);

    std::string lvl = nivelToString(nivel);
    std::string dom = domainToString(dominio);
    size_t pos = 0;
    size_t pos_2 = 0;

    std::string str_aux = "";
    std::string resultado = "";

    while (getline(fs, str_aux)){
        pos = str_aux.find("["+lvl+"]");
        pos_2 = str_aux.find("["+dom+"]");

        if(pos != std::string::npos && pos_2 != std::string::npos){
            resultado += str_aux + "\n"; 
        }
    }
    
    fs.close();
    return resultado;
}