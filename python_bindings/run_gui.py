#!/usr/bin/env python3
"""
Script de Execução da GUI EMU65
===============================

Script robusto para executar a interface gráfica do EMU65,
com detecção automática do contexto de execução.

Autor: Anderson Costa
Versão: 3.0.0
Data: 2025-01-06
"""

import sys
import os

def setup_environment():
    """Configura o ambiente de execução"""
    # Obter o diretório do script atual
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Se estamos executando de dentro da pasta python_bindings
    if os.path.basename(script_dir) == 'python_bindings':
        # Adicionar o diretório pai ao sys.path para permitir imports como python_bindings.xxx
        parent_dir = os.path.dirname(script_dir)
        if parent_dir not in sys.path:
            sys.path.insert(0, parent_dir)
        print(f"Executando de dentro de python_bindings - adicionado {parent_dir} ao sys.path")
    else:
        # Executando de fora - adicionar o diretório atual ao sys.path
        if script_dir not in sys.path:
            sys.path.insert(0, script_dir)
        print(f"Executando de fora - adicionado {script_dir} ao sys.path")

def main():
    """Função principal de execução"""
    try:
        # Configurar ambiente
        setup_environment()
        
        # Verificar se as dependências estão instaladas
        try:
            import PyQt6
        except ImportError:
            print("PyQt6 não está instalado!")
            print("Instale as dependências com: pip install -r requirements.txt")
            sys.exit(1)
        
        # Importar e executar a GUI
        print("Iniciando EMU65 GUI...")
        import emu65_gui
        emu65_gui.main()
        
    except ImportError as e:
        print(f"Erro de importação: {e}")
        print("Certifique-se de que todas as dependências estão instaladas:")
        print("pip install -r requirements.txt")
        sys.exit(1)
    except Exception as e:
        print(f"Erro inesperado: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main() 