includes = Server.cpp 
includes += ../Libreria_RPC/*.cpp 
includes += ../Servidor/Inicio/Inicio.cpp 
includes += ../Servidor/G_Code/G_code.cpp 
includes += ../Servidor/Logger/Logger.cpp 
includes += ../Servidor/Reporte/Reporte.cpp
includes += ../Servidor/Serial_Com/Serial_Com.cpp

flag_version = -std=c++23

main:  $(includes)
	g++ $(flag_version) $(includes) -o Server.exe
clear:
	rm -f Server.exe
clean:
	rm -f Server.exe
exe:
	./Server.exe 8001
