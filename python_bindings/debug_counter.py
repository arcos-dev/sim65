#!/usr/bin/env python3
"""
Debug Counter - Script para testar o programa Counter
===================================================
"""
import sys
import os

# Adicionar o diretório pai ao path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from emu65_core import Emu65Core
from programs_6502 import Programs6502

def test_counter():
    """Testa o programa Counter"""
    print("=== Debug Counter ===")

    # Criar instância do emulador
    core = Emu65Core()
    programs = Programs6502()

    # Usar programa gerado
    print("Usando programa Counter gerado dinamicamente")
    counter_program = programs.counter()

    if not counter_program:
        print("ERROR: Counter não foi gerado!")
        return

    print(f"Programa encontrado: {counter_program['name']}")
    print(f"Estrutura do programa: {counter_program.keys()}")

    # Obter dados binários
    data = counter_program['binary']
    address = counter_program['start_address']
    print(f"Programa carregado: {len(data)} bytes")
    print(f"Endereço: 0x{address:04X}")

    # Carregar programa
    try:
        print(f"Carregando em 0x{address:04X}")
        core.load_program(data, address)

        # Resetar
        core.reset()
        print(f"PC configurado para 0x{address:04X}")

        # Executar alguns ciclos
        print("\nExecutando programa...")

        for cycle in range(2000):  # Mais ciclos para ver o contador
            core.step()

            # Verificar LCD a cada 200 ciclos
            if cycle % 200 == 0:
                lcd_state = core.get_lcd_state()
                if lcd_state:
                    display_bytes = bytes(lcd_state.display)
                    display_text = display_bytes.decode('ascii', errors='replace')

                    # Dividir em linhas
                    if len(display_text) >= 34:
                        null_pos1 = display_text.find('\x00')
                        if null_pos1 >= 0:
                            row1 = display_text[:null_pos1]
                            remaining = display_text[null_pos1 + 1:]
                            null_pos2 = remaining.find('\x00')
                            if null_pos2 >= 0:
                                row2 = remaining[:null_pos2]
                            else:
                                row2 = remaining
                        else:
                            row1 = display_text[:16]
                            row2 = display_text[17:33] if len(display_text) >= 33 else ""
                    else:
                        row1 = display_text[:16] if len(display_text) >= 16 else ""
                        row2 = display_text[16:32] if len(display_text) >= 32 else ""

                    print(f"Ciclo {cycle}: LCD = '{row1.strip()}' | '{row2.strip()}'")
                    if row1.strip() or row2.strip():
                        print(f"  Display ON: {lcd_state.display_on}")
                        print(f"  Cursor: ({lcd_state.cursor_row}, {lcd_state.cursor_col})")

        # Estado final
        final_lcd = core.get_lcd_state()
        if final_lcd:
            display_bytes = bytes(final_lcd.display)
            display_text = display_bytes.decode('ascii', errors='replace')
            print(f"\nEstado final LCD:")
            print(f"  Raw bytes: {display_bytes[:34]}")
            print(f"  Text: '{display_text[:34]}'")
            print(f"  Display ON: {final_lcd.display_on}")

        print("\n=== Fim do teste ===")

    except Exception as e:
        print(f"ERROR: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    test_counter()
