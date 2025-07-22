#!/usr/bin/env python3
"""
Teste de Componentes Automáticos
================================

Script para testar a adição automática de componentes nos exemplos.

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
from python_bindings.programs_6502 import Programs6502

def test_components_field_exists():
    programs = Programs6502.get_all_programs()
    for prog in programs:
        assert 'components' in prog, f"Programa {prog['name']} não possui campo 'components'"
        assert isinstance(prog['components'], list)
        assert len(prog['components']) > 0

def test_component_names_are_strings():
    programs = Programs6502.get_all_programs()
    for prog in programs:
        for comp in prog['components']:
            assert isinstance(comp, str)

def test_component_usage_summary():
    # Apenas para garantir que todos os componentes esperados aparecem em pelo menos um programa
    expected = {'6502 CPU', 'LCD 16x2', 'LED', 'Botão', 'RAM', 'ROM', 'Resistor', 'Capacitor'}
    found = set()
    programs = Programs6502.get_all_programs()
    for prog in programs:
        found.update(prog['components'])
    assert expected.issubset(found), f"Nem todos os componentes esperados estão presentes: {expected - found}" 