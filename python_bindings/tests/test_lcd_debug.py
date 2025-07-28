#!/usr/bin/env python3
"""
Testes de Debug do LCD
=====================

Testes para verificar se o processamento do LCD baseado em steps está funcionando.
Convertido para formato pytest.

Autor: Anderson Costa
Versão: 1.0.0
Data: 2025-01-27
"""

import pytest


@pytest.mark.lcd
@pytest.mark.integration
def test_lcd_debug_with_steps(emu_core, hello_world_program):
    """Testa o LCD com debug output step-by-step"""

    emu = emu_core
    hello_world_program_data = hello_world_program['binary']

    # Carrega o programa em 0x8000
    emu.load_program(hello_world_program_data, 0x8000)

    # Reset CPU para começar no nosso programa
    emu.reset()

    max_steps = 50  # Suficiente para ver os primeiros caracteres
    lcd_changes = []

    for step in range(max_steps):
        # Estado do LCD antes do step
        lcd_before = emu.get_lcd_state()
        line1_before = lcd_before.display[:16].decode('utf-8', errors='replace').rstrip('\x00')

        # Executa um step
        cycles = emu.step()
        if cycles <= 0:
            break

        # Estado do LCD depois do step
        lcd_after = emu.get_lcd_state()
        line1_after = lcd_after.display[:16].decode('utf-8', errors='replace').rstrip('\x00')

        # Verifica se o estado do LCD mudou
        if line1_before != line1_after:
            change_info = {
                'step': step,
                'before': line1_before,
                'after': line1_after,
                'cursor_row': lcd_after.cursor_row,
                'cursor_col': lcd_after.cursor_col
            }
            lcd_changes.append(change_info)

    # Verificações
    assert len(lcd_changes) > 0, "LCD deveria ter pelo menos uma mudança durante a execução"

    # Estado final do LCD
    lcd_final = emu.get_lcd_state()
    line1_final = lcd_final.display[:16].decode('utf-8', errors='replace').rstrip('\x00')

    # O Hello World deveria exibir pelo menos algum texto
    assert len(line1_final.strip()) > 0, f"LCD deveria ter algum texto, mas contém: '{line1_final}'"


@pytest.mark.lcd
@pytest.mark.unit
def test_lcd_state_structure(emu_core):
    """Testa se a estrutura do estado do LCD está correta"""

    emu = emu_core
    lcd_state = emu.get_lcd_state()

    # Verifica se tem os campos esperados
    assert hasattr(lcd_state, 'display')
    assert hasattr(lcd_state, 'cursor_row')
    assert hasattr(lcd_state, 'cursor_col')
    assert hasattr(lcd_state, 'display_on')
    assert hasattr(lcd_state, 'cursor_on')
    assert hasattr(lcd_state, 'blink_on')

    # Verifica tipos
    assert isinstance(lcd_state.cursor_row, int)
    assert isinstance(lcd_state.cursor_col, int)
    assert isinstance(lcd_state.display_on, bool)


@pytest.mark.lcd
@pytest.mark.integration
def test_via_lcd_interaction(emu_core, hello_world_program):
    """Testa a interação entre VIA e LCD durante execução"""

    emu = emu_core
    hello_world_program_data = hello_world_program['binary']

    # Carrega o programa
    emu.load_program(hello_world_program_data, 0x8000)
    emu.reset()

    via_interactions = []
    max_steps = 30

    for step in range(max_steps):
        cycles = emu.step()
        if cycles <= 0:
            break

        # Verifica estado do VIA
        portb = emu.read_byte(0x6000)
        porta = emu.read_byte(0x6001)

        if portb != 0 or porta != 0:
            rs = (porta & 0x20) != 0
            e = (porta & 0x80) != 0

            interaction = {
                'step': step,
                'portb': portb,
                'porta': porta,
                'rs': rs,
                'e': e
            }
            via_interactions.append(interaction)

    # Deveria haver pelo menos algumas interações VIA
    assert len(via_interactions) > 0, "Deveria haver interações VIA durante a execução do Hello World"
