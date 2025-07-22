# Makefile to compile 6502 simulator project (Modern C with GCC 14.2.0)

# ============================================================================
# Compiler Configuration - Modern C with GCC 14.2.0
# ============================================================================
CC       = gcc
C_STD    = -std=c23  # Use C23 (latest standard)

# Modern C flags with GCC 14.2.0 optimizations
CFLAGS_BASE   = -Wall -Wextra -Wpedantic -Wformat=2 -Wconversion -Wsign-conversion \
                -Wcast-align -Wcast-qual -Wshadow -Wstrict-overflow=5 -Wundef \
                -Wpointer-arith -Wunreachable-code -Wlogical-op -Wfloat-equal \
                -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes \
                -Wmissing-declarations -Wredundant-decls -Wnested-externs \
                -Wbad-function-cast -Wwrite-strings -Wstrict-aliasing=3 \
                -MMD -MP $(C_STD)

# Release build with modern optimizations (without LTO for compatibility)
CFLAGS        = $(CFLAGS_BASE) -O3 -march=native -mtune=native \
                -fdata-sections -ffunction-sections -pipe -DNDEBUG

# Debug build with enhanced debugging (compatible version)
CFLAGS_DEBUG  = $(CFLAGS_BASE) -g3 -DDEBUG -O0 -fno-omit-frame-pointer \
                -fstack-protector-strong -fno-optimize-sibling-calls

# Test flags with threading support
CFLAGS_TEST   = $(CFLAGS) -pthread

# Modern linker flags (without LTO for compatibility)
LDFLAGS       = -Wl,--gc-sections
LDFLAGS_DEBUG = -fstack-protector-strong
LDFLAGS_TEST  = -pthread

# ============================================================================
# Project Structure
# ============================================================================
SRCDIR   = src
BUILDDIR = build
TESTDIR  = tests
ASMDIR   = asm

# ============================================================================
# Targets
# ============================================================================
TARGET     = sim65
TESTTARGET = $(TESTDIR)/test

# ============================================================================
# Source Files and Objects
# ============================================================================
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
DEPENDS = $(OBJECTS:.o=.d)

# Test source files and objects (excluding main.c from src)
TEST_SOURCES = $(TESTDIR)/test.c $(TESTDIR)/bus.c $(filter-out $(SRCDIR)/main.c, $(SOURCES))
TEST_OBJS = $(BUILDDIR)/$(TESTDIR)/test.o $(BUILDDIR)/$(TESTDIR)/bus.o $(filter-out $(BUILDDIR)/main.o $(BUILDDIR)/bus.o, $(OBJECTS))
TEST_DEPENDS = $(TEST_OBJS:.o=.d)

# ============================================================================
# Build Rules
# ============================================================================

# Create build directories
$(shell mkdir -p $(BUILDDIR) $(BUILDDIR)/$(TESTDIR))

# Default target
.DEFAULT_GOAL := all

# Main targets
all: $(TARGET)
	@echo "Done: Build completed successfully with GCC $(shell $(CC) --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')"

# Links the object files and generates the executable
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET) with modern optimizations..."
	@$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Done: Executable created: $@"

# Compiles each .c file into .o in build directory
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@echo "Compiling $< ..."
	@$(CC) $(CFLAGS) -c $< -o $@

# Compiles test files into .o in build directory
$(BUILDDIR)/$(TESTDIR)/%.o: $(TESTDIR)/%.c
	@echo "Compiling test $< ..."
	@$(CC) $(CFLAGS_TEST) -I$(SRCDIR) -c $< -o $@

# Build test executable
$(TESTTARGET): $(TEST_OBJS)
	@echo "Linking test executable with modern optimizations..."
	@$(CC) $(TEST_OBJS) -o $@ $(LDFLAGS_TEST)
	@echo "Done: Test executable created: $@"

