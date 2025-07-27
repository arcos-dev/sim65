#!/usr/bin/env python3
"""
Test LCD state after Hello World execution to verify the fix
"""

import sys
import os

# Add the python_bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__)))

from emu65_core import Emu65Core
from programs_6502 import Programs6502

def test_lcd_final():
    """Final test to verify LCD displays HELLO WORLD correctly"""

    print("=== FINAL LCD TEST ===")
    print("Testing if VIA fix resolves the LCD display issue...\n")

    # Create emulator instance
    emu = Emu65Core()

    # Generate Hello World program
    programs = Programs6502()
    hello_world_info = programs.hello_world()
    hello_world_program = hello_world_info['binary']

    print(f"Program loaded: {len(hello_world_program)} bytes")

    # Load the program at 0x8000
    emu.load_program(hello_world_program, 0x8000)

    # Reset CPU to start at our program
    emu.reset()

    print("Executing Hello World program...")

    # Run the program for enough steps to complete one cycle
    max_steps = 200

    for step in range(max_steps):
        cycles = emu.step()
        if cycles <= 0:
            print(f"CPU halted at step {step}")
            break

        # Check if we've returned to the start (program completed one cycle)
        cpu_state = emu.get_cpu_state()
        if step > 50 and cpu_state.pc == 0x8000:
            print(f"Program completed one cycle at step {step}")
            break

    print("\nChecking LCD state...")

    # Get the final LCD state
    lcd_state = emu.get_lcd_state()

    # The LCD structure has a display field containing both lines
    display_data = lcd_state.display

    # Extract the lines (first 16 chars = line 1, next 16 chars = line 2, assuming null terminators)
    line1 = display_data[:16].decode('utf-8', errors='replace').rstrip('\x00')
    line2 = display_data[17:33].decode('utf-8', errors='replace').rstrip('\x00')

    print(f"LCD Line 1: '{line1}'")
    print(f"LCD Line 2: '{line2}'")
    print(f"LCD Cursor: Row {lcd_state.cursor_row}, Col {lcd_state.cursor_col}")
    print(f"LCD Status: Display={lcd_state.display_on}, Cursor={lcd_state.cursor_on}, Blink={lcd_state.blink_on}")

    # Check if "HELLO WORLD!" appears in the LCD
    expected_text = "HELLO WORLD!"

    if expected_text in line1 or expected_text in line2:
        print(f"\n✅ SUCCESS! LCD contains '{expected_text}'")
        return True
    else:
        print(f"\n❌ FAILED! LCD does not contain '{expected_text}'")
        print(f"   Line 1: '{line1}'")
        print(f"   Line 2: '{line2}'")

        # Show hex dump of display data for debugging
        print(f"\nHex dump of display data:")
        for i, byte in enumerate(display_data):
            if i % 16 == 0:
                print(f"\n  {i:02X}: ", end="")
            print(f"{byte:02X} ", end="")
        print()

        # Also check raw VIA registers for debugging
        print("\nDirect VIA register check:")
        portb_data = emu.read_byte(0x6000)
        porta_control = emu.read_byte(0x6001)
        print(f"   PORTB (data): 0x{portb_data:02X}")
        print(f"   PORTA (control): 0x{porta_control:02X}")

        return False

if __name__ == "__main__":
    success = test_lcd_final()
    if success:
        print("\n🎉 VIA FIX SUCCESSFUL - LCD IS WORKING!")
    else:
        print("\n😞 VIA fix did not fully resolve the issue")
        print("Additional debugging may be needed")

    print("\nNote: The GUI should now be displaying 'HELLO WORLD!' correctly!")
    print("Check the GUI window to verify the fix.")
