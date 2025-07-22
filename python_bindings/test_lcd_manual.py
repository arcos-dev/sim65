#!/usr/bin/env python3
"""Teste manual do LCD"""

print('=== Teste Manual LCD ===')
from emu65_core import Emu65Core
core = Emu65Core()

# Simular inicialização do LCD manualmente
print('1. Simulando comandos LCD...')

# Function Set
core._lib.emu6502_write_byte(core._core, 0x6000, 0x38)  # Command
core._lib.emu6502_write_byte(core._core, 0x6001, 0x20)  # E=1, RS=0
core._lib.emu6502_write_byte(core._core, 0x6001, 0x00)  # E=0, RS=0

# Display ON
core._lib.emu6502_write_byte(core._core, 0x6000, 0x0C)  # Command
core._lib.emu6502_write_byte(core._core, 0x6001, 0x20)  # E=1, RS=0
core._lib.emu6502_write_byte(core._core, 0x6001, 0x00)  # E=0, RS=0

# Clear Display
core._lib.emu6502_write_byte(core._core, 0x6000, 0x01)  # Command
core._lib.emu6502_write_byte(core._core, 0x6001, 0x20)  # E=1, RS=0
core._lib.emu6502_write_byte(core._core, 0x6001, 0x00)  # E=0, RS=0

print('2. Escrevendo "HELLO"...')
# Escrever 'H'
core._lib.emu6502_write_byte(core._core, 0x6000, ord('H'))  # Data
core._lib.emu6502_write_byte(core._core, 0x6001, 0x60)      # E=1, RS=1
core._lib.emu6502_write_byte(core._core, 0x6001, 0x40)      # E=0, RS=1

# Escrever 'E'
core._lib.emu6502_write_byte(core._core, 0x6000, ord('E'))  # Data
core._lib.emu6502_write_byte(core._core, 0x6001, 0x60)      # E=1, RS=1
core._lib.emu6502_write_byte(core._core, 0x6001, 0x40)      # E=0, RS=1

# Escrever 'L'
core._lib.emu6502_write_byte(core._core, 0x6000, ord('L'))  # Data
core._lib.emu6502_write_byte(core._core, 0x6001, 0x60)      # E=1, RS=1
core._lib.emu6502_write_byte(core._core, 0x6001, 0x40)      # E=0, RS=1

# Escrever 'L'
core._lib.emu6502_write_byte(core._core, 0x6000, ord('L'))  # Data
core._lib.emu6502_write_byte(core._core, 0x6001, 0x60)      # E=1, RS=1
core._lib.emu6502_write_byte(core._core, 0x6001, 0x40)      # E=0, RS=1

# Escrever 'O'
core._lib.emu6502_write_byte(core._core, 0x6000, ord('O'))  # Data
core._lib.emu6502_write_byte(core._core, 0x6001, 0x60)      # E=1, RS=1
core._lib.emu6502_write_byte(core._core, 0x6001, 0x40)      # E=0, RS=1

# Verificar LCD
lcd_state = core.get_lcd_state()
if lcd_state:
    display_line1 = lcd_state.display[:16].decode('ascii', errors='ignore').rstrip('\x00')
    print(f'3. LCD Resultado: "{display_line1}"')
else:
    print('3. ERRO: Estado LCD é None')

print('Teste manual concluído.')
