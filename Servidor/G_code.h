#ifndef _G_CODE_
#define _G_CODE_

#include <cstring>
#include <iostream>
#include <vector>

#include "../Logger/Logger.hpp"

struct Pos{
    double x;
    double y;
    double z;
};

class G_Code{
    public:
        G_Code(const std::string &pos_str = "");
        G_Code(const std::string &pos_str, Logger &log);
        ~G_Code();

        void setLog(Logger &log);
        void setPosicion(const std::string &posicion_str);
        // ==============================================================================================================

        void Transformar_Pos(); // Transforma el string de posicion a una estructura tipo Pos
        Pos getPos();
        
    private:
        std::string pos_str = "";
        Pos posicion;
        Logger *log = nullptr;


};


#endif