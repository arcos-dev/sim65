#!/usr/bin/env python3
"""
Script de limpeza do diretório python_bindings.

Remove arquivos temporários, cache e outros arquivos desnecessários
mantendo apenas os arquivos essenciais do projeto.

Autor: Anderson Costa
Data: 2025-01-22
"""

import os
import shutil
import glob
from pathlib import Path


def clean_pycache():
    """Remove diretórios __pycache__."""
    print("🧹 Limpando __pycache__...")

    for pycache in glob.glob("**/__pycache__", recursive=True):
        if os.path.isdir(pycache):
            print(f"  Removendo: {pycache}")
            shutil.rmtree(pycache)


def clean_pyc_files():
    """Remove arquivos .pyc."""
    print("🧹 Limpando arquivos .pyc...")

    for pyc_file in glob.glob("**/*.pyc", recursive=True):
        if os.path.isfile(pyc_file):
            print(f"  Removendo: {pyc_file}")
            os.remove(pyc_file)


def clean_build_artifacts():
    """Remove artefatos de build."""
    print("🧹 Limpando artefatos de build...")

    # Diretório build
    if os.path.exists("build"):
        print("  Removendo: build/")
        shutil.rmtree("build")

    # Arquivos objeto
    for obj_file in glob.glob("*.o"):
        print(f"  Removendo: {obj_file}")
        os.remove(obj_file)

    # Arquivos de debug
    for debug_file in glob.glob("*.pdb"):
        print(f"  Removendo: {debug_file}")
        os.remove(debug_file)


def clean_test_artifacts():
    """Remove artefatos de teste."""
    print("🧹 Limpando artefatos de teste...")

    # Coverage
    if os.path.exists(".coverage"):
        print("  Removendo: .coverage")
        os.remove(".coverage")

    if os.path.exists("htmlcov"):
        print("  Removendo: htmlcov/")
        shutil.rmtree("htmlcov")

    # Pytest cache
    if os.path.exists(".pytest_cache"):
        print("  Removendo: .pytest_cache/")
        shutil.rmtree(".pytest_cache")


def clean_temporary_files():
    """Remove arquivos temporários."""
    print("🧹 Limpando arquivos temporários...")

    # Arquivos temporários comuns
    temp_patterns = ["*.tmp", "*.temp", "*~", "*.bak"]

    for pattern in temp_patterns:
        for temp_file in glob.glob(pattern):
            print(f"  Removendo: {temp_file}")
            os.remove(temp_file)


def clean_logs():
    """Remove arquivos de log."""
    print("🧹 Limpando logs...")

    log_patterns = ["*.log", "debug.txt", "trace.txt"]

    for pattern in log_patterns:
        for log_file in glob.glob(pattern):
            print(f"  Removendo: {log_file}")
            os.remove(log_file)


def clean_test_files():
    """Remove arquivos de teste desnecessários para a GUI (apenas na raiz)."""
    print("🧹 Limpando arquivos de teste soltos na raiz...")

    # Arquivos de teste específicos na raiz (não a pasta tests/)
    test_files = [
        "test_simple.py",
        "test_report.py",
        "run_tests.py",
        "simple_examples.py",
        "pytest.ini",
        "test_lcd.py",
        "test_lcd_functionality.py",
        "test_lcd_manual.py",
        "test_init.c",
        "test_examples.py"  # Na raiz (diferente do tests/test_examples.py)
    ]

    for test_file in test_files:
        if os.path.exists(test_file):
            print(f"  Removendo: {test_file}")
            os.remove(test_file)

    # NOTA: A pasta tests/ é mantida pois contém testes estruturados importantes


def show_disk_usage():
    """Mostra uso de disco antes e depois."""
    def get_size(path):
        total = 0
        for dirpath, dirnames, filenames in os.walk(path):
            for filename in filenames:
                filepath = os.path.join(dirpath, filename)
                if os.path.exists(filepath):
                    total += os.path.getsize(filepath)
        return total

    size = get_size(".")
    return size / (1024 * 1024)  # MB


def main():
    """Função principal."""
    print("🧹 Iniciando limpeza do python_bindings...")

    initial_size = show_disk_usage()
    print(f"📊 Tamanho inicial: {initial_size:.2f} MB")

    # Executar limpezas
    clean_pycache()
    clean_pyc_files()
    clean_build_artifacts()
    clean_test_artifacts()
    clean_temporary_files()
    clean_logs()
    clean_test_files()

    final_size = show_disk_usage()
    saved = initial_size - final_size

    print(f"📊 Tamanho final: {final_size:.2f} MB")
    if saved > 0:
        print(f"💾 Espaço economizado: {saved:.2f} MB")

    print("✅ Limpeza concluída!")

    # Mostrar estrutura limpa
    print("\n📁 Estrutura após limpeza:")
    for root, dirs, files in os.walk("."):
        # Ignorar diretórios ocultos
        dirs[:] = [d for d in dirs if not d.startswith('.')]

        level = root.replace(".", "").count(os.sep)
        indent = " " * 2 * level
        print(f"{indent}{os.path.basename(root)}/")

        subindent = " " * 2 * (level + 1)
        for file in files:
            if not file.startswith('.'):
                print(f"{subindent}{file}")


if __name__ == '__main__':
    main()
