#!/usr/bin/env python3
"""
Teste dos Programas de Exemplo
==============================

Script para testar os programas de exemplo do EMU65.

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

def test_total_programs():
    programs = Programs6502.get_all_programs()
    assert len(programs) == 13

def test_program_fields():
    programs = Programs6502.get_all_programs()
    for prog in programs:
        assert 'name' in prog
        assert 'description' in prog
        assert 'binary' in prog
        assert 'start_address' in prog
        assert isinstance(prog['binary'], bytes)
        assert isinstance(prog['start_address'], int)

def test_unique_names():
    programs = Programs6502.get_all_programs()
    names = [prog['name'] for prog in programs]
    assert len(names) == len(set(names)), "Nomes de programas devem ser únicos"

def test_binary_not_empty():
    programs = Programs6502.get_all_programs()
    for prog in programs:
        assert len(prog['binary']) > 0, f"Programa {prog['name']} tem binário vazio"

def test_start_address():
    programs = Programs6502.get_all_programs()
    for prog in programs:
        assert 0x0000 <= prog['start_address'] <= 0xFFFF, f"Endereço inválido em {prog['name']}" 