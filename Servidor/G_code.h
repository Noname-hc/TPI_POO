#ifndef _G_CODE_
#define _G_CODE_

#include <cstring>
#include <iostream>
#include <vector>

struct Pos{
    double x;
    double y;
    double z;
};

class G_Code{
    public:
        G_Code(std::string &pos_str);
        ~G_Code();

        void setPosicion(std::string &posicion_str);
        // ==============================================================================================================

        void Transformar_Pos(); // Transforma el string de posicion a una estructura tipo Pos
        Pos getPos();
        
    private:
        std::string pos_str = "";
        Pos posicion;


};


#endif