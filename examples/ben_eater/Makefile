# Makefile for Ben Eater 6502 Examples
# Compiles all example programs using asm6f assembler

ASM6F = ../../tools/asm6f/asm6f.exe

# List of all example programs
EXAMPLES = echo binary_counter hello_lcd fibonacci interrupt_timer \
          basic_interpreter clock memory_test calculator

# Binary targets
BINS = $(EXAMPLES:=.bin)

# Default target - build all examples
all: $(BINS)

# Individual program rules
echo.bin: echo.asm
	$(ASM6F) $< $@

binary_counter.bin: binary_counter.asm
	$(ASM6F) $< $@

hello_lcd.bin: hello_lcd.asm
	$(ASM6F) $< $@

fibonacci.bin: fibonacci.asm
	$(ASM6F) $< $@

interrupt_timer.bin: interrupt_timer.asm
	$(ASM6F) $< $@

basic_interpreter.bin: basic_interpreter.asm
	$(ASM6F) $< $@

clock.bin: clock.asm
	$(ASM6F) $< $@

memory_test.bin: memory_test.asm
	$(ASM6F) $< $@

calculator.bin: calculator.asm
	$(ASM6F) $< $@

# Clean all generated binaries
clean:
	@echo "Cleaning Ben Eater examples..."
	@if exist *.bin del *.bin
	@echo "Clean completed!"

# Test a specific program with the main simulator
test: $(EXAMPLE).bin
	@echo "Testing $(EXAMPLE).bin with simulator..."
	@cd ../.. && sim65.exe examples/ben_eater/$(EXAMPLE).bin

# Run a specific example (usage: make run EXAMPLE=binary_counter)
run:
	@echo "Running $(EXAMPLE).bin..."
	@cd ../.. && sim65.exe examples/ben_eater/$(EXAMPLE).bin

# List all available examples
list:
	@echo "Available Ben Eater examples:"
	@echo "  echo - Serial echo using VIA 6522"
	@echo "  binary_counter - Binary counter with LEDs"
	@echo "  hello_lcd - Hello World on LCD display"
	@echo "  fibonacci - Fibonacci sequence calculator"
	@echo "  interrupt_timer - Timer-based LED patterns"
	@echo "  basic_interpreter - Simple BASIC interpreter"
	@echo "  clock - Digital clock/stopwatch"
	@echo "  memory_test - RAM memory testing"
	@echo "  calculator - Simple 4-bit calculator"

# Help target
help:
	@echo "Ben Eater 6502 Examples Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build all examples"
	@echo "  clean        - Remove all .bin files"
	@echo "  list         - List all available examples"
	@echo "  run          - Run specific example (use EXAMPLE=name)"
	@echo "  <name>.bin   - Build specific example"
	@echo ""
	@echo "Examples:"
	@echo "  make all"
	@echo "  make binary_counter.bin"
	@echo "  make run EXAMPLE=fibonacci"
	@echo "  make test EXAMPLE=clock"

.PHONY: all clean list help run test