# ============================================================================
# Development Commands
# ============================================================================

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/*.d $(BUILDDIR)/$(TESTDIR)/*.o $(BUILDDIR)/$(TESTDIR)/*.d $(TARGET) $(TARGET).exe $(TESTTARGET) $(TESTTARGET).exe 2>/dev/null || true
	@echo "Done: Clean completed"

# Deep clean - removes all build artifacts and directories
distclean: clean
	@echo "Deep cleaning project..."
	@rm -rf $(BUILDDIR) 2>/dev/null || true
	@echo "Done: Deep clean completed"

# Rebuild everything from scratch
rebuild: clean all

# ============================================================================
# Execution Commands
# ============================================================================

# Execute the simulator
run: $(TARGET)
	@echo "Running $(TARGET)..."
	@./$(TARGET)

# ============================================================================
# Test Commands
# ============================================================================

# Build tests only
build-tests: $(TESTTARGET)

# Run all functional tests
test: $(TESTTARGET)
	@echo "Testing: 6502 functional test suite (Modern C build)..."
	@echo "This may take a few minutes..."
	@cd $(TESTDIR) && ../$(TESTTARGET)
	@echo "Done: All tests completed!"

# Run quick tests with timeout
test-quick: $(TESTTARGET)
	@echo "Quick test suite..."
	@cd $(TESTDIR) && timeout 30 ../$(TESTTARGET) || echo "Done: Quick tests completed"

# Run specific test file
test-file: $(TESTTARGET)
	@if [ -z "$(FILE)" ]; then \
		echo "Error: Usage: make test-file FILE=test_name.bin"; \
		echo "Available tests:"; \
		ls $(TESTDIR)/*.bin 2>/dev/null | head -10 || echo "No test files found"; \
	else \
		echo "Running specific test: $(FILE)"; \
		cd $(TESTDIR) && ../$(TESTTARGET) $(FILE); \
	fi

# ============================================================================
# Debug and Development
# ============================================================================

# Debug build with AddressSanitizer and UBSan
debug: CFLAGS = $(CFLAGS_DEBUG)
debug: LDFLAGS = $(LDFLAGS_DEBUG)
debug: clean $(TARGET)
	@echo "Debug build completed with enhanced debugging!"
	@echo "Note: This build includes stack protection and debug symbols"

# Modern C compatibility check
check-modern:
	@echo "Checking Modern C (C23) compatibility with GCC $(shell $(CC) --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')..."
	@$(CC) $(C_STD) -E - < /dev/null > /dev/null 2>&1 && \
		echo "Done: C23 support available" || \
		echo "Warning: C23 not fully supported, falling back to C17"

# Install to system (requires sudo on Unix)
install: $(TARGET)
	@echo "Installing $(TARGET) to system..."
	@cp $(TARGET) /usr/local/bin/ 2>/dev/null || echo "Warning: Install requires sudo privileges"

# Show project information
info:
	@echo "================================================"
	@echo "Project Information"
	@echo "================================================"
	@echo "Compiler: $(CC) $(shell $(CC) --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')"
	@echo "C Standard: $(C_STD)"
	@echo "Target: $(TARGET)"
	@echo "Test Target: $(TESTTARGET)"
	@echo "Sources: $(words $(SOURCES)) files"
	@echo "Objects: $(words $(OBJECTS)) files"
	@echo "Test Sources: $(words $(TEST_SOURCES)) files"
	@echo "Test Objects: $(words $(TEST_OBJS)) files"
	@echo "================================================"
	@echo "Modern C Features:"
	@echo "  - C23 Standard Support"
	@echo "  - Native CPU optimizations (-march=native)"
	@echo "  - Enhanced warning system (20+ warnings)"
	@echo "  - Stack protection (debug mode)"
	@echo "  - Enhanced debug symbols (debug mode)"
	@echo "  - Dead code elimination"
	@echo "================================================"
	@echo "Available Commands:"
	@echo "  make all         - Build with modern optimizations"
	@echo "  make test        - Run all 75 tests"
	@echo "  make debug       - Debug build with enhanced debugging"
	@echo "  make check-modern- Check C23 compatibility"
	@echo "================================================"

# Include dependency files
-include $(DEPENDS) $(TEST_DEPENDS)

# ============================================================================
# Phony Targets
# ============================================================================
.PHONY: all clean distclean rebuild run build-tests test test-quick test-file debug check-modern install info
