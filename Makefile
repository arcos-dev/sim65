# Makefile to compile project 6502 in C (Windows)

# Compiler and Flags
CC      = gcc
CFLAGS  = -Wall -Wextra -O2

# Executable name (without .exe)
TARGET  = sim65

# Source files and objects
SRC     = main.c acia.c tia.c memory.c clock.c bus.c cpu.c
OBJ     = $(SRC:.c=.o)

# Default rule
all: $(TARGET)

# Links the object files and generates the .exe executable
$(TARGET): $(OBJ)
	@echo "Linking objects..."
	$(CC) -o $(TARGET).exe $(OBJ)
	@echo "Build completed! -> $(TARGET).exe"

# Compiles each .c file into .o
%.o: %.c
	@echo "Compiling $<..."
	$(CC) -c $< -o $@ $(CFLAGS)

# Cleaning object and executable files
clean:
	@echo "Cleaning up..."
	cmd.exe del $(TARGET).exe $(OBJ)
	@echo "Clean completed."

# Execute the binary
run: $(TARGET)
	@echo "Running the project..."
	./$(TARGET).exe

# Rebuild the project
rebuild: clean all

.PHONY: all clean run rebuild
