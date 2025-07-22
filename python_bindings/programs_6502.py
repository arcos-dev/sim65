#!/usr/bin/env python3
"""
Programas 6502 de Exemplo - Versão Refatorada
=============================================

Biblioteca de programas 6502 para demonstração do emulador.
Todos os programas agora incluem inicialização correta do LCD Ben Eater
e componentes necessários.

Autor: Anderson Costa
Versão: 2.0.0
Data: 2025-01-06
"""

class Programs6502:
    """Coleção de programas 6502 de exemplo"""

    @staticmethod
    def _get_lcd_init_sequence():
        """Retorna a sequência de inicialização do LCD Ben Eater"""
        return bytes.fromhex(
            # Configuração dos portos como saída
            "A9 FF 8D 02 60"  # LDA #$FF, STA $6002 (DDRB = output)
            "A9 E0 8D 03 60"  # LDA #$E0, STA $6003 (DDRA = output)

            # Function Set (8-bit, 2 lines, 5x8 font)
            "A9 38 8D 00 60"  # LDA #$38, STA $6000 (PORTB = 0x38)
            "A9 20 8D 01 60"  # LDA #$20, STA $6001 (PORTA = RS=0, RW=0, E=1)
            "A9 00 8D 01 60"  # LDA #$00, STA $6001 (PORTA = RS=0, RW=0, E=0)

            # Display ON/OFF (display on, cursor off, blink off)
            "A9 0C 8D 00 60"  # LDA #$0C, STA $6000 (PORTB = 0x0C)
            "A9 20 8D 01 60"  # LDA #$20, STA $6001 (PORTA = RS=0, RW=0, E=1)
            "A9 00 8D 01 60"  # LDA #$00, STA $6001 (PORTA = RS=0, RW=0, E=0)

            # Clear Display
            "A9 01 8D 00 60"  # LDA #$01, STA $6000 (PORTB = 0x01)
            "A9 20 8D 01 60"  # LDA #$20, STA $6001 (PORTA = RS=0, RW=0, E=1)
            "A9 00 8D 01 60"  # LDA #$00, STA $6001 (PORTA = RS=0, RW=0, E=0)

            # Entry Mode Set (increment cursor, no shift)
            "A9 06 8D 00 60"  # LDA #$06, STA $6000 (PORTB = 0x06)
            "A9 20 8D 01 60"  # LDA #$20, STA $6001 (PORTA = RS=0, RW=0, E=1)
            "A9 00 8D 01 60"  # LDA #$00, STA $6001 (PORTA = RS=0, RW=0, E=0)
        )

    @staticmethod
    def _write_char_to_lcd(char_code):
        """Retorna o código para escrever um caractere no LCD"""
        return bytes.fromhex(
            f"A9 {char_code:02X} 8D 00 60"  # LDA #$char, STA $6000 (PORTB = char)
            "A9 60 8D 01 60"                # LDA #$60, STA $6001 (RS=1, E=1)
            "A9 40 8D 01 60"                # LDA #$40, STA $6001 (RS=1, E=0)
        )

    @staticmethod
    def _add_reset_vector(binary: bytes, start_address: int) -> bytes:
        """Não adiciona mais o vetor de reset - isso é feito no load_program agora"""
        # Simplesmente retorna o binário original
        # O vetor de reset será definido diretamente no load_program
        return binary

    @staticmethod
    def hello_world():
        """Programa Hello World com inicialização correta do LCD"""
        # Sequência de inicialização + "HELLO WORLD!"
        init_seq = Programs6502._get_lcd_init_sequence()

        # Escrever "HELLO WORLD!"
        text_chars = [0x48, 0x45, 0x4C, 0x4C, 0x4F, 0x20, 0x57, 0x4F, 0x52, 0x4C, 0x44, 0x21]
        text_code = b''
        for char in text_chars:
            text_code += Programs6502._write_char_to_lcd(char)

        # Loop infinito
        loop_code = bytes.fromhex("4C 00 80")  # JMP $8000

        full_program = init_seq + text_code + loop_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'Hello World',
            'description': 'Escreve "HELLO WORLD!" no LCD com inicialização correta',
            'components': ['6502 CPU', 'LCD 16x2', 'RAM', 'ROM'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def counter():
        """Programa contador no LCD com inicialização correta"""
        init_seq = Programs6502._get_lcd_init_sequence()

        # Código do contador
        counter_code = bytes.fromhex(
            # Inicializar contador
            "A9 00 85 00"                    # LDA #$00, STA $00 (contador = 0)

            # Loop principal
            "A5 00 C9 64 F0 0A"             # LDA $00, CMP #$64, BEQ reset
            "E6 00"                         # INC $00
            "A5 00 69 30"                   # LDA $00, ADC #$30 (converter para ASCII)
            "8D 00 60"                      # STA $6000 (PORTB)
            "A9 60 8D 01 60"                # LDA #$60, STA $6001 (RS=1, E=1)
            "A9 40 8D 01 60"                # LDA #$40, STA $6001 (RS=1, E=0)
            "4C 02 80"                      # JMP loop

            # Reset contador
            "A9 30 8D 00 60"                # LDA #$30, STA $6000 (escrever '0')
            "A9 60 8D 01 60"                # LDA #$60, STA $6001 (RS=1, E=1)
            "A9 40 8D 01 60"                # LDA #$40, STA $6001 (RS=1, E=0)
            "A9 00 85 00"                   # LDA #$00, STA $00 (reset contador)
            "4C 02 80"                      # JMP loop
        )

        full_program = init_seq + counter_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'Contador',
            'description': 'Conta de 0 a 99 no LCD com inicialização correta',
            'components': ['6502 CPU', 'LCD 16x2', 'RAM'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def math_demo():
        """Demonstração de matemática com LCD"""
        init_seq = Programs6502._get_lcd_init_sequence()

        math_code = bytes.fromhex(
            # ADC demo
            "A9 05 69 03 85 00"             # LDA #$05, ADC #$03, STA $00 (5+3=8)
            "A5 00 69 30 8D 00 60"          # LDA $00, ADC #$30, STA $6000 (converter para ASCII)
            "A9 60 8D 01 60"                # LDA #$60, STA $6001 (RS=1, E=1)
            "A9 40 8D 01 60"                # LDA #$40, STA $6001 (RS=1, E=0)

            # SBC demo
            "A9 10 E5 00 85 01"             # LDA #$10, SBC $00, STA $01 (16-8=8)
            "A5 01 69 30 8D 00 60"          # LDA $01, ADC #$30, STA $6000
            "A9 60 8D 01 60"                # LDA #$60, STA $6001 (RS=1, E=1)
            "A9 40 8D 01 60"                # LDA #$40, STA $6001 (RS=1, E=0)

            "4C 00 80"                      # JMP $8000
        )

        full_program = init_seq + math_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'Matemática',
            'description': 'Demonstra ADC e SBC com exibição no LCD',
            'components': ['6502 CPU', 'LCD 16x2', 'RAM'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def interrupt_demo():
        """Demonstração de interrupções com LCD"""
        init_seq = Programs6502._get_lcd_init_sequence()

        interrupt_code = bytes.fromhex(
            # Desabilitar interrupções
            "78"                            # SEI

            # Escrever "IRQ"
            "A9 49 8D 00 60"                # LDA #$49, STA $6000 ('I')
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A9 52 8D 00 60"                # LDA #$52, STA $6000 ('R')
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A9 51 8D 00 60"                # LDA #$51, STA $6000 ('Q')
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0

            # Habilitar interrupções
            "58"                            # CLI

            # Escrever "NMI"
            "A9 4E 8D 00 60"                # LDA #$4E, STA $6000 ('N')
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A9 4D 8D 00 60"                # LDA #$4D, STA $6000 ('M')
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A9 49 8D 00 60"                # LDA #$49, STA $6000 ('I')
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0

            "4C 00 80"                      # JMP $8000
        )

        full_program = init_seq + interrupt_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'Interrupções',
            'description': 'Demonstra IRQ e NMI com exibição no LCD',
            'components': ['6502 CPU', 'LCD 16x2', 'RAM'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def scroll_text():
        """Texto rolante no LCD com inicialização correta"""
        init_seq = Programs6502._get_lcd_init_sequence()

        # Código para texto rolante "HELLO WORLD!"
        scroll_code = bytes.fromhex(
            # Carregar texto na memória
            "A9 48 85 00"                   # LDA #$48, STA $00 ('H')
            "A9 45 85 01"                   # LDA #$45, STA $01 ('E')
            "A9 4C 85 02"                   # LDA #$4C, STA $02 ('L')
            "A9 4C 85 03"                   # LDA #$4C, STA $03 ('L')
            "A9 4F 85 04"                   # LDA #$4F, STA $04 ('O')
            "A9 20 85 05"                   # LDA #$20, STA $05 (' ')
            "A9 57 85 06"                   # LDA #$57, STA $06 ('W')
            "A9 4F 85 07"                   # LDA #$4F, STA $07 ('O')
            "A9 52 85 08"                   # LDA #$52, STA $08 ('R')
            "A9 4C 85 09"                   # LDA #$4C, STA $09 ('L')
            "A9 44 85 0A"                   # LDA #$44, STA $0A ('D')
            "A9 21 85 0B"                   # LDA #$21, STA $0B ('!')

            # Escrever texto no LCD
            "A5 00 8D 00 60"                # LDA $00, STA $6000
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A5 01 8D 00 60"                # LDA $01, STA $6000
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A5 02 8D 00 60"                # LDA $02, STA $6000
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A5 03 8D 00 60"                # LDA $03, STA $6000
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A5 04 8D 00 60"                # LDA $04, STA $6000
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A5 05 8D 00 60"                # LDA $05, STA $6000
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A5 06 8D 00 60"                # LDA $06, STA $6000
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A5 07 8D 00 60"                # LDA $07, STA $6000
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A5 08 8D 00 60"                # LDA $08, STA $6000
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A5 09 8D 00 60"                # LDA $09, STA $6000
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A5 0A 8D 00 60"                # LDA $0A, STA $6000
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A5 0B 8D 00 60"                # LDA $0B, STA $6000
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0

            "4C 00 80"                      # JMP $8000
        )

        full_program = init_seq + scroll_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'Texto Rolante',
            'description': 'Escreve "HELLO WORLD!" no LCD com inicialização correta',
            'components': ['6502 CPU', 'LCD 16x2', 'RAM'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def led_blink():
        """LED piscante com RAM"""
        led_code = bytes.fromhex(
            # Configurar porta como saída
            "A9 FF 8D 02 60"                # LDA #$FF, STA $6002 (DDRB = output)

            # Loop de piscagem
            "A9 FF 8D 00 60"                # LDA #$FF, STA $6000 (LED ON)
            "A9 00 8D 00 60"                # LDA #$00, STA $6000 (LED OFF)
            "4C 00 80"                      # JMP $8000
        )

        full_program = led_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'LED Piscante',
            'description': 'Pisca LED conectado ao endereço 0x6000',
            'components': ['6502 CPU', 'LED', 'RAM'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def keyboard_demo():
        """Demonstração de teclado com LCD"""
        init_seq = Programs6502._get_lcd_init_sequence()

        keyboard_code = bytes.fromhex(
            # Ler teclado e exibir no LCD
            "AD 00 70 8D 00 60"             # LDA $7000, STA $6000 (ler teclado)
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "4C 00 80"                      # JMP $8000
        )

        full_program = init_seq + keyboard_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'Teclado',
            'description': 'Lê teclado e exibe no LCD com inicialização correta',
            'components': ['6502 CPU', 'LCD 16x2', 'Botão', 'RAM'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def memory_test():
        """Teste de memória com LCD"""
        init_seq = Programs6502._get_lcd_init_sequence()

        memory_code = bytes.fromhex(
            # Teste de escrita e leitura
            "A9 55 85 00"                   # LDA #$55, STA $00
            "A9 AA 85 01"                   # LDA #$AA, STA $01
            "A5 00 8D 00 60"                # LDA $00, STA $6000 (exibir primeiro valor)
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A5 01 8D 00 60"                # LDA $01, STA $6000 (exibir segundo valor)
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "4C 00 80"                      # JMP $8000
        )

        full_program = init_seq + memory_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'Teste de Memória',
            'description': 'Testa escrita e leitura de memória com exibição no LCD',
            'components': ['6502 CPU', 'RAM', 'LCD 16x2'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def stack_demo():
        """Demonstração de stack com LCD"""
        init_seq = Programs6502._get_lcd_init_sequence()

        stack_code = bytes.fromhex(
            # Demonstração de PHA e PLA
            "A9 42 48"                      # LDA #$42, PHA (empilhar 'B')
            "A9 00 68"                      # LDA #$00, PLA (desempilhar 'B')
            "8D 00 60"                      # STA $6000 (exibir caractere)
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "4C 00 80"                      # JMP $8000
        )

        full_program = init_seq + stack_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'Stack',
            'description': 'Demonstra operações de stack (PHA, PLA) com LCD',
            'components': ['6502 CPU', 'LCD 16x2', 'RAM'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def flags_demo():
        """Demonstração de flags com LCD"""
        init_seq = Programs6502._get_lcd_init_sequence()

        flags_code = bytes.fromhex(
            # Teste Carry Flag
            "A9 00 69 01 8D 00 60"          # LDA #$00, ADC #$01, STA $6000 (sem carry)
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0

            # Teste Carry Flag com overflow
            "A9 FF 69 01 8D 00 60"          # LDA #$FF, ADC #$01, STA $6000 (com carry)
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0

            # Teste Zero Flag
            "A9 00 69 00 8D 00 60"          # LDA #$00, ADC #$00, STA $6000 (zero)
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0

            "4C 00 80"                      # JMP $8000
        )

        full_program = init_seq + flags_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'Flags',
            'description': 'Demonstra flags de status (C, Z, N) com LCD',
            'components': ['6502 CPU', 'LCD 16x2', 'RAM'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def complex_demo():
        """Demonstração complexa com LCD"""
        init_seq = Programs6502._get_lcd_init_sequence()

        complex_code = bytes.fromhex(
            # Programa complexo com loops e condicionais
            "A9 00 85 00"                   # LDA #$00, STA $00 (contador1 = 0)
            "A9 01 85 01"                   # LDA #$01, STA $01 (contador2 = 1)

            # Loop principal
            "A5 00 69 01 85 00"             # LDA $00, ADC #$01, STA $00 (incrementar contador1)
            "C9 0A F0 08"                   # CMP #$0A, BEQ reset1
            "A5 00 69 30 8D 00 60"          # LDA $00, ADC #$30, STA $6000 (exibir contador1)
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "4C 02 80"                      # JMP loop

            # Reset contador1
            "A9 30 8D 00 60"                # LDA #$30, STA $6000 (exibir '0')
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "A9 00 85 00"                   # LDA #$00, STA $00 (reset contador1)

            # Incrementar contador2
            "A5 01 69 01 85 01"             # LDA $01, ADC #$01, STA $01 (incrementar contador2)
            "C9 0A F0 08"                   # CMP #$0A, BEQ reset2
            "A5 01 69 30 8D 00 60"          # LDA $01, ADC #$30, STA $6000 (exibir contador2)
            "A9 60 8D 01 60 A9 40 8D 01 60" # RS=1, E=1, RS=1, E=0
            "4C 02 80"                      # JMP loop

            # Reset contador2
            "A9 01 85 01"                   # LDA #$01, STA $01 (reset contador2)
            "4C 02 80"                      # JMP loop
        )

        full_program = init_seq + complex_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'Demo Complexa',
            'description': 'Programa complexo com loops e condicionais no LCD',
            'components': ['6502 CPU', 'LCD 16x2', 'RAM'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def sound_demo():
        """Demonstração de som com componentes"""
        sound_code = bytes.fromhex(
            # Gera diferentes frequências de som
            "A9 01 8D 00 50"                # LDA #$01, STA $5000 (som ON)
            "A9 00 8D 00 50"                # LDA #$00, STA $5000 (som OFF)
            "4C 00 80"                      # JMP $8000
        )

        full_program = sound_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'Som',
            'description': 'Gera diferentes frequências de som',
            'components': ['6502 CPU', 'Resistor', 'Capacitor', 'RAM'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def graphics_demo():
        """Demonstração de gráficos com ROM"""
        graphics_code = bytes.fromhex(
            # Desenha padrões no display gráfico
            "A9 FF 8D 00 40"                # LDA #$FF, STA $4000 (padrão 1)
            "A9 00 8D 01 40"                # LDA #$00, STA $4001 (padrão 2)
            "A9 FF 8D 02 40"                # LDA #$FF, STA $4002 (padrão 3)
            "A9 00 8D 03 40"                # LDA #$00, STA $4003 (padrão 4)
            "4C 00 80"                      # JMP $8000
        )

        full_program = graphics_code
        start_address = 0x8000
        binary = Programs6502._add_reset_vector(full_program, start_address)

        return {
            'name': 'Gráficos',
            'description': 'Desenha padrões no display gráfico',
            'components': ['6502 CPU', 'ROM', 'RAM'],
            'binary': binary,
            'start_address': start_address
        }

    @staticmethod
    def get_all_programs():
        """Retornar todos os programas disponíveis dos exemplos Ben Eater"""
        import os

        # Caminho base para os exemplos
        base_path = os.path.dirname(os.path.dirname(__file__))
        examples_path = os.path.join(base_path, 'examples', 'ben_eater')

        programs = []

        # Definições dos exemplos Ben Eater
        ben_eater_examples = [
            {
                'file': 'hello_lcd.bin',
                'name': 'Hello LCD',
                'description': 'Display "Hello, World!" no LCD',
                'address': 0x8000,
                'components': ['6502 CPU', 'LCD 16x2', 'RAM', 'ROM']
            },
            {
                'file': 'binary_counter.bin',
                'name': 'Binary Counter',
                'description': 'Contador binário no LCD',
                'address': 0x8000,
                'components': ['6502 CPU', 'LCD 16x2', 'RAM', 'ROM']
            },
            {
                'file': 'echo.bin',
                'name': 'Echo',
                'description': 'Echo de caracteres via ACIA',
                'address': 0x8000,
                'components': ['6502 CPU', 'RAM', 'ROM']
            },
            {
                'file': 'fibonacci.bin',
                'name': 'Fibonacci',
                'description': 'Sequência de Fibonacci',
                'address': 0x8000,
                'components': ['6502 CPU', 'LCD 16x2', 'RAM', 'ROM']
            },
            {
                'file': 'calculator.bin',
                'name': 'Calculator',
                'description': 'Calculadora simples',
                'address': 0x8000,
                'components': ['6502 CPU', 'LCD 16x2', 'Botão', 'RAM', 'ROM']
            },
            {
                'file': 'clock.bin',
                'name': 'Clock',
                'description': 'Relógio digital',
                'address': 0x8000,
                'components': ['6502 CPU', 'LCD 16x2', 'RAM', 'ROM']
            },
            {
                'file': 'memory_test.bin',
                'name': 'Memory Test',
                'description': 'Teste de memória',
                'address': 0x8000,
                'components': ['6502 CPU', 'LCD 16x2', 'LED', 'RAM', 'ROM']
            },
            {
                'file': 'interrupt_timer.bin',
                'name': 'Interrupt Timer',
                'description': 'Timer com interrupções',
                'address': 0x8000,
                'components': ['6502 CPU', 'LCD 16x2', 'LED', 'RAM', 'ROM']
            },
            {
                'file': 'basic_interpreter.bin',
                'name': 'BASIC Interpreter',
                'description': 'Interpretador BASIC simples',
                'address': 0x8000,
                'components': ['6502 CPU', 'LCD 16x2', 'RAM', 'ROM']
            }
        ]        # Carregar programas que existem
        for example in ben_eater_examples:
            file_path = os.path.join(examples_path, example['file'])
            if os.path.exists(file_path):
                try:
                    with open(file_path, 'rb') as f:
                        binary_data = f.read()

                    programs.append({
                        'name': example['name'],
                        'description': example['description'],
                        'binary': binary_data,
                        'start_address': example['address'],
                        'pc_start': example['address'],  # Para compatibilidade
                        'components': example['components']  # Adicionar componentes
                    })
                except Exception as e:
                    print(f"Erro ao carregar {example['file']}: {e}")
                    continue

        # Se não encontrou nenhum exemplo, usar programas gerados
        if not programs:
            print("Nenhum exemplo Ben Eater encontrado, usando programas gerados")
            programs = [
                Programs6502.hello_world(),
                Programs6502.counter(),
                Programs6502.math_demo()
            ]

        return programs