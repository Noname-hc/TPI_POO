#include "Logger.hpp"


int main(int argc, char const *argv[])
{


    Logger& logger = Logger::getInstance();
    logger.VerLog(LogLevel::INFO);
    
    return 0;
}