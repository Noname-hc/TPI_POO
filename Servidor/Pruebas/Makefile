includes = client.cpp 
includes += ../../Libreria_RPC/*.cpp 
includes += ../Inicio/Inicio.cpp 
includes += ../G_Code/G_code.cpp 
includes += ../Logger/Logger.cpp 
includes += ../Reporte/Reporte.cpp
includes += ../Serial_Com/Serial_Com.cpp

flag_version = -std=c++23

main:  $(includes)
	g++ $(flag_version) $(includes) -o test_client.exe
clear:
	rm -f test_client.exe
clean:
	rm -f test_client.exe
exe:
	./test_client.exe 127.0.0.1 8001