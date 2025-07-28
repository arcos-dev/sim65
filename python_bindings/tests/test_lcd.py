#!/usr/bin/env python3
"""
Teste do LCD - Debug Compl        print("   Programa carregado e reset executado")

        # 4. Verificar estado inicial do LCD
        print("4. Estado inicial do LCD...")
        lcd_state = core.get_lcd_state()
        print(f"   display_on: {lcd_state.display_on}")
        print(f"   cursor: row={lcd_state.cursor_row}, col={lcd_state.cursor_col}")
        print(f"   function_set: 0x{lcd_state.function_set:02X}")
        print(f"   display_control: 0x{lcd_state.display_control:02X}")
        print(f"   entry_mode: 0x{lcd_state.entry_mode:02X}")

        # 5. Executar steps e monitorar o LCD
        print("5. Executando steps e monitorando LCD...")

        max_steps = 100  # Limite para evitar loop infinito===================

Script para testar e debugar o funcionamento do LCD com o exemplo Hello World.

Autor: Anderson Costa
Versão: 1.0.0
Data: 2025-01-06
"""

import sys
import os
import pytest

# Adiciona o diretório raiz do projeto ao sys.path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from python_bindings.emu65_core import Emu65Core
from python_bindings.programs_6502 import Programs6502

def test_lcd_hello_world_step_by_step():
    """Testa o exemplo Hello World passo a passo para debugar o LCD"""
    print("\n=== TESTE COMPLETO DO LCD HELLO WORLD ===")

    # 1. Criar o core do emulador
    print("1. Criando core do emulador...")
    with Emu65Core({
        'clock_frequency': 100,
        'debug_mode': True,
        'trace_execution': True
    }) as core:
        # 2. Obter o programa Hello World
        print("2. Obtendo programa Hello World...")
        programs = Programs6502.get_all_programs()
        hello_world = None
        for prog in programs:
            if prog['name'] == 'Hello World':
                hello_world = prog
                break

        assert hello_world is not None, "Programa Hello World não encontrado!"
        print(f"   Programa: {hello_world['name']}")
        print(f"   Descrição: {hello_world['description']}")
        print(f"   Tamanho: {len(hello_world['binary'])} bytes")
        print(f"   Endereço: 0x{hello_world['start_address']:04X}")
        print(f"   Componentes: {hello_world['components']}")

        # 3. Carregar o programa
        print("3. Carregando programa...")
        core.reset()
        core.load_program(hello_world['binary'], hello_world['start_address'])
        core.reset()
        print("   Programa carregado e reset executado")

        # 4. Verificar estado inicial do LCD
        print("4. Estado inicial do LCD...")
        lcd_state = core.get_lcd_state()
        print(f"   display_on: {lcd_state.display_on}")
        print(f"   cursor: row={lcd_state.cursor_row}, col={lcd_state.cursor_col}")
        print(f"   function_set: 0x{lcd_state.function_set:02X}")
        print(f"   display_control: 0x{lcd_state.display_control:02X}")
        print(f"   entry_mode: 0x{lcd_state.entry_mode:02X}")

        # 5. Executar steps e monitorar o LCD
        print("5. Executando steps e monitorando LCD...")

        max_steps = 100  # Limite para evitar loop infinito
        step_count = 0

        while step_count < max_steps:
            step_count += 1
            print(f"\n--- STEP {step_count} ---")

            # Executar um step
            try:
                result = core.step()

                # Obter estado do barramento
                bus_state = core.get_bus_state()
                print(f"Bus: addr=0x{bus_state.address:04X}, data=0x{bus_state.data:02X}, rw={'R' if bus_state.rw else 'W'}")

                # Se for acesso ao VIA (LCD), mostrar detalhes
                if 0x6000 <= bus_state.address <= 0x6003:
                    print(f"*** ACESSO VIA/LCD ***")
                    print(f"    Endereço: 0x{bus_state.address:04X}")
                    print(f"    Dado: 0x{bus_state.data:02X} ('{chr(bus_state.data) if 32 <= bus_state.data <= 126 else '?'}')")
                    print(f"    Operação: {'READ' if bus_state.rw else 'WRITE'}")

                    if bus_state.address == 0x6000:
                        print(f"    PORTB (dados LCD): 0x{bus_state.data:02X}")
                    elif bus_state.address == 0x6001:
                        print(f"    PORTA (controle LCD): 0x{bus_state.data:02X}")
                        rs = (bus_state.data & 0x20) != 0
                        rw = (bus_state.data & 0x40) != 0
                        e = (bus_state.data & 0x80) != 0
                        print(f"    Sinais: RS={rs}, RW={rw}, E={e}")

                    # Verificar mudanças no estado do LCD
                    new_lcd_state = core.get_lcd_state()
                    if (new_lcd_state.display_on != lcd_state.display_on or
                        new_lcd_state.cursor_row != lcd_state.cursor_row or
                        new_lcd_state.cursor_col != lcd_state.cursor_col):
                        print(f"    MUDANÇA NO LCD:")
                        print(f"      display_on: {lcd_state.display_on} -> {new_lcd_state.display_on}")
                        print(f"      cursor: ({lcd_state.cursor_row},{lcd_state.cursor_col}) -> ({new_lcd_state.cursor_row},{new_lcd_state.cursor_col})")
                        lcd_state = new_lcd_state

                    # Mostrar conteúdo do display
                    display_bytes = bytes(new_lcd_state.display)
                    row1 = display_bytes[:16].decode('ascii', errors='replace').rstrip('\x00')
                    row2 = display_bytes[17:33].decode('ascii', errors='replace').rstrip('\x00')
                    if row1 or row2:
                        print(f"    DISPLAY ATUAL:")
                        print(f"      Linha 1: '{row1}'")
                        print(f"      Linha 2: '{row2}'")

                # Parar se chegamos no final (JMP $8000)
                if bus_state.address == 0x8000 and step_count > 10:
                    print(f"\nDetectado loop no endereço 0x8000 - programa inicializado")
                    break

            except Exception as e:
                print(f"Erro no step {step_count}: {e}")
                break

        # 6. Estado final do LCD
        print(f"\n6. Estado final do LCD após {step_count} steps...")
        final_lcd_state = core.get_lcd_state()
        print(f"   display_on: {final_lcd_state.display_on}")
        print(f"   cursor: row={final_lcd_state.cursor_row}, col={final_lcd_state.cursor_col}")
        print(f"   function_set: 0x{final_lcd_state.function_set:02X}")
        print(f"   display_control: 0x{final_lcd_state.display_control:02X}")
        print(f"   entry_mode: 0x{final_lcd_state.entry_mode:02X}")

        # Extrair texto do display
        display_bytes = bytes(final_lcd_state.display)
        row1 = display_bytes[:16].decode('ascii', errors='replace').rstrip('\x00 ')
        row2 = display_bytes[17:33].decode('ascii', errors='replace').rstrip('\x00 ')

        print(f"   DISPLAY FINAL:")
        print(f"     Linha 1: '{row1}'")
        print(f"     Linha 2: '{row2}'")

        # 7. Verificações
        print("7. Verificações...")
        assert final_lcd_state.display_on, "LCD deveria estar ligado"
        assert "HELLO WORLD" in row1, f"Esperado 'HELLO WORLD' na linha 1, mas encontrado: '{row1}'"

        print("*** TESTE CONCLUÍDO COM SUCESSO! ***")

