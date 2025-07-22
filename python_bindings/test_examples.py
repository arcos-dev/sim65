#!/usr/bin/env python3
"""
Teste de Exemplos - EMU65
=========================

Script para testar se os exemplos estão funcionando corretamente
com os arquivos binários.

Autor: Anderson Costa
Versão: 2.0.0
Data: 2025-01-06
"""

import os
import sys
from emu65_core import Emu65Core
from simple_examples import SimpleExamples

def test_example_loading():
    """Testa o carregamento de todos os exemplos"""
    print("=== Teste de Carregamento de Exemplos ===\n")
    
    # Inicializar core
    try:
        core = Emu65Core({
            'clock_frequency': 1000000.0,
            'debug_mode': False,
            'trace_execution': False
        })
        print("✓ Core inicializado com sucesso")
    except Exception as e:
        print(f"✗ Erro ao inicializar core: {e}")
        return False
    
    # Obter todos os exemplos
    examples = SimpleExamples.get_all_examples()
    print(f"✓ Encontrados {len(examples)} exemplos\n")
    
    success_count = 0
    
    for example in examples:
        print(f"Testando: {example['name']}")
        print(f"  Descrição: {example['description']}")
        print(f"  Arquivo: {example['bin_path']}")
        print(f"  Endereço: 0x{example['start_address']:04X}")
        
        # Verificar se o arquivo existe
        if not os.path.exists(example['bin_path']):
            print(f"  ✗ Arquivo não encontrado: {example['bin_path']}")
            continue
        
        # Verificar tamanho do arquivo
        file_size = os.path.getsize(example['bin_path'])
        print(f"  Tamanho: {file_size} bytes")
        
        try:
            # Carregar o arquivo
            with open(example['bin_path'], 'rb') as f:
                binary_data = f.read()
            
            # Reset do core
            core.reset()
            
            # Carregar o programa
            core.load_program(binary_data, example['start_address'])
            
            # Reset novamente para garantir que o PC está correto
            core.reset()
            
            print(f"  ✓ Carregado com sucesso")
            success_count += 1
            
        except Exception as e:
            print(f"  ✗ Erro ao carregar: {e}")
        
        print()
    
    print(f"=== Resultado: {success_count}/{len(examples)} exemplos carregados com sucesso ===\n")
    
    # Limpeza
    core.destroy()
    
    return success_count == len(examples)

def test_lcd_functionality():
    """Testa a funcionalidade do LCD com o exemplo Hello World"""
    print("=== Teste de Funcionalidade do LCD ===\n")
    
    try:
        core = Emu65Core({
            'clock_frequency': 1000000.0,
            'debug_mode': False,
            'trace_execution': False
        })
        print("✓ Core inicializado")
        
        # Carregar exemplo Hello World
        hello_example = SimpleExamples.hello_world()
        
        if not os.path.exists(hello_example['bin_path']):
            print(f"✗ Arquivo não encontrado: {hello_example['bin_path']}")
            return False
        
        with open(hello_example['bin_path'], 'rb') as f:
            binary_data = f.read()
        
        core.reset()
        core.load_program(binary_data, hello_example['start_address'])
        core.reset()
        
        print("✓ Exemplo Hello World carregado")
        
        # Executar alguns passos para ver se o LCD é atualizado
        print("\nExecutando 50 passos...")
        for i in range(50):
            core.step()
            
            # Verificar estado do LCD a cada 10 passos
            if i % 10 == 0:
                lcd_state = core.get_lcd_state()
                display_bytes = bytes(lcd_state.display)
                display_text = display_bytes.decode('ascii', errors='ignore')
                
                # Extrair linhas do display
                null_pos1 = display_text.find('\x00')
                if null_pos1 >= 0:
                    row1 = display_text[:null_pos1]
                    remaining = display_text[null_pos1 + 1:]
                    null_pos2 = remaining.find('\x00')
                    row2 = remaining[:null_pos2] if null_pos2 >= 0 else remaining
                else:
                    row1 = display_text[:16]
                    row2 = display_text[17:33] if len(display_text) >= 33 else ""
                
                print(f"  Passo {i}: LCD='{row1}' | '{row2}' (ON:{lcd_state.display_on})")
        
        print("✓ Teste de execução concluído")
        
        # Limpeza
        core.destroy()
        return True
        
    except Exception as e:
        print(f"✗ Erro no teste do LCD: {e}")
        return False

def main():
    """Função principal"""
    print("EMU65 - Teste de Exemplos")
    print("=" * 40)
    
    # Teste 1: Carregamento de exemplos
    test1_passed = test_example_loading()
    
    print("\n" + "=" * 40 + "\n")
    
    # Teste 2: Funcionalidade do LCD
    test2_passed = test_lcd_functionality()
    
    print("\n" + "=" * 40)
    print("RESULTADO FINAL:")
    print(f"✓ Carregamento de exemplos: {'PASSOU' if test1_passed else 'FALHOU'}")
    print(f"✓ Funcionalidade do LCD: {'PASSOU' if test2_passed else 'FALHOU'}")
    
    if test1_passed and test2_passed:
        print("\n🎉 Todos os testes passaram!")
        return 0
    else:
        print("\n❌ Alguns testes falharam!")
        return 1

if __name__ == "__main__":
    sys.exit(main()) 