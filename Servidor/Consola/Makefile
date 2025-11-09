# ====== Makefile - Consola Servidor (C++17) ======
# Uso básico:
#   make            # compila en modo release
#   make run        # ejecuta con usuarios.csv y log_operaciones.csv
#   make debug      # build de depuración
#   make clean      # limpia artefactos

# ---------- Config ----------
APP       := consola_servidor
CXX       := g++
CXXFLAGS  := -std=gnu++17 -O2 -Wall -Wextra
LDFLAGS   := 
# Ajustá los -I según la ubicación real de G_code.h:
INCLUDES  := -I. \
             -I../G_Code \
             -I../../Libreria_RPC

# Si necesitás librerías (ej. RPC/serial), agregalas:
# LDFLAGS  += -lws2_32         # (MinGW/Windows) si tu RPC lo requiere
# LDFLAGS  += -lpthread        # (Linux) si tu server usa threads
# LDFLAGS  += -lxmlrpc -lxmlrpc_client -lcurl  # ejemplo RPC (ajustar)

# ---------- Fuentes ----------
# Agregá aquí cualquier otro .cpp que uses (p.ej. tu Adapter real)
SRCS := \
	GUI_Consola.cpp \
	../../Libreria_RPC/XmlRpcClient.cpp \
	../../Libreria_RPC/XmlRpcValue.cpp \
	../../Libreria_RPC/XmlRpcSocket.cpp \
	../../Libreria_RPC/XmlRpcSource.cpp \
	../../Libreria_RPC/XmlRpcDispatch.cpp \
	../../Libreria_RPC/XmlRpcUtil.cpp

# Si compilar falla por símbolos faltantes del lado servidor,
# descomentar estas líneas (no deberían ser necesarias para el cliente):
# SRCS += \
# 	../../Libreria_RPC/XmlRpcServer.cpp \
# 	../../Libreria_RPC/XmlRpcServerConnection.cpp \
# 	../../Libreria_RPC/XmlRpcServerMethod.cpp

OBJS := $(SRCS:.cpp=.o)

# ---------- Reglas ----------
all: release

release: CXXFLAGS += -DNDEBUG
release: $(APP)

debug: CXXFLAGS := -std=gnu++17 -O0 -g3 -Wall -Wextra -DDEBUG
debug: clean $(APP)

$(APP): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

run: $(APP)
	./$(APP) usuarios.csv log_operaciones.csv

# Ejecutar contra servidor real (requiere HOST y PORT)
run_rpc: $(APP)
	@if [ -z "$(HOST)" ] || [ -z "$(PORT)" ]; then \
	  echo "Uso: make run_rpc HOST=127.0.0.1 PORT=8001"; \
	  exit 1; \
	fi
	./$(APP) usuarios.csv log_operaciones.csv $(HOST) $(PORT)

clean:
	$(RM) $(OBJS) $(APP)

.PHONY: all release debug run clean