def test_lcd_basic_functionality():
    """Teste básico de funcionalidade do LCD"""
    print("\n=== TESTE BÁSICO DO LCD ===")

    with Emu65Core({'clock_frequency': 100}) as core:
        # Estado inicial
        lcd_state = core.get_lcd_state()
        print(f"Estado inicial - display_on: {lcd_state.display_on}")

        # Simular comandos básicos manualmente
        print("Simulando comandos LCD manuais...")

        # Function Set
        core.write_byte(0x6000, 0x38)  # Dados
        core.write_byte(0x6001, 0x80)  # E=1
        core.write_byte(0x6001, 0x00)  # E=0

        # Display On
        core.write_byte(0x6000, 0x0C)  # Dados
        core.write_byte(0x6001, 0x80)  # E=1
        core.write_byte(0x6001, 0x00)  # E=0

        # Verificar estado
        lcd_state = core.get_lcd_state()
        print(f"Após comandos - display_on: {lcd_state.display_on}")

        # Escrever caractere 'H'
        core.write_byte(0x6000, 0x48)  # 'H'
        core.write_byte(0x6001, 0xA0)  # RS=1, E=1
        core.write_byte(0x6001, 0x20)  # RS=1, E=0

        # Verificar display
        lcd_state = core.get_lcd_state()
        display_bytes = bytes(lcd_state.display)
        row1 = display_bytes[:16].decode('ascii', errors='replace').rstrip('\x00')
        print(f"Após escrever 'H': '{row1}'")

        assert 'H' in row1, f"Esperado 'H' no display, mas encontrado: '{row1}'"

    print("*** TESTE BÁSICO CONCLUÍDO! ***")

if __name__ == "__main__":
    # Executar testes diretamente
    test_lcd_basic_functionality()
    test_lcd_hello_world_step_by_step()
