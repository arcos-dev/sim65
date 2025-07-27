#!/usr/bin/env python3
import sys, os
sys.path.insert(0, os.path.abspath('..'))
from emu65_core import Emu65Core

print("=== Quick LCD Test ===")
core = Emu65Core({'debug_mode': False})

print("Writing LCD commands...")
core.write_byte(0x6000, 0x38)  # Function Set
core.write_byte(0x6001, 0x80)  # E=1
core.write_byte(0x6001, 0x00)  # E=0

core.write_byte(0x6000, 0x0C)  # Display On
core.write_byte(0x6001, 0x80)  # E=1
core.write_byte(0x6001, 0x00)  # E=0

print("Writing 'H' character...")
core.write_byte(0x6000, 0x48)  # 'H'
core.write_byte(0x6001, 0xA0)  # RS=1, E=1
core.write_byte(0x6001, 0x20)  # RS=1, E=0

lcd_state = core.get_lcd_state()
display_bytes = bytes(lcd_state.display)
row1 = display_bytes[:16].decode('ascii', errors='replace').rstrip('\x00')
print(f'LCD Display: "{row1}"')
print(f'Success: {"H" in row1}')

core.destroy()
print("Test completed!")
