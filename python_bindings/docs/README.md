# Documentação EMU65 Python

Esta pasta contém toda a documentação relacionada ao módulo Python do EMU65.

## Estrutura da Documentação

### 📚 **EXEMPLOS.md**
Guia completo dos programas de exemplo disponíveis no EMU65.
- Lista de todos os 13 programas de exemplo
- Descrições detalhadas de cada programa
- Instruções de uso
- Endereços de I/O utilizados
- Dicas para iniciantes e avançados

### 🔧 **COMPONENTES_AUTOMATICOS.md**
Documentação da funcionalidade de componentes automáticos.
- Como funciona o carregamento automático
- Posicionamento inteligente dos componentes
- Benefícios para diferentes tipos de usuários
- Detalhes técnicos da implementação
- Guia de personalização

### 📋 **CHANGELOG_EXEMPLOS.md**
Histórico de mudanças e melhorias nos exemplos.
- Versão 2.1.0 - Adição do menu de exemplos
- Lista completa de programas adicionados
- Modificações técnicas realizadas
- Funcionalidades implementadas
- Compatibilidade e próximos passos

## Como Usar

### Para Desenvolvedores
1. Leia `CHANGELOG_EXEMPLOS.md` para entender as mudanças recentes
2. Consulte `COMPONENTES_AUTOMATICOS.md` para detalhes técnicos
3. Use `EXEMPLOS.md` como referência dos programas disponíveis

### Para Usuários
1. Comece com `EXEMPLOS.md` para entender os programas disponíveis
2. Leia `COMPONENTES_AUTOMATICOS.md` para entender como usar a interface
3. Use `CHANGELOG_EXEMPLOS.md` para verificar novas funcionalidades

### Para Educadores
1. `EXEMPLOS.md` - Lista de programas para demonstração
2. `COMPONENTES_AUTOMATICOS.md` - Benefícios educacionais
3. `CHANGELOG_EXEMPLOS.md` - Evolução do projeto

## Estrutura do Projeto

```
python_bindings/
├── docs/                    # 📁 Esta pasta
│   ├── README.md           # Este arquivo
│   ├── EXEMPLOS.md         # Guia dos exemplos
│   ├── COMPONENTES_AUTOMATICOS.md  # Componentes automáticos
│   └── CHANGELOG_EXEMPLOS.md       # Histórico de mudanças
├── panels/                 # Painéis da interface
├── widgets/                # Widgets personalizados
├── emu65_core.py          # Core do emulador
├── emu65_gui.py           # Interface gráfica
├── programs_6502.py       # Programas de exemplo
├── test_examples.py       # Teste dos exemplos
├── test_components.py     # Teste dos componentes
└── requirements.txt       # Dependências Python
```

## Contribuição

Para contribuir com a documentação:

1. **Adicionar novos exemplos**: Atualize `EXEMPLOS.md` e `CHANGELOG_EXEMPLOS.md`
2. **Nova funcionalidade**: Documente em `COMPONENTES_AUTOMATICOS.md`
3. **Correções**: Atualize o arquivo relevante e o changelog

## Versão

**Versão atual**: 2.1.0  
**Data**: 2025-01-06  
**Autor**: Anderson Costa

## Suporte

Para dúvidas sobre a documentação ou o projeto EMU65, consulte os arquivos específicos ou entre em contato com a equipe de desenvolvimento. 