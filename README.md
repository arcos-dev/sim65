# Simulador 6502

[![CI](https://github.com/arcos-dev/sim65/workflows/CI/badge.svg)](https://github.com/arcos-dev/sim65/actions)
[![codecov](https://codecov.io/gh/arcos-dev/sim65/branch/master/graph/badge.svg)](https://codecov.io/gh/arcos-dev/sim65)

Um simulador completo do processador 6502 escrito em **C moderno (C23)**, com suporte a opcodes documentados e não-documentados. Utiliza GCC 14.2.0 com otimizações avançadas e ferramentas de análise modernas.

## Estrutura do Projeto

```
sim65/
├── src/           # Código fonte (.c e .h)
├── build/         # Arquivos objeto (.o)
├── asm/           # Exemplos de assembly
├── tests/         # Testes e programas de teste
├── tools/         # Ferramentas auxiliares
├── sim65          # Executável do simulador
└── Makefile       # Build system
```

## Compilação

### Comandos principais:

```bash
# Verificar suporte ao C23
make check-modern

# Compilar o projeto (C23 + otimizações nativas)
make all

# Limpar arquivos de build
make clean

# Limpeza profunda (remove diretório build)
make distclean

# Recompilar tudo
make rebuild

# Executar o simulador
make run

# Build de debug (com stack protection + símbolos debug)
make debug

# Compilar testes funcionais
make build-tests

# Executar todos os testes funcionais (75 testes)
make test

# Executar testes rápidos (com timeout)
make test-quick

# Executar teste específico
make test-file FILE=nome_do_teste.bin

# Mostrar informações do projeto
make info
```

### Script de desenvolvimento (alternativo):

```bash
# Usar o script dev.sh para comandos mais intuitivos
./dev.sh help       # Ver todos os comandos disponíveis
./dev.sh build      # Construir o projeto
./dev.sh test       # Executar testes
./dev.sh format     # Formatar código
./dev.sh coverage   # Relatório de cobertura
./dev.sh package    # Criar pacote de distribuição
```

### Requisitos:
- **GCC 14.2.0+** (com suporte ao C23) ou Clang 16+
- Make
- Sistema compatível com Unix/Linux/Windows (com Bash)

### Opcional para desenvolvimento:
- `clang-format` - Para formatação automática de código
- `cppcheck` - Para análise estática
- `gcov` - Para relatórios de cobertura de código

### Recursos C23 utilizados:
- **Padrão C23** com todas as melhorias modernas
- **20+ warnings avançados** para código mais seguro
- **Otimizações nativas** (`-march=native -mtune=native`)
- **Stack protection** e símbolos de debug aprimorados
- **Dead code elimination** e otimizações de função

## Componentes

- **main.c** - Função principal e loop do simulador
- **cpu.c/h** - Implementação do processador 6502
- **memory.c/h** - Sistema de memória
- **bus.c/h** - Barramento do sistema
- **acia.c/h** - Interface de comunicação assíncrona
- **tia.c/h** - Chip de interface de televisão (TIA)
- **clock.c/h** - Sistema de clock

## Testes

O simulador inclui uma suíte abrangente de testes funcionais:

- **Klaus Dormann Tests** - Testes funcionais e decimais básicos
- **Bird Computer Test** - Teste de compatibilidade
- **Lorenz Tests** - Testes para opcodes não-documentados
- **Visual6502 Test** - Teste do modo decimal
- **Avery Lee Tests** - Testes adicionais de precisão
- **AllSuiteA** - Suíte completa de testes

Total: **75 testes funcionais** + **384 testes de ciclos**

```bash
# Executar todos os testes (pode demorar alguns minutos)
make test

# Ver lista de testes disponíveis
ls tests/*.bin
```

## Uso

Após compilar com `make`, execute:

```bash
./sim65
```

ou use:

```bash
make run
```
