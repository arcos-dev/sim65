#!/usr/bin/env python3
"""
Debug Ben Eater Examples - Script para testar exemplos do Ben Eater
==================================================================
"""
import sys
import os

# Adicionar o diretório pai ao path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from emu65_core import Emu65Core
from programs_6502 import Programs6502

def test_ben_eater_example(example_name):
    """Testa um exemplo do Ben Eater"""
    print(f"=== Debug {example_name} ===")

    # Criar instância do emulador
    core = Emu65Core()
    programs = Programs6502()

    # Carregar exemplos
    examples = programs.get_all_programs()

    # Encontrar o exemplo
    target_example = None
    for program in examples:
        if example_name.lower() in program.get('name', '').lower():
            target_example = program
            break

    if not target_example:
        print(f"ERROR: Exemplo '{example_name}' não encontrado!")
        print("Exemplos disponíveis:")
        for program in examples:
            print(f"  - {program.get('name', 'Sem nome')}")
        return

    print(f"Programa encontrado: {target_example['name']}")
    print(f"Descrição: {target_example['description']}")

    # Obter dados binários
    data = target_example['binary']
    address = target_example['start_address']
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
        print("Testando por 5000 ciclos (pode ser lento para exemplos Ben Eater)...")

        for cycle in range(5000):
            core.step()

            # Verificar LCD a cada 500 ciclos
            if cycle % 500 == 0:
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

                    row1_clean = row1.strip()
                    row2_clean = row2.strip()

                    print(f"Ciclo {cycle}: LCD = '{row1_clean}' | '{row2_clean}'")
                    if row1_clean or row2_clean:
                        print(f"  *** TEXTO ENCONTRADO! ***")
                        print(f"  Display ON: {lcd_state.display_on}")
                        print(f"  Cursor: ({lcd_state.cursor_row}, {lcd_state.cursor_col})")
                        if cycle > 1000:  # Se já rodou bastante, pode parar
                            break

        # Estado final
        final_lcd = core.get_lcd_state()
        if final_lcd:
            display_bytes = bytes(final_lcd.display)
            display_text = display_bytes.decode('ascii', errors='replace')
            print(f"\nEstado final LCD:")
            print(f"  Text: '{display_text[:34].strip()}'")
            print(f"  Display ON: {final_lcd.display_on}")

        print("\n=== Fim do teste ===")

    except Exception as e:
        print(f"ERROR: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    # Testar diferentes exemplos
    examples_to_test = [
        "Binary Counter",
        "Fibonacci",
        "Calculator"
    ]

    for example in examples_to_test:
        test_ben_eater_example(example)
        print("\n" + "="*60 + "\n")
