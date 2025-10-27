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

  std::cout << "Ingrese un numero del 0 al 1\n\n";
  std::cin >> cont;

 
  switch(cont){
    case 0:
      std::cout << "---------------------------------------------------------------------" << std::endl ;
      std::cout << "*********** Una mirada a los metodos soportados por la API **********" << std::endl ;
      if (c.execute("system.listMethods", parametros, result))
        std::cout << "\nMetodos:\n " << result << "\n\n";
      else
        std::cout << "Error en la llamada a 'listMethods'\n\n";
      break;


    case 2:
      std::cout << "Ingrese un numero para G_Code" << std::endl ;
      cin >> cont;

      std::string stringaux;
      parametros[0] = stringaux;

      std::cout << "********************** Llamada al metodo G_Code **********************" << std::endl ;
      if(cont == 0){
        cout << "Ingrese posicion" << endl;
        cin >> stringaux;
      parametros[1] = stringaux;
      }

      if (c.execute("G_Code", parametros, result))
        std::cout << result << "\n\n";
      else
        std::cout << "Error en la llamada a 'G_Code'\n\n";
      break;
  }
  

  return 0;
}
