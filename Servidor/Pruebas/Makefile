# ==========================================
# Makefile para compilar test_client.cpp
# Genera un único ejecutable: test_cli.exe
# ==========================================

# Compilador y flags
CXX := g++
CXXFLAGS := -Wall -std=c++11 \
            -I../.. \
            -I../../Libreria_RPC \
            -I../G_Code \
            -I../Logger \
            -I../Reporte \
            -I../Serial_Com

# Archivos fuente
LIBRPC_DIR := ../../Libreria_RPC
LIBRPC_SRCS := $(wildcard $(LIBRPC_DIR)/*.cpp)
LIBRPC_OBJS := $(patsubst %.cpp,%.o,$(LIBRPC_SRCS))

# Fuente principal
MAIN_SRC := test_client.cpp
MAIN_OBJ := $(MAIN_SRC:.cpp=.o)

# Nombre del ejecutable
TARGET := test_cli.exe

# Regla por defecto
all: $(TARGET)

# Compilación de la librería RPC
$(LIBRPC_DIR)/%.o: $(LIBRPC_DIR)/%.cpp
	@echo Compilando $<
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compilación del cliente
%.o: %.cpp
	@echo Compilando $<
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link final
$(TARGET): $(MAIN_OBJ) $(LIBRPC_OBJS)
	@echo "Linkeando $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(MAIN_OBJ) $(LIBRPC_OBJS)

# Limpieza
clean:
	@echo "Limpiando..."
	rm -f $(TARGET) $(MAIN_OBJ) $(LIBRPC_OBJS)

.PHONY: all clean
