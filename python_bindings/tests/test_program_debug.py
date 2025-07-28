#!/usr/bin/env python3
"""
Testes de Debug dos Programas
=============================

Testes para debugar e verificar a saída da função hello_world().
Convertido para formato pytest.

Autor: Anderson Costa
Versão: 1.0.0
Data: 2025-01-27
"""

import pytest


@pytest.mark.unit
def test_hello_world_program_structure(programs_6502):
    """Testa a estrutura do programa Hello World"""

    hello_world_program = programs_6502.hello_world()

    # Verifica tipo básico
    assert isinstance(hello_world_program, dict), \
        f"hello_world() deveria retornar dict, mas retornou {type(hello_world_program)}"

    # Verifica se tem os campos esperados
    assert 'binary' in hello_world_program, "Programa deveria ter campo 'binary'"
    assert 'name' in hello_world_program, "Programa deveria ter campo 'name'"

    # Verifica o campo binary
    binary_data = hello_world_program['binary']
    assert isinstance(binary_data, (bytes, bytearray)), \
        f"Campo 'binary' deveria ser bytes ou bytearray, mas é {type(binary_data)}"

    assert len(binary_data) > 0, "Programa binário não deveria estar vazio"


@pytest.mark.unit
def test_hello_world_binary_content(programs_6502):
    """Testa o conteúdo binário do Hello World"""

    hello_world_program = programs_6502.hello_world()
    binary_data = hello_world_program['binary']

    # Verifica tamanho razoável
    assert len(binary_data) > 10, f"Programa muito pequeno: {len(binary_data)} bytes"
    assert len(binary_data) < 10000, f"Programa muito grande: {len(binary_data)} bytes"

    # Verifica se tem dados válidos (não todos zeros)
    non_zero_bytes = sum(1 for b in binary_data if b != 0)
    assert non_zero_bytes > 0, "Programa não deveria ser apenas zeros"


@pytest.mark.unit
def test_hello_world_name_field(programs_6502):
    """Testa o campo name do Hello World"""

    hello_world_program = programs_6502.hello_world()

    assert isinstance(hello_world_program['name'], str), \
        "Campo 'name' deveria ser string"

    assert len(hello_world_program['name']) > 0, \
        "Campo 'name' não deveria estar vazio"

    # Deveria conter "hello" no nome
    name_lower = hello_world_program['name'].lower()
    assert 'hello' in name_lower, \
        f"Nome deveria conter 'hello', mas é: '{hello_world_program['name']}'"


@pytest.mark.unit
def test_programs_6502_all_programs(programs_6502):
    """Testa se Programs6502 retorna lista de programas"""

    all_programs = programs_6502.get_all_programs()

    assert isinstance(all_programs, list), \
        f"get_all_programs() deveria retornar lista, mas retornou {type(all_programs)}"

    assert len(all_programs) > 0, "Deveria haver pelo menos um programa disponível"

    # Verifica estrutura de cada programa
    for i, program in enumerate(all_programs):
        assert isinstance(program, dict), \
            f"Programa {i} deveria ser dict, mas é {type(program)}"

        assert 'name' in program, f"Programa {i} deveria ter campo 'name'"
        assert 'binary' in program, f"Programa {i} deveria ter campo 'binary'"


@pytest.mark.unit
def test_binary_data_conversion(programs_6502):
    """Testa conversão de dados binários"""

    hello_world_program = programs_6502.hello_world()
    binary_data = hello_world_program['binary']

    # Testa se pode iterar sobre os bytes
    try:
        byte_list = list(binary_data)
        assert len(byte_list) == len(binary_data), \
            "Iteração deveria produzir mesmo número de elementos"

        # Verifica se todos os elementos são bytes válidos
        for i, byte_val in enumerate(byte_list[:10]):  # Primeiros 10 bytes
            assert isinstance(byte_val, int), \
                f"Byte {i} deveria ser int, mas é {type(byte_val)}"
            assert 0 <= byte_val <= 255, \
                f"Byte {i} deveria estar no range 0-255, mas é {byte_val}"

    except Exception as e:
        pytest.fail(f"Erro ao iterar sobre dados binários: {e}")


@pytest.mark.unit
def test_program_metadata(programs_6502):
    """Testa metadados dos programas"""

    hello_world_program = programs_6502.hello_world()

    # Verifica campos opcionais comuns
    if 'description' in hello_world_program:
        assert isinstance(hello_world_program['description'], str), \
            "Campo 'description' deveria ser string"

    if 'load_address' in hello_world_program:
        load_addr = hello_world_program['load_address']
        assert isinstance(load_addr, int), \
            "Campo 'load_address' deveria ser int"
        assert 0 <= load_addr <= 0xFFFF, \
            "Load address deveria ser endereço 16-bit válido"

    if 'components' in hello_world_program:
        components = hello_world_program['components']
        assert isinstance(components, list), \
            "Campo 'components' deveria ser lista"


@pytest.mark.integration
def test_hello_world_can_be_loaded(emu_core, programs_6502):
    """Testa se o Hello World pode ser carregado no emulador"""

    emu = emu_core
    hello_world_program = programs_6502.hello_world()
    binary_data = hello_world_program['binary']

    # Testa carregamento no emulador
    try:
        emu.load_program(binary_data, 0x8000)
        # Se chegou aqui, o carregamento foi bem-sucedido
        assert True
    except Exception as e:
        pytest.fail(f"Erro ao carregar programa no emulador: {e}")


@pytest.mark.unit
def test_binary_data_hex_representation(programs_6502):
    """Testa representação hexadecimal dos dados binários"""

    hello_world_program = programs_6502.hello_world()
    binary_data = hello_world_program['binary']

    # Testa se pode converter para hex
    try:
        hex_repr = ' '.join(f'{b:02X}' for b in binary_data[:20])
        assert len(hex_repr) > 0, "Representação hex não deveria estar vazia"

        # Verifica formato (deveria ter espaços entre bytes)
        assert ' ' in hex_repr, "Representação hex deveria ter espaços"

    except Exception as e:
        pytest.fail(f"Erro ao criar representação hex: {e}")
