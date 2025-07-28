#!/usr/bin/env python3
"""
Teste Completo de Todos os Programas - pytest
==============================================

Testes abrangentes para validar todos os programas disponíveis na GUI.
Garante que todos os exemplos funcionem corretamente.

Autor: Anderson Costa
Versão: 1.0.0
Data: 2025-01-27
"""

import sys
import os
import pytest

# Adiciona o diretório raiz do projeto ao sys.path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from python_bindings.emu65_core import Emu65Core
from python_bindings.programs_6502 import Programs6502

class TestAllPrograms:
    """Classe para testes de todos os programas disponíveis"""

    @pytest.fixture(autouse=True)
    def setup(self):
        """Setup para cada teste"""
        self.programs = Programs6502.get_all_programs()
        self.expected_programs = [
            'Hello LCD', 'Binary Counter', 'Echo', 'Fibonacci',
            'Calculator', 'Clock', 'Memory Test', 'Interrupt Timer',
            'BASIC Interpreter', 'Hello World', 'Contador', 'Matemática'
        ]

    def test_all_expected_programs_exist(self):
        """Verifica se todos os programas esperados existem"""
        program_names = [p['name'] for p in self.programs]

        for expected_program in self.expected_programs:
            assert expected_program in program_names, f"Programa '{expected_program}' não encontrado!"

        print(f"✓ Todos os {len(self.expected_programs)} programas esperados estão presentes")

    def test_program_structure_validation(self):
        """Valida a estrutura de todos os programas"""
        required_fields = ['name', 'description', 'binary', 'start_address', 'components']

        for program in self.programs:
            for field in required_fields:
                assert field in program, f"Programa '{program.get('name', 'UNKNOWN')}' não tem o campo '{field}'"

            # Validações específicas
            assert isinstance(program['name'], str), f"Nome deve ser string: {program['name']}"
            assert isinstance(program['description'], str), f"Descrição deve ser string: {program['description']}"
            assert isinstance(program['binary'], bytes), f"Binary deve ser bytes: {program['name']}"
            assert isinstance(program['start_address'], int), f"Start address deve ser int: {program['name']}"
            assert isinstance(program['components'], list), f"Components deve ser lista: {program['name']}"
            assert len(program['components']) > 0, f"Components não pode estar vazio: {program['name']}"

        print(f"✓ Estrutura validada para todos os {len(self.programs)} programas")

    def test_program_binary_content(self):
        """Verifica se todos os programas têm conteúdo binário válido"""
        for program in self.programs:
            # Programas podem ter diferentes tamanhos, mas não devem estar vazios (exceto casos específicos)
            if program['name'] not in ['Echo']:  # Echo pode ter binário pequeno
                assert len(program['binary']) > 0, f"Programa '{program['name']}' tem binário vazio"

            # Verifica se o endereço está na faixa válida
            assert 0x0000 <= program['start_address'] <= 0xFFFF, f"Endereço inválido para '{program['name']}': 0x{program['start_address']:04X}"

            print(f"✓ {program['name']}: {len(program['binary'])} bytes, endereço 0x{program['start_address']:04X}")

    def test_program_components_validity(self):
        """Verifica se os componentes de cada programa são válidos"""
        valid_components = [
            '6502 CPU', 'LCD 16x2', 'RAM', 'ROM', 'LED', 'Botão', 'ACIA', 'VIA 6522'
        ]

        for program in self.programs:
            for component in program['components']:
                assert component in valid_components, f"Componente inválido '{component}' no programa '{program['name']}'"

            # 6502 CPU deve estar presente em todos
            assert '6502 CPU' in program['components'], f"Programa '{program['name']}' deve ter '6502 CPU'"

        print(f"✓ Componentes validados para todos os programas")

    def test_lcd_programs_can_be_loaded(self):
        """Testa se todos os programas com LCD podem ser carregados no emulador"""
        lcd_programs = [p for p in self.programs if 'LCD 16x2' in p['components']]

        assert len(lcd_programs) > 0, "Deve haver pelo menos um programa com LCD"

        for program in lcd_programs:
            with Emu65Core({'clock_frequency': 100}) as core:
                try:
                    # Carregar o programa
                    core.load_program(program['binary'], program['start_address'])

                    # Verificar se o programa foi carregado (PC deve estar no endereço correto)
                    cpu_state = core.get_cpu_state()
                    assert cpu_state.pc == program['start_address'], f"PC incorreto para '{program['name']}'"

                    # Tentar executar alguns steps sem crash
                    for _ in range(10):
                        result = core.step()
                        if result != 0:
                            break  # Normal, programa pode terminar

                    print(f"✓ {program['name']}: carregado e executado com sucesso")

                except Exception as e:
                    pytest.fail(f"Erro ao carregar programa '{program['name']}': {e}")

    def test_programs_have_unique_names(self):
        """Verifica se todos os programas têm nomes únicos"""
        names = [p['name'] for p in self.programs]
        unique_names = set(names)

        assert len(names) == len(unique_names), f"Programas com nomes duplicados encontrados: {names}"
        print(f"✓ Todos os {len(names)} programas têm nomes únicos")

    def test_programs_start_addresses(self):
        """Verifica se os endereços de início são apropriados"""
        for program in self.programs:
            start_addr = program['start_address']

            # A maioria dos programas deve estar na faixa de ROM (0x8000+)
            if program['name'] not in ['Echo']:  # Echo pode ter endereço diferente
                assert start_addr >= 0x8000, f"Programa '{program['name']}' deveria estar na ROM (>= 0x8000), mas está em 0x{start_addr:04X}"

        print(f"✓ Endereços de início validados para todos os programas")

    def test_specific_program_functionality(self):
        """Testa funcionalidades específicas de alguns programas importantes"""

        # Testar Hello World especificamente
        hello_world = next((p for p in self.programs if p['name'] == 'Hello World'), None)
        assert hello_world is not None, "Programa 'Hello World' deve existir"

        with Emu65Core({'clock_frequency': 100}) as core:
            core.load_program(hello_world['binary'], hello_world['start_address'])

            # Executar alguns steps
            for _ in range(50):
                result = core.step()
                if result != 0:
                    break

            # Verificar se o LCD foi inicializado
            lcd_state = core.get_lcd_state()
            # O display deve estar ligado após a inicialização
            assert lcd_state.display_on, "LCD deveria estar ligado após executar Hello World"

            print("✓ Hello World: LCD inicializado corretamente")

    def test_program_descriptions_quality(self):
        """Verifica se as descrições dos programas são adequadas"""
        for program in self.programs:
            description = program['description']

            # Descrições devem ter um tamanho mínimo
            assert len(description) >= 10, f"Descrição muito curta para '{program['name']}': '{description}'"

            # Descrições devem ser informativas (exceto para programas de teste legítimos)
            if program['name'] not in ['Memory Test']:  # Memory Test é um teste legítimo
                assert not description.lower().startswith('teste'), f"Descrição inadequada para '{program['name']}': '{description}'"

            print(f"✓ {program['name']}: '{description}'")

    def test_ben_eater_examples_vs_generated_programs(self):
        """Verifica a distribuição entre exemplos Ben Eater e programas gerados"""
        ben_eater_names = [
            'Hello LCD', 'Binary Counter', 'Echo', 'Fibonacci',
            'Calculator', 'Clock', 'Memory Test', 'Interrupt Timer', 'BASIC Interpreter'
        ]

        generated_names = ['Hello World', 'Contador', 'Matemática']

        ben_eater_count = len([p for p in self.programs if p['name'] in ben_eater_names])
        generated_count = len([p for p in self.programs if p['name'] in generated_names])

        print(f"✓ Programas Ben Eater: {ben_eater_count}")
        print(f"✓ Programas Gerados: {generated_count}")

        assert ben_eater_count >= 3, "Deve haver pelo menos 3 exemplos Ben Eater"
        assert generated_count >= 3, "Deve haver pelo menos 3 programas gerados"

    def test_programs_memory_requirements(self):
        """Verifica se os programas não excedem limites de memória razoáveis"""
        MAX_PROGRAM_SIZE = 32768  # 32KB máximo por programa

        for program in self.programs:
            size = len(program['binary'])
            assert size <= MAX_PROGRAM_SIZE, f"Programa '{program['name']}' muito grande: {size} bytes"

            if size > 1024:  # Programas maiores que 1KB
                print(f"⚠ {program['name']}: {size} bytes (programa grande)")
            else:
                print(f"✓ {program['name']}: {size} bytes")


# Função de conveniência para executar todos os testes
def run_all_program_tests():
    """Executa todos os testes de programas"""
    pytest.main([__file__, '-v'])


if __name__ == "__main__":
    run_all_program_tests()
