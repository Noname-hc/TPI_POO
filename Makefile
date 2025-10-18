main: ./Libreria_RPC/*.cpp ./Servidor/G_code/G_code.cpp ./Servidor/Logger/Logger.cpp Server.cpp
	g++ -std=c++23 ./Libreria_RPC/*.cpp ./Servidor/G_code/G_code.cpp ./Servidor/Logger/Logger.cpp Server.cpp -o Server.exe
clear:
	rm -f Server.exe
clean:
	rm -f Server.exe
Ex:
	./Server.exe