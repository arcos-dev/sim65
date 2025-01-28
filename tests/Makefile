# Define the compiler to use (GCC)
CC := gcc

# Compiler flags for optimization and warnings
# -flto: Enables link-time optimization
# -march=native: Optimizes for the host machine's CPU architecture
# -O3: High-level optimization
# -W, -Wall, -Wextra: Enable various warning levels
CFLAGS := -pthread -flto -march=native -O3 -W -Wall -Wextra -g

# Default binary name
TARGET := test

# Source files
SRCS := test.c ../cpu.c ../memory.c ../clock.c bus.c

# Object files (automatically generated from source files)
OBJS := $(SRCS:.c=.o)

# Detect OS and adjust file extension, clean command, etc.
ifeq ($(OS),Windows_NT)
	SHELL := cmd.exe
	EXT := .exe
	CLEAN := if exist $(TARGET)$(EXT) del /Q $(TARGET)$(EXT) $(OBJS)
else
	# UNIX-based systems (Linux, macOS)
	EXT :=
	CLEAN := rm -f $(TARGET) $(OBJS)
endif

# Target: Default action when 'make' is run without arguments
all: $(TARGET)$(EXT)

# Rule to build the executable
# This compiles and links the source files into the 'test' executable
$(TARGET)$(EXT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up generated files
clean:
	$(CLEAN)

# Phony targets ensure that make does not confuse target names with file names
.PHONY: all clean
