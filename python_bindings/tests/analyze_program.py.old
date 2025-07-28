#!/usr/bin/env python3
"""
Decodificador de programa 6502
==============================
"""
import sys, os
sys.path.insert(0, os.path.abspath('..'))
from programs_6502 import Programs6502

hello_world = Programs6502.hello_world()
binary = hello_world['binary']

print("=== ANÁLISE DO PROGRAMA HELLO WORLD ===")
print(f"Tamanho: {len(binary)} bytes")
print(f"Hex completo: {binary.hex()}")

print("\n=== DECODIFICAÇÃO (manual) ===")
# Primeiros bytes da inicialização LCD:
init_expected = "a9ff8d0260a9e08d0360a9388d0060a9808d0160a9008d0160a90c8d0060a9808d0160a9008d0160"
if binary.hex().startswith(init_expected):
    print("✅ Sequência de inicialização LCD parece correta")
    print("   A9 FF 8D 02 60 = LDA #$FF, STA $6002 (DDRB = output)")
    print("   A9 E0 8D 03 60 = LDA #$E0, STA $6003 (DDRA = output)")
    print("   A9 38 8D 00 60 = LDA #$38, STA $6000 (Function Set)")
    print("   A9 80 8D 01 60 = LDA #$80, STA $6001 (E=1)")
    print("   A9 00 8D 01 60 = LDA #$00, STA $6001 (E=0)")
    print("   A9 0C 8D 00 60 = LDA #$0C, STA $6000 (Display On)")
    print("   A9 80 8D 01 60 = LDA #$80, STA $6001 (E=1)")
    print("   A9 00 8D 01 60 = LDA #$00, STA $6001 (E=0)")
else:
    print(f"❌ Sequência de inicialização NÃO está correta")
    print(f"   Esperado: {init_expected}")
    print(f"   Obtido:   {binary.hex()[:len(init_expected)]}")

# Procurar por escritas de caracteres ('H' = 0x48)
h_char_expected = "a9488d0060a9a08d0160a9208d0160"  # LDA #$48, STA $6000, LDA #$A0, STA $6001, LDA #$20, STA $6001
if h_char_expected in binary.hex():
    pos = binary.hex().find(h_char_expected)
    print(f"✅ Encontrado código para escrever 'H' na posição {pos//2}")
    print("   A9 48 8D 00 60 = LDA #$48, STA $6000 ('H')")
    print("   A9 A0 8D 01 60 = LDA #$A0, STA $6001 (RS=1, E=1)")
    print("   A9 20 8D 01 60 = LDA #$20, STA $6001 (RS=1, E=0)")
else:
    print(f"❌ Código para escrever 'H' NÃO encontrado")
    print(f"   Procurando: {h_char_expected}")

# Verificar loop final
loop_expected = "4c0080"  # JMP $8000
if binary.hex().endswith(loop_expected):
    print("✅ Loop final encontrado: JMP $8000")
else:
    print(f"❌ Loop final NÃO encontrado")
    print(f"   Esperado no final: {loop_expected}")
    print(f"   Últimos 6 bytes:   {binary.hex()[-6:]}")
