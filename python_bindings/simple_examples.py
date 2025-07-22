#!/usr/bin/env python3
"""
Exemplos Simples - EMU65
========================

Exemplos básicos que funcionam diretamente com o core do emulador 6502 em C.
Agora centralizados em arquivos .bin gerados pelo VASM.

Autor: Anderson Costa
Versão: 2.0.0
Data: 2025-07-16
"""
import os

EXAMPLES_DIR = os.path.join(os.path.dirname(__file__), '..', 'examples')

class SimpleExamples:
    """Exemplos simples para o emulador 6502, agora usando arquivos binários externos"""
    @staticmethod
    def hello_world():
        return {
            'name': 'Hello World',
            'description': 'Escreve "HI" no LCD usando protocolo Ben Eater',
            'bin_path': os.path.join(EXAMPLES_DIR, 'hello_world.bin'),
            'start_address': 0x8000
        }

    @staticmethod
    def counter():
        return {
            'name': 'Contador',
            'description': 'Conta de 0 a 15 e reinicia',
            'bin_path': os.path.join(EXAMPLES_DIR, 'counter.bin'),
            'start_address': 0x8000
        }

    @staticmethod
    def led_blink():
        return {
            'name': 'LED Piscante',
            'description': 'Pisca LED no endereço 0x6000',
            'bin_path': os.path.join(EXAMPLES_DIR, 'led_blink.bin'),
            'start_address': 0x8000
        }

    @staticmethod
    def hello_world_complete():
        return {
            'name': 'Hello World Completo',
            'description': 'Hello World com protocolo LCD robusto e busy flag',
            'bin_path': os.path.join(EXAMPLES_DIR, 'hello_world_complete.bin'),
            'start_address': 0x0200
        }

    @staticmethod
    def get_all_examples():
        return [
            SimpleExamples.hello_world(),
            SimpleExamples.counter(),
            SimpleExamples.led_blink(),
            SimpleExamples.hello_world_complete()
        ] 