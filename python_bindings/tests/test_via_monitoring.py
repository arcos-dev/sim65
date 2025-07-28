#!/usr/bin/env python3
"""
Testes de Monitoramento VIA
===========================

Testes para monitorar registros VIA durante a execução do Hello World.
Convertido para formato pytest.

Autor: Anderson Costa
Versão: 1.0.0
Data: 2025-01-27
"""

import pytest


@pytest.mark.via
@pytest.mark.integration
def test_via_monitoring_during_execution(emu_core, hello_world_program):
    """Testa o monitoramento de registros VIA durante execução do Hello World"""

    emu = emu_core
    hello_world_program_data = hello_world_program['binary']

    # Carrega o programa em 0x8000
    emu.load_program(hello_world_program_data, 0x8000)
    emu.reset()

    via_operations = []
    max_steps = 1000  # Previne loops infinitos

    for step in range(max_steps):
        # Estado da CPU antes do step
        cpu_state = emu.get_cpu_state()
        pc_before = cpu_state.pc

        # Executa um step
        cycles = emu.step()
        if cycles <= 0:
            break

        # Estado da CPU depois do step
        cpu_state = emu.get_cpu_state()
        pc_after = cpu_state.pc

        # Lê registros VIA diretamente
        portb_data = emu.read_byte(0x6000)      # VIA PORTB (dados LCD)
        porta_control = emu.read_byte(0x6001)   # VIA PORTA (controle LCD)

        # Verifica se parece com operações LCD
        if porta_control != 0 or portb_data != 0:
            operation = {
                'step': step,
                'pc_before': pc_before,
                'pc_after': pc_after,
                'portb_data': portb_data,
                'porta_control': porta_control,
                'cycles': cycles
            }
            via_operations.append(operation)

        # Para se voltou ao início (programa completou loop)
        if step > 0 and pc_after == 0x8000:
            break

    # Verificações
    assert len(via_operations) > 0, "Deveria haver operações VIA detectadas"

    # Analisa as operações
    init_commands = 0
    data_writes = 0

    for op in via_operations:
        rs = (op['porta_control'] & 0x20) != 0
        e = (op['porta_control'] & 0x80) != 0

        if e and op['portb_data'] != 0:  # Conta apenas quando enable está high
            if rs:  # Dados
                data_writes += 1
            else:  # Comando
                init_commands += 1

    assert data_writes > 0, "Deveria haver pelo menos uma escrita de dados detectada"
    assert init_commands > 0, "Deveria haver pelo menos um comando de inicialização detectado"


@pytest.mark.via
@pytest.mark.unit
def test_via_register_access(emu_core):
    """Testa acesso básico aos registros VIA"""

    emu = emu_core

    # Testa leitura inicial (deveria ser 0)
    portb = emu.read_byte(0x6000)
    porta = emu.read_byte(0x6001)

    assert isinstance(portb, int), "PORTB deveria retornar um inteiro"
    assert isinstance(porta, int), "PORTA deveria retornar um inteiro"
    assert 0 <= portb <= 255, "PORTB deveria estar no range 0-255"
    assert 0 <= porta <= 255, "PORTA deveria estar no range 0-255"


@pytest.mark.via
@pytest.mark.integration
def test_lcd_control_signals(emu_core, hello_world_program):
    """Testa sinais de controle LCD específicos"""

    emu = emu_core
    hello_world_program_data = hello_world_program['binary']

    emu.load_program(hello_world_program_data, 0x8000)
    emu.reset()

    control_signals = {
        'rs_high': [],
        'rs_low': [],
        'enable_high': [],
        'enable_low': []
    }

    max_steps = 500

    for step in range(max_steps):
        cycles = emu.step()
        if cycles <= 0:
            break

        porta_control = emu.read_byte(0x6001)

        if porta_control != 0:
            rs = (porta_control & 0x20) != 0    # RS bit (PA5)
            rw = (porta_control & 0x40) != 0    # RW bit (PA6)
            e = (porta_control & 0x80) != 0     # Enable bit (PA7)

            signal_state = {
                'step': step,
                'porta': porta_control,
                'rs': rs,
                'rw': rw,
                'e': e
            }

            if rs:
                control_signals['rs_high'].append(signal_state)
            else:
                control_signals['rs_low'].append(signal_state)

            if e:
                control_signals['enable_high'].append(signal_state)
            else:
                control_signals['enable_low'].append(signal_state)

    # Verificações dos sinais de controle
    assert len(control_signals['enable_high']) > 0, "Deveria haver pulsos de enable detectados"
    assert len(control_signals['rs_high']) > 0, "Deveria haver sinais RS=1 (escrita de dados)"
    assert len(control_signals['rs_low']) > 0, "Deveria haver sinais RS=0 (comandos)"


@pytest.mark.via
@pytest.mark.slow
def test_via_operation_sequence(emu_core, hello_world_program):
    """Testa a sequência completa de operações VIA"""

    emu = emu_core
    hello_world_program_data = hello_world_program['binary']

    emu.load_program(hello_world_program_data, 0x8000)
    emu.reset()

    sequence = []
    max_steps = 1000

    for step in range(max_steps):
        cycles = emu.step()
        if cycles <= 0:
            break

        portb = emu.read_byte(0x6000)
        porta = emu.read_byte(0x6001)

        if portb != 0 or porta != 0:
            rs = (porta & 0x20) != 0
            e = (porta & 0x80) != 0

            operation = {
                'step': step,
                'data': portb,
                'control': porta,
                'rs': rs,
                'enable': e,
                'type': 'data' if rs else 'command'
            }
            sequence.append(operation)

    # Verifica que temos uma sequência válida
    assert len(sequence) > 0, "Deveria haver uma sequência de operações VIA"

    # Verifica que temos tanto comandos quanto dados
    commands = [op for op in sequence if op['type'] == 'command']
    data_ops = [op for op in sequence if op['type'] == 'data']

    assert len(commands) > 0, "Deveria haver comandos na sequência"
    assert len(data_ops) > 0, "Deveria haver operações de dados na sequência"
