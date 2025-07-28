#!/usr/bin/env python3
"""
Test Runner para EMU65
======================

Script para executar diferentes conjuntos de testes de forma organizada.

Uso:
    python run_tests.py                    # Executa todos os testes
    python run_tests.py --unit             # Executa apenas testes unitários
    python run_tests.py --integration      # Executa apenas testes de integração
    python run_tests.py --lcd              # Executa apenas testes LCD
    python run_tests.py --via              # Executa apenas testes VIA
    python run_tests.py --fast             # Executa testes rápidos (exclui slow)
    python run_tests.py --coverage         # Executa com relatório de coverage

Autor: Anderson Costa
Versão: 1.0.0
Data: 2025-01-27
"""

import sys
import subprocess
import argparse
import os

def run_pytest(args_list):
    """Executa pytest com argumentos específicos"""
    cmd = [sys.executable, "-m", "pytest"] + args_list

    print(f"Executando: {' '.join(cmd)}")
    print("=" * 60)

    try:
        result = subprocess.run(cmd, cwd=os.path.dirname(__file__))
        return result.returncode == 0
    except Exception as e:
        print(f"Erro ao executar testes: {e}")
        return False

def main():
    """Função principal"""
    parser = argparse.ArgumentParser(description="Execute testes do EMU65")

    # Tipos de teste
    parser.add_argument("--unit", action="store_true",
                       help="Executa apenas testes unitários")
    parser.add_argument("--integration", action="store_true",
                       help="Executa apenas testes de integração")
    parser.add_argument("--lcd", action="store_true",
                       help="Executa apenas testes relacionados ao LCD")
    parser.add_argument("--via", action="store_true",
                       help="Executa apenas testes relacionados ao VIA")
    parser.add_argument("--cpu", action="store_true",
                       help="Executa apenas testes relacionados ao CPU")
    parser.add_argument("--gui", action="store_true",
                       help="Executa apenas testes da GUI")

    # Modificadores
    parser.add_argument("--fast", action="store_true",
                       help="Executa apenas testes rápidos (exclui marcador 'slow')")
    parser.add_argument("--slow", action="store_true",
                       help="Executa apenas testes lentos")
    parser.add_argument("--coverage", action="store_true",
                       help="Executa com relatório de coverage")
    parser.add_argument("--verbose", "-v", action="store_true",
                       help="Saída verbose")
    parser.add_argument("--quiet", "-q", action="store_true",
                       help="Saída silenciosa")
    parser.add_argument("--headless", action="store_true",
                       help="Executa testes GUI em modo headless")

    args = parser.parse_args()

    # Monta argumentos do pytest
    pytest_args = []

    # Configurações de output
    if args.verbose:
        pytest_args.extend(["-v", "-s"])
    elif args.quiet:
        pytest_args.append("-q")

    # Marcadores
    markers = []

    if args.unit:
        markers.append("unit")
    if args.integration:
        markers.append("integration")
    if args.lcd:
        markers.append("lcd")
    if args.via:
        markers.append("via")
    if args.cpu:
        markers.append("cpu")
    if args.gui:
        markers.append("gui")

    # Modificadores de velocidade
    if args.fast:
        markers.append("not slow")
    elif args.slow:
        markers.append("slow")

    # Adiciona marcadores ao pytest
    if markers:
        pytest_args.extend(["-m", " and ".join(markers)])

    # Coverage
    if args.coverage:
        pytest_args.extend(["--cov=.", "--cov-report=term-missing"])

    # Configuração headless para GUI
    if args.headless and args.gui:
        os.environ["QT_QPA_PLATFORM"] = "offscreen"

    # Executa os testes
    print("EMU65 Test Runner")
    print("=" * 60)

    if markers:
        print(f"Executando testes com marcadores: {' and '.join(markers)}")
    else:
        print("Executando todos os testes")

    print()

    success = run_pytest(pytest_args)

    print()
    print("=" * 60)

    if success:
        print("✅ Todos os testes passaram!")
        return 0
    else:
        print("❌ Alguns testes falharam!")
        return 1

if __name__ == "__main__":
    sys.exit(main())
