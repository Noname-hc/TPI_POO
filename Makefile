main: ./Servidor/G_code.cpp ./Servidor/Logger/Logger.cpp main.cpp
	g++ -std=c++23 ./Servidor/G_code.cpp ./Servidor/Logger/Logger.cpp main.cpp -o Server.exe
clear:
	rm -f Server.exe
clean:
	rm -f Server.exe
Ex:
	./Server.exe