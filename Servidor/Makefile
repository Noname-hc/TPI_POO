includes = Server.cpp 
includes += ../Libreria_RPC/*.cpp 
includes += ../Servidor/G_Code/G_code.cpp 
includes += ../Servidor/Logger/Logger.cpp 

flag_version = -std=c++23

main:  $(includes)
	g++ $(flag_version) $(includes) -o Server.exe
clear:
	rm -f Server.exe
clean:
	rm -f Server.exe
exe:
	./Server.exe 8000
