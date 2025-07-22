# Makefile for compiling C projects with Raylib on Windows

# Path to Raylib (replace with the correct path)
RAYLIB_PATH = D:/Code/c/lib/raylib

# Compiler and Flags
CC = gcc
CFLAGS = -I$(RAYLIB_PATH)/include -I../src -DRAYGUI_IMPLEMENTATION -DBUILD_LIBTYPE_SHARED -Wall -Wextra -O2 -Wno-unused-parameter -std=c23
LDFLAGS = -L$(RAYLIB_PATH)/lib
LIBS = -lraylib -lpthread -lopengl32 -lgdi32 -lwinmm -static

# Executable Name
TARGET = tia

# Source Files (usando os arquivos do emulador principal em src/)
SRC = main.c ../src/acia.c ../src/tia.c ../src/memory.c ../src/clock.c ../src/bus.c ../src/cpu.c ../src/via.c

# Object Files
OBJ = $(SRC:.c=.o)

# Default Rule
all: $(TARGET)

# Link Object Files to Create Executable
$(TARGET): $(OBJ)
	@echo "Compiling the project..."
	$(CC) -o $(TARGET).exe $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIBS)
	@echo "Build completed! Generated executable: $(TARGET).exe"

# Compile .c files to .o object files
%.o: %.c
	@echo "Compiling $<..."
	$(CC) -c $< -o $@ $(CFLAGS)

# Clean Generated Files
clean:
	@echo "Cleaning files..."
	@if exist $(TARGET).exe del $(TARGET).exe
	@if exist main.o del main.o
	@if exist "../src/acia.o" del "../src/acia.o"
	@if exist "../src/tia.o" del "../src/tia.o"
	@if exist "../src/memory.o" del "../src/memory.o"
	@if exist "../src/clock.o" del "../src/clock.o"
	@if exist "../src/bus.o" del "../src/bus.o"
	@if exist "../src/cpu.o" del "../src/cpu.o"
	@if exist "../src/via.o" del "../src/via.o"
	@echo "Cleaning completed!"

# Run the Executable
run: $(TARGET)
	@echo "Running the project..."
	./$(TARGET).exe

# Rebuild the Executable
rebuild: clean all

# Phony Targets
.PHONY: all clean run rebuild
