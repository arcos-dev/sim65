#!/usr/bin/env python3
"""
Teste completo do Hello World LCD
================================
"""
import sys, os
sys.path.insert(0, os.path.abspath('..'))
from emu65_core import Emu65Core
from programs_6502 import Programs6502

print("=== TESTE COMPLETO HELLO WORLD ===")

# 1. Criar o core
core = Emu65Core({'debug_mode': False})

# 2. Obter o programa Hello World (gerado dinamicamente)
hello_world = Programs6502.hello_world()

print(f"Programa encontrado: {hello_world['name']}")
print(f"Descrição: {hello_world['description']}")
print(f"Tamanho: {len(hello_world['binary'])} bytes")
print(f"Endereço inicial: 0x{hello_world['start_address']:04X}")
print(f"Hex dump (primeiros 32 bytes): {hello_world['binary'][:32].hex()}")

# 3. Carregar e executar o programa
print("\nCarregando programa...")
core.reset()
core.load_program(hello_world['binary'], hello_world['start_address'])
core.reset()

print("Executando programa (máximo 200 steps)...")
step_count = 0
last_pc = 0
for step in range(200):
    try:
        result = core.step()
        step_count += 1
        bus_state = core.get_bus_state()
        cpu_state = core.get_cpu_state()

        # Mostrar PC para debug
        if step < 10 or cpu_state.pc != last_pc:
            print(f"Step {step:3d}: PC=0x{cpu_state.pc:04X}, addr=0x{bus_state.address:04X}, data=0x{bus_state.data:02X}")
        last_pc = cpu_state.pc

        # Mostrar apenas acessos relevantes ao LCD
        if 0x6000 <= bus_state.address <= 0x6003:
            if bus_state.address == 0x6000:
                char_info = f" ('{chr(bus_state.data)}' se printável)" if 32 <= bus_state.data <= 126 else ""
                print(f"*** LCD PORTB write 0x{bus_state.data:02X}{char_info}")
            elif bus_state.address == 0x6001:
                rs = (bus_state.data & 0x20) != 0
                e = (bus_state.data & 0x80) != 0
                print(f"*** LCD PORTA write 0x{bus_state.data:02X} (RS={rs}, E={e})")

        # Verificar se chegou no loop final
        if bus_state.address == 0x8000 and step > 50:
            print(f"Detectado loop no endereço 0x8000 - programa concluído no step {step}")
            break

    except Exception as e:
        print(f"Erro no step {step}: {e}")
        break

print(f"Executou {step_count} steps total")

# 4. Verificar estado final do LCD
print("\n=== ESTADO FINAL DO LCD ===")
lcd_state = core.get_lcd_state()
print(f"Display ligado: {lcd_state.display_on}")
print(f"Cursor: linha {lcd_state.cursor_row}, coluna {lcd_state.cursor_col}")

# Extrair e mostrar o conteúdo do display
display_bytes = bytes(lcd_state.display)
row1 = display_bytes[:16].decode('ascii', errors='replace').rstrip('\x00 ')
row2 = display_bytes[17:33].decode('ascii', errors='replace').rstrip('\x00 ')

print(f"Linha 1: '{row1}'")
print(f"Linha 2: '{row2}'")

# 5. Verificar se contém "HELLO WORLD"
success = "HELLO WORLD" in row1 or "HELLO WORLD" in row2
print(f"\nSUCESSO: {success}")
if success:
    print("✅ O texto 'HELLO WORLD' foi encontrado no display!")
else:
    print("❌ O texto 'HELLO WORLD' NÃO foi encontrado no display!")
    print("Debug: Conteúdo bruto do display:")
    for i, byte in enumerate(display_bytes[:33]):
        if i == 16:
            print(" | ", end="")
        print(f"{byte:02X}", end=" ")
    print()

core.destroy()
