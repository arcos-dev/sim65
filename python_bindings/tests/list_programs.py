#!/usr/bin/env python3
import sys, os
sys.path.insert(0, os.path.abspath('..'))
from programs_6502 import Programs6502

print("=== PROGRAMAS DISPONÍVEIS ===")
programs = Programs6502.get_all_programs()
for i, prog in enumerate(programs):
    print(f"{i+1}. {prog['name']} - {prog['description']}")
print(f"\nTotal: {len(programs)} programas")
