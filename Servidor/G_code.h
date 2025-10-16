#ifndef _G_CODE_
#define _G_CODE_

#include <cstring>
#include <iostream>
#include <vector>

#include "../Servidor/Logger/Logger.hpp"

enum class comandos
{
    Movimiento_Lineal = 0,
    Rampa = 4,
    Homing = 28,
    C_Absolutas = 90,
    C_Relativas = 91,
    Set_0 = 92,
    Bomba_ON = 10001,
    Bomba_OFF = 10002,
    Griper_ON = 10003,
    Griper_OFF = 10005,
    Laser_ON = 10006,
    Laser_OFF = 10007,
    Motor_ON = 100017,
    Motor_OFF = 100018,
    Ventiladores_ON = 1000106,
    Ventiladores_OFF = 1000107,
    Reporte_General = 1000114,
    Reporte_finales = 1000119,
};

class G_Code{
    public:
        G_Code(Logger *log = nullptr);

        ~G_Code();

        void setLog(Logger *log = nullptr);
        // ==============================================================================================================
        std::string Interpretar(comandos cmnd, const std::string &posicion = "");
        
    private:
        Logger *log = nullptr;


};


#endif