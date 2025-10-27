includes = client.cpp 
includes += ../G_Code/G_code.cpp
includes += ../../Libreria_RPC/*.cpp
includes += ../Logger/Logger.cpp
flag_version = -std=c++23

main:  $(includes)
	g++ $(flag_version) $(includes) -o prueba.exe
clear:
	rm -f prueba.exe
clean:
	rm -f prueba.exe
exe:
	./prueba.exe 8000
