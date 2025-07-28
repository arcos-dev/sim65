#!/usr/bin/env python3

import sys
sys.path.append('.')
from programs_6502 import Programs6502

def main():
    programs = Programs6502.get_all_programs()
    print(f'Total de programas disponíveis: {len(programs)}')
    print('\nProgramas:')
    for i, program in enumerate(programs, 1):
        print(f'{i:2d}. {program["name"]} - {program["description"]}')

if __name__ == '__main__':
    main()
