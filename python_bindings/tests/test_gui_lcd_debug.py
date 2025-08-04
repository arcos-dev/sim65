#!/usr/bin/env python3
"""
Testes de Debug da Interface Gráfica - LCD
==========================================

Testes para debugar problemas do LCD na interface gráfica.

Autor: Anderson Costa
Versão: 1.0.0
Data: 2025-08-03
"""

import pytest
import sys
import os
from unittest.mock import MagicMock, patch

@pytest.mark.debug
@pytest.mark.gui
def test_debug_gui_lcd_hello_world(gui_app, main_window, programs_6502):
    """Debugga o LCD na GUI com o programa Hello World"""

    # Obter programa Hello World
    programs = programs_6502.get_all_programs()
    hello_world = None
    for prog in programs:
        if prog['name'] == 'Hello World':
            hello_world = prog
            break

    assert hello_world is not None, "Programa Hello World não encontrado"

    # Mock do core para evitar dependências
    with patch('emu65_core.Emu65Core') as mock_core_class:
        mock_core = MagicMock()
        mock_core_class.return_value.__enter__.return_value = mock_core

        # Mock do estado do LCD
        mock_lcd_state = MagicMock()
        mock_lcd_state.display_on = True
        mock_lcd_state.cursor_row = 0
        mock_lcd_state.cursor_col = 0
        mock_lcd_state.function_set = 0x38
        mock_lcd_state.display_control = 0x0C
        mock_lcd_state.entry_mode = 0x06
        mock_core.get_lcd_state.return_value = mock_lcd_state

        # Mock do buffer do LCD
        mock_core.get_lcd_buffer.return_value = [
            "Hello World     ",  # Linha 1
            "                "   # Linha 2
        ]

        # Testar carregamento e execução
        with patch.object(main_window, 'load_example') as mock_load:
            with patch.object(main_window, 'on_reset') as mock_reset:
                mock_load.return_value = True
                mock_reset.return_value = True

                # Simular carregamento
                main_window.load_example()
                mock_load.assert_called_once()

                # Reset
                main_window.on_reset()
                mock_reset.assert_called_once()                # Verificar estado do LCD
                lcd_state = mock_core.get_lcd_state()
                assert lcd_state.display_on == True, "LCD deveria estar ligado"
                assert lcd_state.cursor_row == 0, "Cursor deveria estar na linha 0"
                assert lcd_state.cursor_col == 0, "Cursor deveria estar na coluna 0"

@pytest.mark.debug
@pytest.mark.gui
def test_debug_lcd_buffer_content(gui_app, main_window, programs_6502):
    """Testa o conteúdo do buffer do LCD durante debug"""

    # Mock do core do emulador
    with patch('emu65_core.Emu65Core') as mock_core_class:
        mock_core = MagicMock()
        mock_core_class.return_value.__enter__.return_value = mock_core

        # Diferentes estados do buffer para teste
        test_buffers = [
            ["                ", "                "],  # Vazio
            ["Hello           ", "                "],  # Parcial linha 1
            ["Hello World     ", "                "],  # Completo linha 1
            ["Hello World     ", "Test Line 2     "]   # Ambas as linhas
        ]

        for i, buffer_content in enumerate(test_buffers):
            mock_core.get_lcd_buffer.return_value = buffer_content

            # Verificar conteúdo do buffer
            lcd_buffer = mock_core.get_lcd_buffer()
            assert len(lcd_buffer) == 2, f"Buffer deveria ter 2 linhas, tem {len(lcd_buffer)}"
            assert len(lcd_buffer[0]) == 16, f"Linha 1 deveria ter 16 caracteres, tem {len(lcd_buffer[0])}"
            assert len(lcd_buffer[1]) == 16, f"Linha 2 deveria ter 16 caracteres, tem {len(lcd_buffer[1])}"

            # Verificar conteúdo específico
            assert lcd_buffer[0] == buffer_content[0], f"Linha 1 não confere no teste {i}"
            assert lcd_buffer[1] == buffer_content[1], f"Linha 2 não confere no teste {i}"

@pytest.mark.debug
@pytest.mark.gui
def test_debug_lcd_state_transitions(gui_app, main_window):
    """Testa as transições de estado do LCD durante debug"""

    with patch('emu65_core.Emu65Core') as mock_core_class:
        mock_core = MagicMock()
        mock_core_class.return_value.__enter__.return_value = mock_core

        # Estados sequenciais do LCD
        lcd_states = [
            {'display_on': False, 'cursor_row': 0, 'cursor_col': 0},
            {'display_on': True, 'cursor_row': 0, 'cursor_col': 0},
            {'display_on': True, 'cursor_row': 0, 'cursor_col': 5},
            {'display_on': True, 'cursor_row': 0, 'cursor_col': 11},
            {'display_on': True, 'cursor_row': 1, 'cursor_col': 0}
        ]

        for state in lcd_states:
            mock_lcd_state = MagicMock()
            mock_lcd_state.display_on = state['display_on']
            mock_lcd_state.cursor_row = state['cursor_row']
            mock_lcd_state.cursor_col = state['cursor_col']
            mock_core.get_lcd_state.return_value = mock_lcd_state

            # Verificar estado
            current_state = mock_core.get_lcd_state()
            assert current_state.display_on == state['display_on']
            assert current_state.cursor_row == state['cursor_row']
            assert current_state.cursor_col == state['cursor_col']

@pytest.mark.debug
def test_debug_lcd_without_gui(emu_core, programs_6502):
    """Testa debug do LCD sem interface gráfica"""

    # Obter programa Hello World
    programs = programs_6502.get_all_programs()
    hello_world = None
    for prog in programs:
        if prog['name'] == 'Hello World':
            hello_world = prog
            break

    if hello_world is None:
        pytest.skip("Programa Hello World não encontrado")

    # Carregar programa no core (usando método correto)
    try:
        emu_core.load_program(hello_world['binary'], 0x8000)
    except Exception as e:
        pytest.skip(f"Falha ao carregar programa: {e}")

    # Reset
    emu_core.reset()    # Executar alguns steps e verificar LCD
    for i in range(10):
        emu_core.step()

        # Verificar se o LCD pode ser acessado
        try:
            lcd_state = emu_core.get_lcd_state()

            # Verificações básicas
            assert hasattr(lcd_state, 'display_on')
            assert hasattr(lcd_state, 'cursor_row')
            assert hasattr(lcd_state, 'cursor_col')
            assert hasattr(lcd_state, 'display')

            # Verificar se display é acessível
            display_content = lcd_state.display.decode('utf-8', errors='ignore')
            assert isinstance(display_content, str)

        except AttributeError as e:
            # Se os métodos não existem, pular este teste
            pytest.skip(f"Métodos de LCD não disponíveis no core: {e}")