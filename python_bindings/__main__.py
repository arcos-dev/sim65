#!/usr/bin/env python3
"""
Ponto de entrada principal do pacote python_bindings
Permite execução como: python -m python_bindings
"""

import sys
import os

# Adicionar o diretório pai ao sys.path para permitir imports como python_bindings.xxx
current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.dirname(current_dir)
if parent_dir not in sys.path:
    sys.path.insert(0, parent_dir)

def main():
    """Função principal para execução como módulo"""
    try:
        # Verificar se as dependências estão instaladas
        try:
            import PyQt6
        except ImportError:
            print("PyQt6 não está instalado!")
            print("Instale as dependências com: pip install -r requirements.txt")
            sys.exit(1)
        
        # Importar e executar a GUI
        print("Iniciando EMU65 GUI...")
        from python_bindings.emu65_gui import main as gui_main
        gui_main()
        
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