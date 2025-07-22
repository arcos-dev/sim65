# Emulador 6502 + TIA com Raylib

Este projeto integra o emulador 6502 completo (da pasta `src/`) com o chip Television Interface Adaptor (TIA) do Atari 2600, usando a biblioteca Raylib para renderização gráfica.

## Funcionalidades

- **Emulador 6502 Completo**: Utiliza o emulador mais robusto da pasta `src/` com:
  - Suporte a opcodes documentados e não-documentados
  - Sistema de bus com mapeamento de memória
  - Suporte a dispositivos periféricos (ACIA, TIA, VIA)
  - Emulação de clock precisa

- **TIA (Television Interface Adaptor)**:
  - Emulação básica do chip gráfico do Atari 2600
  - Framebuffer de 160x192 pixels em RGBA
  - Suporte a scanlines e ciclos de cor
  - Geração de vídeo em tempo real

- **Interface Gráfica com Raylib**:
  - Janela de 640x480 pixels
  - Renderização em tempo real do framebuffer TIA
  - Suporte a drag & drop de arquivos ROM
  - Taxa de atualização de 60 FPS

## Estrutura do Projeto

```
tia/
├── main.c          # Programa principal que integra CPU + TIA + Raylib
├── Makefile        # Build system
├── tia.exe         # Executável compilado
├── raygui.h        # Header da biblioteca RayGUI
├── raygui.dll      # Biblioteca RayGUI
└── README.md       # Este arquivo
```

## Dependências

- **GCC** com suporte ao C23
- **Raylib** (biblioteca gráfica)
- **RayGUI** (interface de usuário)

### Configuração do Raylib

O Makefile está configurado para usar o Raylib instalado em:
```
D:/Code/c/lib/raylib
```

Se o seu Raylib estiver em outro local, edite a variável `RAYLIB_PATH` no Makefile.

## Compilação

```bash
# Limpar arquivos de build
make clean

# Compilar o projeto
make

# Executar o emulador
make run
```

## Uso

### Execução Básica
```bash
# Executar sem ROM (exibe instruções na tela)
tia.exe

# Executar com uma ROM específica
tia.exe ../rom/testcart.bin
```

### Controles
- **ESC**: Sair do programa
- **Drag & Drop**: Arrastar e soltar arquivos ROM na janela para carregar

### ROMs Testadas
O projeto inclui várias ROMs de teste na pasta `../rom/`:
- `testcart.bin`: Cartucho de teste básico
- `ehbasic.rom`: EhBASIC (interpretador BASIC)
- `pitfall.a26`: Jogo Pitfall (Atari 2600)
- `echo_irq.rom`: Teste de interrupção

## Arquitetura

### Fluxo de Execução
1. **Inicialização**:
   - Criação do ACIA (comunicação serial)
   - Inicialização do TIA (chip gráfico)
   - Configuração do bus de sistema
   - Inicialização da CPU 6502

2. **Loop Principal**:
   - Execução de ~16666 ciclos de CPU por frame (1MHz @ 60fps)
   - Atualização do TIA com ciclos de cor
   - Renderização do framebuffer na tela
   - Processamento de eventos (drag & drop, teclado)

3. **Mapeamento de Memória**:
   - `0x0000-0x003F`: Registradores TIA
   - `0x0080-0x00FF`: Registradores RIOT/ACIA
   - `0xF000-0xFFFF`: Área padrão para ROMs

### Integração CPU-TIA
- A CPU escreve nos registradores TIA através do bus de sistema
- O TIA processa os dados em ciclos de cor (3 ciclos TIA por ciclo CPU)
- O framebuffer é atualizado pixel a pixel durante a renderização
- A sincronização é mantida via VSYNC e VBLANK

## Melhorias Implementadas

### Vs. Versão Anterior
- **Emulador mais robusto**: Migrado do código básico para o emulador completo da pasta `src/`
- **Melhor integração**: Uso correto das APIs do TIA (`tia_init`, `tia_cycle`, `tia_get_framebuffer`)
- **Mapeamento correto**: Dimensões do framebuffer usando `TIA_SCREEN_WIDTH` e `TIA_SCREEN_HEIGHT`
- **Standards modernos**: Compilação com C23 e flags de otimização

### Funcionalidades Adicionadas
- Suporte a VIA 6522 (Versatile Interface Adapter)
- Melhor tratamento de erros e logging
- Processamento de ACIA para comunicação serial
- Sistema de clock configurável

## Desenvolvimento Futuro

### Melhorias Planejadas
- [ ] Implementar collision detection completa no TIA
- [ ] Adicionar suporte a som (canais de áudio do TIA)
- [ ] Melhorar a emulação de sprites e playfield
- [ ] Implementar score mode e reflection
- [ ] Adicionar debugger gráfico
- [ ] Suporte a paletas de cores PAL/NTSC
- [ ] Implementar horizontal motion (HMOVE)

### Otimizações
- [ ] Rendering em batch para melhor performance
- [ ] Multithreading para CPU e TIA separados
- [ ] Cache de framebuffer para evitar redesenhos
- [ ] Profiling e otimização de hot paths

## Troubleshooting

### Problemas Comuns

**Erro de compilação "raylib.h not found"**:
- Verifique se o Raylib está instalado no caminho especificado
- Ajuste `RAYLIB_PATH` no Makefile

**ROM não carrega**:
- Verifique se o arquivo existe e tem permissões de leitura
- ROMs devem ser arquivos binários (.bin, .rom, .a26)
- O endereço de carregamento padrão é 0xF000

**Tela preta/sem vídeo**:
- O TIA precisa ser programado pela CPU para gerar imagem
- Nem todas as ROMs produzem saída visual imediata
- Tente com `testcart.bin` para verificar funcionamento básico

**Performance baixa**:
- Ajuste o número de ciclos por frame no main.c
- Verifique se está compilando com `-O2` ou `-O3`
- GPU antiga pode ter dificuldades com RGBA textures

## Contribuição

Para contribuir:
1. Teste o emulador com diferentes ROMs
2. Reporte bugs ou comportamentos incorretos
3. Implemente funcionalidades do TIA em falta
4. Otimize performance crítica
5. Melhore documentação e exemplos

## Licença

Este projeto segue a mesma licença do projeto principal sim65 (2-clause BSD license).

---
**Autor**: Anderson Costa
**Data**: 2025-01-28 (atualizado)
**Versão**: 2.0 (integrado com emulador completo)
