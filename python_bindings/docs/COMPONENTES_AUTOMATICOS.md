# Componentes Automáticos - EMU65

## Visão Geral

A partir da versão 2.1.0, o EMU65 agora adiciona automaticamente os componentes necessários quando um programa de exemplo é carregado. Isso simplifica significativamente o processo de configuração e permite que os usuários se concentrem na execução e análise dos programas.

## Como Funciona

### 1. Carregamento Inteligente
Quando você seleciona um programa do menu **Ajuda > Exemplos**:

1. **Limpeza Automática**: A área de trabalho é limpa automaticamente
2. **Análise de Componentes**: O sistema identifica quais componentes são necessários
3. **Adição Automática**: Os componentes são posicionados estrategicamente
4. **Carregamento do Programa**: O código 6502 é carregado na memória
5. **Reset do Sistema**: O emulador é resetado para o estado inicial

### 2. Posicionamento Inteligente
Os componentes são posicionados em locais predefinidos para facilitar a visualização:

- **6502 CPU**: (50, 50) - Canto superior esquerdo
- **LCD 16x2**: (300, 50) - Lado direito, parte superior
- **LED**: (300, 150) - Lado direito, parte média
- **Botão**: (300, 200) - Lado direito, parte inferior
- **RAM**: (150, 50) - Centro superior
- **ROM**: (150, 150) - Centro médio
- **Resistor**: (400, 50) - Extremo direito, superior
- **Capacitor**: (400, 150) - Extremo direito, médio

## Programas e Componentes

### Programas com LCD (8 programas)
- **Hello World**: 6502 CPU + LCD 16x2
- **Contador**: 6502 CPU + LCD 16x2
- **Matemática**: 6502 CPU + LCD 16x2
- **Interrupções**: 6502 CPU + LCD 16x2
- **Texto Rolante**: 6502 CPU + LCD 16x2
- **Teclado**: 6502 CPU + LCD 16x2 + Botão
- **Teste de Memória**: 6502 CPU + RAM + LCD 16x2
- **Stack**: 6502 CPU + LCD 16x2
- **Flags**: 6502 CPU + LCD 16x2
- **Demo Complexa**: 6502 CPU + LCD 16x2

### Programas Especializados (3 programas)
- **LED Piscante**: 6502 CPU + LED
- **Som**: 6502 CPU + Resistor + Capacitor
- **Gráficos**: 6502 CPU + ROM

## Benefícios

### Para Iniciantes
- **Configuração Zero**: Não precisa saber quais componentes usar
- **Aprendizado Visual**: Vê imediatamente o que cada programa precisa
- **Foco no Código**: Concentra-se na execução, não na configuração

### Para Educadores
- **Demonstração Rápida**: Carrega exemplos instantaneamente
- **Consistência**: Todos os alunos veem a mesma configuração
- **Tempo Economizado**: Não perde tempo configurando manualmente

### Para Desenvolvedores
- **Teste Rápido**: Verifica se o programa funciona com os componentes corretos
- **Debugging**: Identifica problemas de hardware/software rapidamente
- **Prototipagem**: Testa diferentes configurações facilmente

## Fluxo de Trabalho

### Antes (Manual)
1. Adicionar 6502 CPU
2. Adicionar LCD 16x2
3. Posicionar componentes
4. Carregar programa
5. Resetar sistema
6. Executar

### Agora (Automático)
1. Menu Ajuda > Exemplos
2. Selecionar programa
3. Executar

**Economia de tempo: ~80%**

## Detalhes Técnicos

### Limpeza Automática
```python
self.work_area.clear_components()
```
Remove todos os componentes existentes antes de adicionar os novos.

### Adição Inteligente
```python
def add_required_components(self, components):
    # Posiciona cada componente automaticamente
    for component_name in components:
        # Cria e posiciona o componente
```

### Logs Detalhados
O sistema registra cada ação:
- "Componente adicionado: 6502 CPU"
- "Componente adicionado: LCD 16x2"
- "Programa carregado: Hello World"

## Personalização

### Modificar Posições
Para alterar as posições dos componentes, edite o dicionário `positions` em `add_required_components()`:

```python
positions = {
    '6502 CPU': QPoint(50, 50),
    'LCD 16x2': QPoint(300, 50),
    # ... outras posições
}
```

### Adicionar Novos Componentes
Para novos tipos de componentes, adicione a lógica no método `add_required_components()`:

```python
elif component_name == "Novo Componente":
    novo = NovoComponenteWidget()
    pos = positions.get(component_name, QPoint(200, 200))
    self.work_area.add_component(novo, pos)
```

## Compatibilidade

### Componentes Suportados
- ✅ 6502 CPU (com pinos completos)
- ✅ LCD 16x2 (display funcional)
- ✅ LED (componente genérico)
- ✅ Botão (componente genérico)
- ✅ RAM (componente genérico)
- ✅ ROM (componente genérico)
- ✅ Resistor (componente genérico)
- ✅ Capacitor (componente genérico)

### Funcionalidades
- ✅ Posicionamento automático
- ✅ Snap-to-grid
- ✅ Arrastar e soltar
- ✅ Rotação de componentes
- ✅ Logs detalhados
- ✅ Mensagens de confirmação

## Próximos Passos

### Melhorias Planejadas
1. **Conexões Automáticas**: Conectar pinos automaticamente
2. **Configurações Salvas**: Salvar layouts personalizados
3. **Templates**: Configurações pré-definidas para diferentes projetos
4. **Validação**: Verificar se os componentes estão conectados corretamente

### Novos Componentes
1. **Display Gráfico**: Para programas de gráficos
2. **Speaker**: Para programas de som
3. **Teclado**: Para entrada de dados
4. **Sensor**: Para programas de sensores

## Conclusão

A funcionalidade de componentes automáticos transforma o EMU65 de uma ferramenta de configuração manual em uma plataforma educacional intuitiva e eficiente. Agora os usuários podem focar no que realmente importa: aprender programação 6502 através de exemplos práticos e interativos. 