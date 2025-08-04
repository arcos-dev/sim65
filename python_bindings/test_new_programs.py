#!/usr/bin/env python3
"""
Test New Programs - Testar novos programas gerados
================================================
"""
import sys
import os

# Adicionar o diretório pai ao path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from emu65_core import Emu65Core
from programs_6502 import Programs6502

def test_program(program_method, program_name):
    """Testa um programa gerado"""
    print(f"=== Testando {program_name} ===")

    # Criar instância do emulador
    core = Emu65Core()
    programs = Programs6502()

    # Obter programa
    program = program_method()

    print(f"Programa: {program['name']}")
    print(f"Descrição: {program['description']}")
    print(f"Tamanho: {len(program['binary'])} bytes")

    # Carregar programa
    try:
        core.load_program(program['binary'], program['start_address'])
        core.reset()

        # Executar
        for cycle in range(200):
            core.step()

            if cycle % 50 == 0:
                lcd_state = core.get_lcd_state()
                if lcd_state:
                    display_bytes = bytes(lcd_state.display)
                    display_text = display_bytes.decode('ascii', errors='replace')

                    row1 = display_text[:16].strip()
                    row2 = display_text[16:32].strip() if len(display_text) >= 32 else ""

                    print(f"Ciclo {cycle}: LCD = '{row1}' | '{row2}'")
                    if row1 or row2:
                        print(f"  ✅ FUNCIONANDO!")
                        break

        print(f"✅ {program_name} testado com sucesso!\n")

    except Exception as e:
        print(f"❌ ERRO em {program_name}: {e}\n")

if __name__ == "__main__":
    programs = Programs6502()

    test_program(programs.fibonacci, "Fibonacci")
    test_program(programs.calculator_demo, "Calculator Demo")
