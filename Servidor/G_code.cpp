#include <cstring>
#include <iostream>
#include <vector>

#include "G_code.h"

G_Code::G_Code(std::string &pos_str){
    this->pos_str = pos_str;
}
G_Code::~G_Code(){
    this->pos_str.clear();
}
void G_Code::setPosicion(std::string &posicion_str){
    this->pos_str = posicion_str;
}


void G_Code::Transformar_Pos(){
    if(pos_str.size() == 0){

        return;
    }
    std::string str_aux = "";

    str_aux = pos_str;

}
