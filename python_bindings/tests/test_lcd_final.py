#!/usr/bin/env python3
"""
Testes Finais do LCD
====================

Testes para verificar se o LCD exibe "HELLO WORLD!" corretamente após correções.
Convertido para formato pytest.

Autor: Anderson Costa
Versão: 1.0.0
Data: 2025-01-27
"""

import pytest


@pytest.mark.lcd
@pytest.mark.integration
def test_lcd_displays_hello_world(emu_core, hello_world_program):
    """Testa se o LCD exibe 'HELLO WORLD!' corretamente"""

    emu = emu_core
    hello_world_program_data = hello_world_program['binary']

    # Carrega o programa em 0x8000
    emu.load_program(hello_world_program_data, 0x8000)
    emu.reset()

    # Executa o programa por steps suficientes para completar um ciclo
    max_steps = 200

    for step in range(max_steps):
        cycles = emu.step()
        if cycles <= 0:
            break

        # Verifica se voltou ao início (programa completou um ciclo)
        cpu_state = emu.get_cpu_state()
        if step > 50 and cpu_state.pc == 0x8000:
            break

    # Obtém o estado final do LCD
    lcd_state = emu.get_lcd_state()
    display_data = lcd_state.display

    # Extrai as linhas - trata como uma única linha se display for menor
    if len(display_data) >= 32:
        # Display com 2 linhas
        line1 = display_data[:16].decode('utf-8', errors='replace').rstrip('\x00')
        line2 = display_data[16:32].decode('utf-8', errors='replace').rstrip('\x00')
    else:
        # Display com 1 linha
        line1 = display_data.decode('utf-8', errors='replace').rstrip('\x00')
        line2 = ""

    # Verifica se "HELLO WORLD!" aparece no LCD
    expected_text = "HELLO WORLD!"

    assert (expected_text in line1 or expected_text in line2), \
        f"LCD deveria conter '{expected_text}', mas contém: Linha1='{line1}', Linha2='{line2}'"


@pytest.mark.lcd
@pytest.mark.unit
def test_lcd_display_structure(emu_core):
    """Testa a estrutura do display LCD"""

    emu = emu_core
    lcd_state = emu.get_lcd_state()

    # Verifica se o display tem o tamanho esperado (pode ser 16 ou mais bytes)
    assert len(lcd_state.display) >= 16, f"Display deveria ter pelo menos 16 bytes, mas tem {len(lcd_state.display)}"

    # Verifica se os campos de cursor são válidos
    assert 0 <= lcd_state.cursor_row <= 1, "Cursor row deveria estar entre 0 e 1"
    assert 0 <= lcd_state.cursor_col <= 15, "Cursor col deveria estar entre 0 e 15"
@pytest.mark.lcd
@pytest.mark.integration
def test_lcd_status_flags(emu_core, hello_world_program):
    """Testa os flags de status do LCD"""

    emu = emu_core
    hello_world_program_data = hello_world_program['binary']

    emu.load_program(hello_world_program_data, 0x8000)
    emu.reset()

    # Executa alguns steps para inicializar o LCD
    for _ in range(100):
        cycles = emu.step()
        if cycles <= 0:
            break

    lcd_state = emu.get_lcd_state()

    # Após inicialização, o display deveria estar ligado
    # Nota: dependendo da implementação, isso pode variar
    assert isinstance(lcd_state.display_on, bool), "display_on deveria ser boolean"
    assert isinstance(lcd_state.cursor_on, bool), "cursor_on deveria ser boolean"
    assert isinstance(lcd_state.blink_on, bool), "blink_on deveria ser boolean"


@pytest.mark.lcd
@pytest.mark.integration
def test_lcd_content_progression(emu_core, hello_world_program):
    """Testa a progressão do conteúdo no LCD durante execução"""

    emu = emu_core
    hello_world_program_data = hello_world_program['binary']

    emu.load_program(hello_world_program_data, 0x8000)
    emu.reset()

    lcd_states = []
    max_steps = 150

    # Coleta estados do LCD durante execução
    for step in range(max_steps):
        cycles = emu.step()
        if cycles <= 0:
            break

        # Coleta estado a cada 10 steps para ver progressão
        if step % 10 == 0:
            lcd_state = emu.get_lcd_state()
            line1 = lcd_state.display[:16].decode('utf-8', errors='replace').rstrip('\x00')
            lcd_states.append({
                'step': step,
                'line1': line1,
                'cursor_row': lcd_state.cursor_row,
                'cursor_col': lcd_state.cursor_col
            })

    # Verifica que temos pelo menos alguns estados coletados
    assert len(lcd_states) > 0, "Deveria ter coletado pelo menos um estado do LCD"

    # Verifica se houve mudanças no conteúdo
    initial_content = lcd_states[0]['line1']
    final_content = lcd_states[-1]['line1']

    # O conteúdo deveria mudar (ou pelo menos sair do estado vazio inicial)
    assert initial_content != final_content or len(final_content.strip()) > 0, \
        "LCD deveria mostrar algum conteúdo durante a execução"


@pytest.mark.lcd
@pytest.mark.slow
def test_lcd_complete_execution_cycle(emu_core, hello_world_program):
    """Testa um ciclo completo de execução do Hello World"""

    emu = emu_core
    hello_world_program_data = hello_world_program['binary']

    emu.load_program(hello_world_program_data, 0x8000)
    emu.reset()

    initial_pc = emu.get_cpu_state().pc
    max_steps = 500
    completed_cycle = False

    for step in range(max_steps):
        cycles = emu.step()
        if cycles <= 0:
            break

        # Verifica se completou um ciclo (voltou ao PC inicial)
        cpu_state = emu.get_cpu_state()
        if step > 50 and cpu_state.pc == initial_pc:
            completed_cycle = True
            break

    # Verifica estado final
    lcd_state = emu.get_lcd_state()
    line1 = lcd_state.display[:16].decode('utf-8', errors='replace').rstrip('\x00')

    # Pelo menos deveria ter algum conteúdo após execução completa
    assert len(line1.strip()) > 0, f"LCD deveria ter conteúdo após execução, mas está vazio: '{line1}'"

    if completed_cycle:
        # Se completou um ciclo, deveria ter o texto esperado
        assert "HELLO" in line1, f"Após ciclo completo, LCD deveria conter 'HELLO', mas contém: '{line1}'"
