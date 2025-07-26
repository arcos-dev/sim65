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
CFLAGS = -Wall -Wextra -O2 -std=c23 -fPIC -DEMU6502_EXPORTS
LDFLAGS = -shared

# Definições para Windows DLL (forçando para Windows)
ifeq ($(OS),Windows_NT)
    # Windows
    LDFLAGS += -Wl,--out-implib,emu6502.lib
    TARGET = emu6502.dll
    CLEAN_FILES = *.dll *.lib *.o
else
    # Detectar Windows via uname ou forçar
    UNAME_S := $(shell uname -s 2>/dev/null || echo "Windows")
    ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
        # MinGW no Windows
        LDFLAGS += -Wl,--out-implib,emu6502.lib
        TARGET = emu6502.dll
        CLEAN_FILES = *.dll *.lib *.o
    else ifeq ($(findstring MSYS,$(UNAME_S)),MSYS)
        # MSYS no Windows
        LDFLAGS += -Wl,--out-implib,emu6502.lib
        TARGET = emu6502.dll
        CLEAN_FILES = *.dll *.lib *.o
    else ifeq ($(UNAME_S),Windows)
        # Assumir Windows se não conseguir detectar
        LDFLAGS += -Wl,--out-implib,emu6502.lib
        TARGET = emu6502.dll
        CLEAN_FILES = *.dll *.lib *.o
    else
        # Linux/Unix
        TARGET = libemu6502.so
        CLEAN_FILES = *.so *.o
    endif
endif

# Diretórios
SRC_DIR = src
BUILD_DIR = build
LIB_DIR = ../lib
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
all: $(BUILD_DIR) $(LIB_DIR) $(LIB_DIR)/$(TARGET)

# Criar diretórios
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

# Compilar e instalar a biblioteca
$(LIB_DIR)/$(TARGET): $(ALL_OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(LDFLAGS) -o $(TARGET) $(ALL_OBJECTS)
	@echo "Installing $(TARGET) to $(LIB_DIR)..."
	@cp $(TARGET) $(LIB_DIR)/
	@echo "Build completed: $(LIB_DIR)/$(TARGET)"

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
	rm -f $(LIB_DIR)/$(TARGET)
	@echo "Clean completed."

# Instalar (já incluído no target principal)
install: all

# Teste da biblioteca
test: all
	@echo "Testing library..."
	python3 -c "import ctypes; lib = ctypes.CDLL('./$(TARGET)'); print('Library loaded successfully!')"

# Testes
test-unit:
	@echo "Running unit tests..."
	python run_tests.py --unit --fast

test-integration:
	@echo "Running integration tests..."
	python run_tests.py --integration

test-gui:
	@echo "Running GUI tests..."
	python run_tests.py --gui --fast

test-gui-headless:
	@echo "Running GUI tests in headless mode..."
	python run_tests.py --gui --headless --fast

test-all:
	@echo "Running all tests..."
	python run_tests.py

test-coverage:
	@echo "Running tests with coverage..."
	python run_tests.py --coverage

# Limpeza completa
clean-all: clean
	@echo "Deep cleaning..."
	python clean.py

# Ajuda
help:
	@echo "Available targets:"
	@echo "  all            - Build the library (default)"
	@echo "  clean          - Remove build files"
	@echo "  clean-all      - Deep clean including Python cache"
	@echo "  install        - Copy library to lib directory"
	@echo "  test           - Test library loading (legacy)"
	@echo "  test-unit      - Run unit tests"
	@echo "  test-integration - Run integration tests"
	@echo "  test-gui       - Run GUI tests"
	@echo "  test-gui-headless - Run GUI tests in headless mode"
	@echo "  test-all       - Run all tests"
	@echo "  test-coverage  - Run tests with coverage"
	@echo "  help           - Show this help"

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: all clean clean-all install test test-unit test-integration test-gui test-gui-headless test-all test-coverage help debug
