# Exemplos de Programas 6502 - EMU65

Este documento descreve os programas de exemplo disponíveis no EMU65 e como utilizá-los.

## Como Acessar os Exemplos

1. Abra a aplicação EMU65 GUI
2. Vá para o menu **Ajuda** > **Exemplos**
3. Selecione o programa desejado da lista

## Programas Disponíveis

### 1. Hello World
- **Descrição**: Escreve "Hello World!" no LCD
- **Tamanho**: 79 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Demonstra escrita básica no display LCD

### 2. Contador
- **Descrição**: Conta de 0 a 99 no LCD
- **Tamanho**: 49 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Demonstra loops e incremento de valores

### 3. Matemática
- **Descrição**: Demonstra ADC e SBC
- **Tamanho**: 29 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Mostra operações aritméticas básicas

### 4. Interrupções
- **Descrição**: Demonstra IRQ e NMI
- **Tamanho**: 36 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Exibe como as interrupções funcionam

### 5. Texto Rolante
- **Descrição**: Rola texto da direita para esquerda
- **Tamanho**: 211 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Demonstra manipulação de strings e arrays

### 6. LED Piscante
- **Descrição**: Pisca LED conectado ao endereço 0x6000
- **Tamanho**: 13 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Controle básico de I/O

### 7. Teclado
- **Descrição**: Lê teclado e exibe no LCD
- **Tamanho**: 9 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Demonstra entrada de dados

### 8. Teste de Memória
- **Descrição**: Testa escrita e leitura de memória
- **Tamanho**: 21 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Verifica operações de memória

### 9. Stack
- **Descrição**: Demonstra operações de stack (PHA, PLA)
- **Tamanho**: 12 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Mostra uso da pilha

### 10. Flags
- **Descrição**: Demonstra flags de status (C, Z, N)
- **Tamanho**: 24 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Exibe como os flags são afetados

### 11. Demo Complexa
- **Descrição**: Programa complexo com loops e condicionais
- **Tamanho**: 60 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Combina várias técnicas de programação

### 12. Som
- **Descrição**: Gera diferentes frequências de som
- **Tamanho**: 13 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Controle de áudio

### 13. Gráficos
- **Descrição**: Desenha padrões no display gráfico
- **Tamanho**: 20 bytes
- **Endereço**: 0x8000
- **Funcionalidade**: Demonstra saída gráfica

## Como Usar

1. **Carregar um Programa**:
   - Selecione o programa no menu Ajuda > Exemplos
   - O programa será automaticamente carregado na memória
   - Uma mensagem de confirmação será exibida

2. **Executar o Programa**:
   - Use os controles de emulação (Step, Run, Stop)
   - Observe o comportamento no LCD e painéis de status
   - Monitore os logs para informações detalhadas

3. **Analisar o Código**:
   - Cada programa demonstra diferentes aspectos do 6502
   - Use o modo Step para executar instrução por instrução
   - Observe como os registradores e flags mudam

## Endereços de I/O Utilizados

- **0x6000-0x600F**: Display LCD 16x2
- **0x7000**: Teclado (entrada)
- **0x5000**: Controle de som
- **0x4000-0x4003**: Display gráfico

## Dicas de Uso

- **Para iniciantes**: Comece com "Hello World" e "Contador"
- **Para aprender matemática**: Use "Matemática" e "Flags"
- **Para entender controle**: Experimente "LED Piscante" e "Som"
- **Para programação avançada**: Analise "Demo Complexa"

## Personalização

Você pode criar seus próprios programas adicionando novos métodos à classe `Programs6502` no arquivo `programs_6502.py`. Cada programa deve retornar um dicionário com:

```python
{
    'name': 'Nome do Programa',
    'description': 'Descrição detalhada',
    'binary': bytes.fromhex('código_hex_aqui'),
    'start_address': 0x8000
}
```

## Suporte

Para dúvidas sobre os exemplos ou para contribuir com novos programas, consulte a documentação do projeto ou entre em contato com a equipe de desenvolvimento. 