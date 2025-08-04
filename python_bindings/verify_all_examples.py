#!/usr/bin/env python3
"""
Verificar Todos os Exemplos - Teste final de todos os programas gerados
======================================================================
"""
import sys
import os

# Adicionar o diretório pai ao path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from emu65_core import Emu65Core
from programs_6502 import Programs6502

def test_all_fast_examples():
    """Testa todos os exemplos rápidos"""
    print("=== Verificação Final - Todos os Exemplos Rápidos ===\n")

    # Criar instância do emulador
    core = Emu65Core()
    programs = Programs6502()

    # Lista de programas rápidos para testar
    fast_programs = [
        ("Hello World", programs.hello_world),
        ("Contador", programs.counter),
        ("Fibonacci", programs.fibonacci),
        ("Calculator Demo", programs.calculator_demo),
        ("Text Demo", programs.text_demo)
    ]

    results = []

    for name, program_method in fast_programs:
        print(f"Testando {name}...")

        try:
            # Obter programa
            program = program_method()

            # Carregar e testar
            core.load_program(program['binary'], program['start_address'])
            core.reset()

            # Executar por alguns ciclos
            text_found = False
            for cycle in range(150):
                core.step()

                if cycle >= 50 and cycle % 25 == 0:
                    lcd_state = core.get_lcd_state()
                    if lcd_state:
                        display_bytes = bytes(lcd_state.display)
                        display_text = display_bytes.decode('ascii', errors='replace')

                        row1 = display_text[:16].strip()
                        row2 = display_text[16:32].strip() if len(display_text) >= 32 else ""

                        if row1 or row2:
                            print(f"  ✅ LCD: '{row1}' | '{row2}'")
                            text_found = True
                            break

            if text_found:
                results.append((name, "✅ FUNCIONANDO"))
            else:
                results.append((name, "⚠️  SEM TEXTO"))

        except Exception as e:
            results.append((name, f"❌ ERRO: {e}"))

        print()

    # Resumo final
    print("="*60)
    print("RESUMO FINAL:")
    print("="*60)

    working_count = 0
    for name, status in results:
        print(f"{name:20} : {status}")
        if "✅" in status:
            working_count += 1

    print("="*60)
    print(f"RESULTADO: {working_count}/{len(results)} exemplos funcionando")

    if working_count == len(results):
        print("🎉 TODOS OS EXEMPLOS RÁPIDOS FUNCIONANDO!")
    else:
        print("⚠️  Alguns exemplos precisam de ajustes")

    print("="*60)

if __name__ == "__main__":
    test_all_fast_examples()
