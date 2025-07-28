#!/usr/bin/env python3
from programs_6502 import Programs6502

programs = Programs6502.get_all_programs()
print(f'Total de programas: {len(programs)}')
print('\nProgramas disponíveis:')
for i, p in enumerate(programs, 1):
    print(f'  {i:2d}. {p["name"]}: {p["description"]}')
    print(f'      Componentes: {", ".join(p["components"])}')
    print(f'      Endereço: 0x{p["start_address"]:04X}')
    print()
