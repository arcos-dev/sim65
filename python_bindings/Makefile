# Makefile para a biblioteca emu6502 Python Bindings
# ==================================================
#
# Compila a biblioteca compartilhada que conecta o emulador sim65
# com a interface Python existente.
#
# Autor: Anderson Costa
# Data: 2025-01-21

# Configuração do compilador
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c23 -fPIC
LDFLAGS = -shared

# Definições para Windows DLL
ifdef OS
    # Windows (usando MinGW)
    CFLAGS += -DEMU6502_EXPORTS
    LDFLAGS += -Wl,--out-implib,emu6502.lib
    TARGET = emu6502.dll
    CLEAN_FILES = *.dll *.lib *.o
else
    # Linux/Unix
    TARGET = libemu6502.so
    CLEAN_FILES = *.so *.o
endif

# Diretórios
SRC_DIR = src
BUILD_DIR = build
SIM65_SRC = ../src

# Arquivos fonte
API_SOURCES = $(SRC_DIR)/emu6502_api.c
SIM65_SOURCES = $(SIM65_SRC)/cpu.c $(SIM65_SRC)/memory.c $(SIM65_SRC)/bus.c \
                $(SIM65_SRC)/via.c $(SIM65_SRC)/clock.c $(SIM65_SRC)/acia.c $(SIM65_SRC)/tia.c

ALL_SOURCES = $(API_SOURCES) $(SIM65_SOURCES)

# Arquivos objeto
API_OBJECTS = $(BUILD_DIR)/emu6502_api.o
SIM65_OBJECTS = $(BUILD_DIR)/cpu.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/bus.o \
                $(BUILD_DIR)/via.o $(BUILD_DIR)/clock.o $(BUILD_DIR)/acia.o $(BUILD_DIR)/tia.o

ALL_OBJECTS = $(API_OBJECTS) $(SIM65_OBJECTS)

# Diretórios de include
INCLUDES = -I$(SRC_DIR) -I$(SIM65_SRC)

# Target principal
all: $(BUILD_DIR) $(TARGET)

# Criar diretório de build
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Compilar a biblioteca
$(TARGET): $(ALL_OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(LDFLAGS) -o $@ $(ALL_OBJECTS)
	@echo "Build completed: $(TARGET)"

# Compilar arquivos da API
$(BUILD_DIR)/emu6502_api.o: $(SRC_DIR)/emu6502_api.c $(SRC_DIR)/emu6502_api.h
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compilar arquivos do sim65
$(BUILD_DIR)/cpu.o: $(SIM65_SRC)/cpu.c $(SIM65_SRC)/cpu.h
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/memory.o: $(SIM65_SRC)/memory.c $(SIM65_SRC)/memory.h
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/bus.o: $(SIM65_SRC)/bus.c $(SIM65_SRC)/bus.h
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/via.o: $(SIM65_SRC)/via.c $(SIM65_SRC)/via.h
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/clock.o: $(SIM65_SRC)/clock.c $(SIM65_SRC)/clock.h
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/acia.o: $(SIM65_SRC)/acia.c $(SIM65_SRC)/acia.h
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/tia.o: $(SIM65_SRC)/tia.c $(SIM65_SRC)/tia.h
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Limpeza
clean:
	@echo "Cleaning build files..."
	rm -f $(CLEAN_FILES)
	rm -rf $(BUILD_DIR)
	@echo "Clean completed."

# Instalar (copiar para diretório lib)
install: $(TARGET)
	@echo "Installing $(TARGET)..."
	@mkdir -p ../lib
	cp $(TARGET) ../lib/
	@echo "Installation completed."

# Teste da biblioteca
test: $(TARGET)
	@echo "Testing library..."
	python3 -c "import ctypes; lib = ctypes.CDLL('./$(TARGET)'); print('Library loaded successfully!')"

# Ajuda
help:
	@echo "Available targets:"
	@echo "  all      - Build the library (default)"
	@echo "  clean    - Remove build files"
	@echo "  install  - Copy library to lib directory"
	@echo "  test     - Test library loading"
	@echo "  help     - Show this help"

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: all clean install test help debug
