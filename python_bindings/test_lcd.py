#!/usr/bin/env python3
"""Teste de LCD e Hello World"""

print('=== Teste do LCD ===')
from emu65_core import Emu65Core
core = Emu65Core()

# Testar o Hello World gerado internamente primeiro
from programs_6502 import Programs6502
hello_program = Programs6502.hello_world()
print(f'1. Programa Hello World gerado: {len(hello_program["binary"])} bytes')

try:
    core.load_program(hello_program['binary'], hello_program['start_address'])
    print('2. Programa carregado!')

    # Executar alguns steps
    for i in range(100):
        core.step()
        if i % 20 == 0:
            cpu_state = core.get_cpu_state()
            print(f'   Step {i}: PC: 0x{cpu_state.pc:04X}')

    # Verificar estado do LCD
    try:
        lcd_state = core.get_lcd_state()
        if lcd_state:
            display_line1 = lcd_state.display[:16].decode('ascii', errors='ignore').rstrip('\x00')
            print(f'3. LCD Linha 1: "{display_line1}"')
        else:
            print('3. ERRO: Estado LCD é None')
    except Exception as lcd_error:
        print(f'3. ERRO LCD: {lcd_error}')

except Exception as e:
    print(f'ERRO: {e}')
    import traceback
    traceback.print_exc()

print('Teste concluído.')
