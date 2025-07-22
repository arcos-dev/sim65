# Changelog - Menu de Exemplos EMU65

## Versão 2.1.0 - 2025-01-06

### Adicionado
- **Menu de Exemplos**: Novo submenu no menu Ajuda com programas de exemplo
- **13 Programas de Exemplo**: Coleção completa de programas 6502 para demonstração
- **Componentes Automáticos**: Adição automática de componentes necessários para cada exemplo
- **Função load_program**: Método no Emu65Core para carregar programas na memória
- **Função load_example_program**: Handler para carregar exemplos na interface GUI
- **Função add_required_components**: Adiciona automaticamente os componentes necessários
- **Documentação**: Guia completo de uso dos exemplos e componentes automáticos

### Programas Adicionados

#### Programas Originais (5)
1. **Hello World** - Escreve "Hello World!" no LCD
2. **Contador** - Conta de 0 a 99 no LCD
3. **Matemática** - Demonstra ADC e SBC
4. **Interrupções** - Demonstra IRQ e NMI
5. **Texto Rolante** - Rola texto da direita para esquerda

#### Novos Programas (8)
6. **LED Piscante** - Pisca LED conectado ao endereço 0x6000
7. **Teclado** - Lê teclado e exibe no LCD
8. **Teste de Memória** - Testa escrita e leitura de memória
9. **Stack** - Demonstra operações de stack (PHA, PLA)
10. **Flags** - Demonstra flags de status (C, Z, N)
11. **Demo Complexa** - Programa complexo com loops e condicionais
12. **Som** - Gera diferentes frequências de som
13. **Gráficos** - Desenha padrões no display gráfico

### Modificações Técnicas

#### emu65_gui.py
- Adicionado import de `Programs6502`
- Modificada função `create_menu_bar()` para incluir submenu Exemplos
- Adicionada função `load_example_program()` para carregar programas
- Adicionada função `add_required_components()` para componentes automáticos
- Integração com sistema de logs e status
- Limpeza automática da área de trabalho

#### emu65_core.py
- Adicionado método `load_program()` para carregar dados binários na memória
- Suporte para biblioteca C (quando disponível) e modo simulação

#### programs_6502.py
- Expandida de 5 para 13 programas de exemplo
- Adicionados programas demonstrando diferentes aspectos do 6502
- Adicionado campo `components` para cada programa
- Melhorada organização e documentação dos códigos

### Funcionalidades

#### Interface do Usuário
- Menu Ajuda > Exemplos com lista completa de programas
- Tooltips com descrições detalhadas
- Mensagens de confirmação ao carregar programas
- Integração com sistema de logs

#### Carregamento de Programas
- Carregamento automático na memória
- Reset automático do sistema
- Atualização de status e logs
- Tratamento de erros

#### Endereços de I/O Utilizados
- **0x6000-0x600F**: Display LCD 16x2
- **0x7000**: Teclado (entrada)
- **0x5000**: Controle de som
- **0x4000-0x4003**: Display gráfico

### Arquivos Criados/Modificados

#### Criados
- `EXEMPLOS.md` - Documentação completa dos exemplos
- `test_examples.py` - Script de teste dos programas
- `test_components.py` - Script de teste dos componentes
- `COMPONENTES_AUTOMATICOS.md` - Documentação dos componentes automáticos
- `CHANGELOG_EXEMPLOS.md` - Este arquivo

#### Modificados
- `emu65_gui.py` - Adicionado menu e funcionalidades
- `emu65_core.py` - Adicionado método load_program
- `programs_6502.py` - Expandido com novos exemplos

### Como Usar

1. **Acessar Exemplos**: Menu Ajuda > Exemplos
2. **Carregar Programa**: Clique no programa desejado
3. **Componentes Automáticos**: Os componentes necessários são adicionados automaticamente
4. **Executar**: Use controles Step/Run/Stop
5. **Observar**: Monitore LCD, logs e painéis de status

### Fluxo Automático

1. **Limpeza**: Área de trabalho é limpa automaticamente
2. **Análise**: Sistema identifica componentes necessários
3. **Posicionamento**: Componentes são posicionados estrategicamente
4. **Carregamento**: Programa 6502 é carregado na memória
5. **Reset**: Sistema é resetado para estado inicial

### Compatibilidade
- Funciona com biblioteca C (quando compilada)
- Modo simulação para desenvolvimento
- Compatível com todos os componentes da interface

### Próximos Passos
- Adicionar mais exemplos específicos
- Implementar debugger visual
- Adicionar exemplos de jogos simples
- Criar tutoriais interativos 