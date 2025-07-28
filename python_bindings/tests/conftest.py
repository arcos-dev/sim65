#!/usr/bin/env python3
"""
Configuração do pytest para os testes do EMU65
==============================================

Este arquivo configura os fixtures e configurações comuns para todos os testes.

Autor: Anderson Costa
Versão: 1.0.0
Data: 2025-01-27
"""

import pytest
import sys
import os

# Adiciona o diretório python_bindings ao path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

@pytest.fixture
def emu_core():
    """Fixture para criar uma instância do core do emulador"""
    from emu65_core import Emu65Core
    with Emu65Core() as core:
        yield core

@pytest.fixture
def programs_6502():
    """Fixture para criar uma instância do gerador de programas"""
    from programs_6502 import Programs6502
    return Programs6502()

@pytest.fixture
def hello_world_program(programs_6502):
    """Fixture para obter o programa Hello World"""
    return programs_6502.hello_world()

# Configuração para capturar prints durante os testes
def pytest_configure(config):
    """Configuração do pytest"""
    pass

# Configuração de marcadores personalizados
def pytest_configure(config):
    """Registra marcadores customizados"""
    config.addinivalue_line(
        "markers", "slow: marca testes que são lentos para executar"
    )
    config.addinivalue_line(
        "markers", "integration: marca testes de integração"
    )
    config.addinivalue_line(
        "markers", "unit: marca testes unitários"
    )
    config.addinivalue_line(
        "markers", "lcd: marca testes relacionados ao LCD"
    )
    config.addinivalue_line(
        "markers", "via: marca testes relacionados ao VIA"
    )
    config.addinivalue_line(
        "markers", "cpu: marca testes relacionados ao CPU"
    )
