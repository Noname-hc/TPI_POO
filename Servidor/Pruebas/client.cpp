/* client.cpp : codigo auxiliar para TP de un cliente XMLRPC con bucle.
   Uso: client Host Port
*/
#include <iostream>
#include <stdlib.h>
using namespace std;

#include "../../Libreria_RPC/XmlRpcClient.h"
#include "../G_Code/G_code.h"
using namespace XmlRpc;

int main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Uso: miclient IP_HOST N_PORT\n";
    return -1;
  }
  
  int port = atoi(argv[2]);
  //XmlRpc::setVerbosity(5);

  XmlRpcClient c(argv[1], port);

  int cont = 0;
  XmlRpcValue parametros, result;
  XmlRpcValue oneArg;
  XmlRpcValue numbers;

  std::string user;
  std::string contrasena;
  std::string stringaux;

  while(true){
    std::cout << "Ingrese un numero del 0 al 2\n\n";
    std::cin >> cont;

    switch(cont){
      case 0:
        
        std::cout << "ingrese usuario" << std::endl;
        std::cin >> user;

        std::cout << "ingrese contraseña" << std::endl;
        std::cin >> contrasena;

        parametros[0] = user;
        parametros[1] = contrasena;
        c.execute("Inicio", parametros, result);

        std::cout << result << std::endl;

      break;

      case 1:
        std::cout << "Ingrese un numero para G_Code" << std::endl;
        cin >> cont;
        
        if(cont == 0 || cont == 1){
          parametros.setSize(2);
          parametros[0] = cont;

          std::cout << "********************** Llamada al metodo G_Code **********************" << std::endl ;
  
          cout << "Ingrese posicion" << endl;
          cin >> stringaux;
          parametros[1] = stringaux;
          

          if (c.execute("G_Code", parametros, result))
            std::cout << result << "\n\n";
          else
            std::cout << "Error en la llamada a 'G_Code'\n\n";
          break;

        }else{

          try{
            parametros = cont;
            if (c.execute("G_Code", parametros, result)){
              std::cout << result << std::endl;
            }else{
              std::cout << "Error en la llamada a 'G_Code'\n\n";
              break;
            }

          }catch(...){
              std::cerr << "Error al ingresar los parametros: " << std::endl;
          }
        }
      break;
      
      case 2:
        std::cout << "Cuantos parametros utilizara para filtrar el reporte? (0, 1 o 2)" << std::endl;
        cin >> cont;

        switch(cont){
          case 0:
            c.execute("Reporte", parametros, result);
            std::cout << "Reporte:" << std::endl;
            std::cout << result << std::endl;
          break;

          case 1:
            std::cout << "ingrese LogLevel: " << std::endl;
            std::cout << "  0 -> INFO" << std::endl;
            std::cout << "  1 -> WARNING" << std::endl;
            std::cout << "  2 -> ERROR" << std::endl;
            std::cout << "  3 -> DEBUG" << std::endl;
            cin >> cont;
            
            parametros = cont;
            c.execute("Reporte", parametros, result);
            std::cout << "Reporte:" << std::endl;
            std::cout << result << std::endl;
          break;

          case 2:
            std::cout << "ingrese Nivel: " << std::endl;
            std::cout << "  0 -> INFO" << std::endl;
            std::cout << "  1 -> WARNING" << std::endl;
            std::cout << "  2 -> ERROR" << std::endl;
            std::cout << "  3 -> DEBUG" << std::endl;
            cin >> cont;
            parametros[0] = cont;

            std::cout << "ingrese Dominio: " << std::endl;
            std::cout << "  0 -> main" << std::endl;
            std::cout << "  1 -> G_Code" << std::endl;
            std::cout << "  2 -> Reporte" << std::endl;
            std::cout << "  3 -> Inicio" << std::endl;
            cin >> cont;
            parametros[1] = cont;

            c.execute("Reporte", parametros, result);
            std::cout << "Reporte:" << std::endl;
            std::cout << result << std::endl;
          break;
        }

    }
    parametros.clear();
  }

  return 0;
}