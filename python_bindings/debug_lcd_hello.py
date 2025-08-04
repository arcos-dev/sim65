#!/usr/bin/env python3
"""
Debug LCD Hello - Script para testar o Hello LCD
================================================
"""
import sys
import os

# Adicionar o diretório pai ao path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from emu65_core import Emu65Core
from programs_6502 import Programs6502

def test_hello_lcd():
    """Testa o Hello LCD diretamente"""
    print("=== Debug Hello LCD ===")

    # Criar instância do emulador
    core = Emu65Core()
    programs = Programs6502()

    # Carregar exemplos gerados
    examples = programs.get_all_programs()

    # Em vez de usar o arquivo do Ben Eater, vamos usar o programa gerado
    print("Usando programa Hello World gerado dinamicamente")
    hello_lcd = programs.hello_world()

    if not hello_lcd:
        print("ERROR: Hello World não foi gerado!")
        return

    print(f"Programa encontrado: {hello_lcd['name']}")
    print(f"Estrutura do programa: {hello_lcd.keys()}")

    # Obter arquivo e endereço
    file_name = hello_lcd.get('filename', hello_lcd.get('file', 'hello_lcd.bin'))
    address = hello_lcd.get('start_address', hello_lcd.get('address', '0x8000'))

    print(f"Arquivo: {file_name}")
    print(f"Endereço: {address}")

    # Verificar se o programa tem binário
    if 'binary' not in hello_lcd:
        print("ERROR: Programa não tem binário!")
        return

    # Obter dados binários
    data = hello_lcd['binary']
    print(f"Programa carregado: {len(data)} bytes")
    print(f"Primeiros 32 bytes: {data[:32].hex(' ')}")
    print(f"Bytes em 0x8000-0x801F: {' '.join(f'{b:02X}' for b in data[:32])}")

    # Carregar programa
    try:
        # Carregar programa usando o método correto
        print(f"Carregando em 0x{address:04X}")
        core.load_program(data, address)

        # Resetar
        core.reset()
        print(f"PC configurado para 0x{address:04X}")

        # Executar alguns ciclos
        print("\nExecutando programa...")

        # Verificar estado inicial da VIA
        print("\n=== Estado inicial ===")
        via_state = core.get_via_state()
        if via_state:
            print(f"VIA PORTB (0x6000): 0x{core.read_byte(0x6000):02X}")
            print(f"VIA PORTA (0x6001): 0x{core.read_byte(0x6001):02X}")
            print(f"VIA DDRB (0x6002): 0x{core.read_byte(0x6002):02X}")
            print(f"VIA DDRA (0x6003): 0x{core.read_byte(0x6003):02X}")

        for cycle in range(1000):  # Primeiros 1000 ciclos com debug detalhado
            core.step()

            # Debug detalhado nos primeiros ciclos
            if cycle < 50:
                cpu_state = core.get_cpu_state()
                if cpu_state:
                    print(f"Ciclo {cycle}: PC=0x{cpu_state.pc:04X}, A=0x{cpu_state.a:02X}")

                    # Verificar escritas no VIA
                    if cycle > 10 and cycle % 5 == 0:
                        portb = core.read_byte(0x6000)
                        porta = core.read_byte(0x6001)
                        ddrb = core.read_byte(0x6002)
                        ddra = core.read_byte(0x6003)
                        print(f"  VIA: PORTB=0x{portb:02X}, PORTA=0x{porta:02X}, DDRB=0x{ddrb:02X}, DDRA=0x{ddra:02X}")

            # Verificar LCD a cada 100 ciclos depois
            if cycle >= 50 and cycle % 100 == 0:
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
                        print(f"  *** TEXTO ENCONTRADO! ***")
                        print(f"  Display ON: {lcd_state.display_on}")
                        print(f"  Cursor: ({lcd_state.cursor_row}, {lcd_state.cursor_col})")
                        break

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
    test_hello_lcd()
