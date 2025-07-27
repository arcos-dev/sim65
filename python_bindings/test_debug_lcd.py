#!/usr/bin/env python3
"""
Test with debug to see if step-based LCD processing is working
"""

import sys
import os

# Add the python_bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__)))

from emu65_core import Emu65Core
from programs_6502 import Programs6502

def test_with_debug():
    """Test LCD with debug output"""

    print("=== LCD DEBUG TEST ===")

    # Create emulator instance
    emu = Emu65Core()

    # Generate Hello World program
    programs = Programs6502()
    hello_world_info = programs.hello_world()
    hello_world_program = hello_world_info['binary']

    # Load the program at 0x8000
    emu.load_program(hello_world_program, 0x8000)

    # Reset CPU to start at our program
    emu.reset()

    print("Executing with step-by-step LCD state monitoring...")

    max_steps = 50  # Just enough to see the first few characters

    for step in range(max_steps):
        # Get LCD state before step
        lcd_before = emu.get_lcd_state()
        line1_before = lcd_before.display[:16].decode('utf-8', errors='replace').rstrip('\x00')

        # Execute one step
        cycles = emu.step()
        if cycles <= 0:
            break

        # Get LCD state after step
        lcd_after = emu.get_lcd_state()
        line1_after = lcd_after.display[:16].decode('utf-8', errors='replace').rstrip('\x00')

        # Check if LCD state changed
        if line1_before != line1_after:
            print(f"Step {step:3d}: LCD CHANGED!")
            print(f"   Before: '{line1_before}'")
            print(f"   After:  '{line1_after}'")
            print(f"   Cursor: Row {lcd_after.cursor_row}, Col {lcd_after.cursor_col}")

        # Also check VIA state for context
        portb = emu.read_byte(0x6000)
        porta = emu.read_byte(0x6001)
        rs = (porta & 0x20) != 0
        e = (porta & 0x80) != 0

        if portb != 0 or porta != 0:
            char = chr(portb) if 32 <= portb <= 126 else f"\\x{portb:02X}"
            print(f"Step {step:3d}: VIA state - PORTB=0x{portb:02X}('{char}'), PORTA=0x{porta:02X} (RS={rs}, E={e})")

    print(f"\nFinal LCD state after {max_steps} steps:")
    lcd_final = emu.get_lcd_state()
    line1_final = lcd_final.display[:16].decode('utf-8', errors='replace').rstrip('\x00')
    print(f"Line 1: '{line1_final}'")
    print(f"Cursor: Row {lcd_final.cursor_row}, Col {lcd_final.cursor_col}")

if __name__ == "__main__":
    test_with_debug()
