VASM=../../tools/vasm6502_oldstyle.exe
ASM=echo.asm
BIN=echo.bin

all: $(BIN)

$(BIN): $(ASM)
	$(VASM) -Fbin -dotdir -o $@ $<

clean:
	rm -f $(BIN) 