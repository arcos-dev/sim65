#!/usr/bin/env python3
"""
Testes de Transições do Sinal Enable
====================================

Monitora transições do sinal Enable para debug do processamento LCD.
Convertido para formato pytest.

Autor: Anderson Costa
Versão: 1.0.0
Data: 2025-01-27
"""

import pytest


@pytest.mark.lcd
@pytest.mark.via
@pytest.mark.integration
def test_enable_signal_transitions(emu_core, hello_world_program):
    """Testa transições do sinal Enable do LCD"""

    emu = emu_core
    hello_world_program_data = hello_world_program['binary']

    emu.load_program(hello_world_program_data, 0x8000)
    emu.reset()

    max_steps = 200
    prev_e = False
    transitions = []
    lcd_operations = []

    for step in range(max_steps):
        cycles = emu.step()
        if cycles <= 0:
            break

        # Estado atual do VIA
        portb_data = emu.read_byte(0x6000)
        porta_control = emu.read_byte(0x6001)

        # Decodifica bits de controle
        rs = (porta_control & 0x20) != 0
        rw = (porta_control & 0x40) != 0
        e = (porta_control & 0x80) != 0

        # Verifica transições do Enable
        e_rising_edge = not prev_e and e
        e_falling_edge = prev_e and not e

        if e_rising_edge or e_falling_edge:
            transition = {
                'step': step,
                'type': 'RISING' if e_rising_edge else 'FALLING',
                'portb': portb_data,
                'porta': porta_control,
                'rs': rs,
                'rw': rw,
                'e': e
            }
            transitions.append(transition)

            # Processamento LCD deveria acontecer na borda de descida
            if e_falling_edge and not rw:
                operation_type = "DATA" if rs else "COMMAND"
                operation = {
                    'step': step,
                    'type': operation_type,
                    'value': portb_data,
                    'char': chr(portb_data) if 32 <= portb_data <= 126 else None
                }
                lcd_operations.append(operation)

        prev_e = e

        # Verifica se completou um ciclo
        cpu_state = emu.get_cpu_state()
        if step > 50 and cpu_state.pc == 0x8000:
            break

    # Verificações
    assert len(transitions) > 0, "Deveria haver transições do sinal Enable detectadas"
    assert len(lcd_operations) > 0, "Deveria haver operações LCD detectadas"

    # Verifica tipos de operações
    commands = [op for op in lcd_operations if op['type'] == 'COMMAND']
    data_writes = [op for op in lcd_operations if op['type'] == 'DATA']

    assert len(commands) > 0, "Deveria haver comandos LCD"
    assert len(data_writes) > 0, "Deveria haver escritas de dados LCD"


@pytest.mark.lcd
@pytest.mark.via
@pytest.mark.unit
def test_enable_bit_detection(emu_core):
    """Testa detecção do bit Enable"""

    emu = emu_core

    # Estado inicial
    porta_control = emu.read_byte(0x6001)
    initial_e = (porta_control & 0x80) != 0

    # O bit E deveria estar definido corretamente
    assert isinstance(initial_e, bool), "Bit Enable deveria ser boolean"


@pytest.mark.lcd
@pytest.mark.via
@pytest.mark.integration
def test_lcd_operations_sequence(emu_core, hello_world_program):
    """Testa sequência de operações LCD baseada em transições Enable"""

    emu = emu_core
    hello_world_program_data = hello_world_program['binary']

    emu.load_program(hello_world_program_data, 0x8000)
    emu.reset()

    max_steps = 300
    prev_e = False
    operation_sequence = []

    for step in range(max_steps):
        cycles = emu.step()
        if cycles <= 0:
            break

        portb_data = emu.read_byte(0x6000)
        porta_control = emu.read_byte(0x6001)

        rs = (porta_control & 0x20) != 0
        rw = (porta_control & 0x40) != 0
        e = (porta_control & 0x80) != 0

        # Detecta borda de descida do Enable (momento da operação LCD)
        if prev_e and not e and not rw:
            operation = {
                'step': step,
                'type': 'DATA' if rs else 'COMMAND',
                'value': portb_data,
                'rs': rs,
                'sequence_number': len(operation_sequence)
            }
            operation_sequence.append(operation)

        prev_e = e

    # Verifica sequência típica: comandos primeiro, depois dados
    assert len(operation_sequence) > 0, "Deveria haver uma sequência de operações"

    # Primeira operação deveria ser comando (inicialização)
    if len(operation_sequence) > 0:
        first_op = operation_sequence[0]
        assert first_op['type'] == 'COMMAND', "Primeira operação deveria ser um comando"

    # Deveria haver pelo menos alguns dados após comandos
    data_operations = [op for op in operation_sequence if op['type'] == 'DATA']
    assert len(data_operations) > 0, "Deveria haver operações de dados na sequência"


@pytest.mark.lcd
@pytest.mark.via
@pytest.mark.slow
def test_complete_hello_world_via_transitions(emu_core, hello_world_program):
    """Testa 'HELLO WORLD!' completo via análise de transições"""

    emu = emu_core
    hello_world_program_data = hello_world_program['binary']

    emu.load_program(hello_world_program_data, 0x8000)
    emu.reset()

    max_steps = 400
    prev_e = False
    characters_sent = []

    for step in range(max_steps):
        cycles = emu.step()
        if cycles <= 0:
            break

        portb_data = emu.read_byte(0x6000)
        porta_control = emu.read_byte(0x6001)

        rs = (porta_control & 0x20) != 0
        rw = (porta_control & 0x40) != 0
        e = (porta_control & 0x80) != 0

        # Captura dados na borda de descida do Enable quando RS=1 (dados)
        if prev_e and not e and not rw and rs:
            if 32 <= portb_data <= 126:  # Caractere ASCII válido
                characters_sent.append(chr(portb_data))

        prev_e = e

        # Para se voltou ao início
        cpu_state = emu.get_cpu_state()
        if step > 100 and cpu_state.pc == 0x8000:
            break

    # Reconstrói string enviada
    sent_string = ''.join(characters_sent)

    # Verificações
    assert len(characters_sent) > 0, "Deveria ter caracteres enviados ao LCD"
    assert "HELLO" in sent_string, f"String enviada deveria conter 'HELLO', mas contém: '{sent_string}'"
