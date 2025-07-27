#!/usr/bin/env python3
"""
Monitor LCD Enable signal transitions to debug LCD processing
"""

import sys
import os

# Add the python_bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__)))

from emu65_core import Emu65Core
from programs_6502 import Programs6502

def monitor_enable_transitions():
    """Monitor Enable signal transitions for LCD processing"""

    print("=== LCD ENABLE SIGNAL MONITORING ===")
    print("Tracking E signal transitions to debug LCD processing...\n")

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

    print("Monitoring Enable signal transitions...")

    max_steps = 200
    prev_e = False
    prev_portb = 0
    prev_porta = 0
    lcd_operations = []

    for step in range(max_steps):
        # Execute one step
        cycles = emu.step()
        if cycles <= 0:
            break

        # Get current VIA state
        portb_data = emu.read_byte(0x6000)
        porta_control = emu.read_byte(0x6001)

        # Decode control bits
        rs = (porta_control & 0x20) != 0
        rw = (porta_control & 0x40) != 0
        e = (porta_control & 0x80) != 0

        # Check for Enable signal transitions
        e_rising_edge = not prev_e and e
        e_falling_edge = prev_e and not e

        # Track any Enable transitions
        if e_rising_edge or e_falling_edge:
            transition_type = "RISING" if e_rising_edge else "FALLING"

            print(f"Step {step:3d}: E {transition_type} edge")
            print(f"   PORTB: 0x{portb_data:02X} = '{chr(portb_data) if 32 <= portb_data <= 126 else '?'}'")
            print(f"   PORTA: 0x{porta_control:02X} (RS={rs}, RW={rw}, E={e})")

            # LCD processing should happen on falling edge
            if e_falling_edge and not rw:
                operation_type = "DATA" if rs else "COMMAND"
                lcd_operations.append({
                    'step': step,
                    'type': operation_type,
                    'value': portb_data,
                    'char': chr(portb_data) if 32 <= portb_data <= 126 else None
                })

                print(f"   -> LCD {operation_type}: 0x{portb_data:02X}")
                if operation_type == "DATA" and 32 <= portb_data <= 126:
                    print(f"   -> CHARACTER: '{chr(portb_data)}'")

            print()

        # Update previous state
        prev_e = e
        prev_portb = portb_data
        prev_porta = porta_control

        # Get CPU state to check if we've completed a cycle
        cpu_state = emu.get_cpu_state()
        if step > 50 and cpu_state.pc == 0x8000:
            print(f"Program returned to start at step {step}")
            break

    print(f"\nTotal LCD operations detected: {len(lcd_operations)}")

    # Analyze the operations
    if lcd_operations:
        print("\nLCD Operations Summary:")
        commands = [op for op in lcd_operations if op['type'] == 'COMMAND']
        data_writes = [op for op in lcd_operations if op['type'] == 'DATA']

        print(f"Commands: {len(commands)}")
        for cmd in commands:
            print(f"  0x{cmd['value']:02X} at step {cmd['step']}")

        print(f"\nData writes: {len(data_writes)}")
        characters = ""
        for data in data_writes:
            char = data['char'] if data['char'] else f"\\x{data['value']:02X}"
            characters += char if data['char'] else "?"
            print(f"  '{char}' (0x{data['value']:02X}) at step {data['step']}")

        print(f"\nCombined string: '{characters}'")

        # Check if we got the expected result
        if "HELLO WORLD!" in characters:
            print("✅ SUCCESS! LCD operations contain 'HELLO WORLD!'")
            return True
        else:
            print("❌ 'HELLO WORLD!' not found in LCD operations")

    # Check final LCD state
    print("\nChecking final LCD state...")
    lcd_state = emu.get_lcd_state()

    display_data = lcd_state.display
    line1 = display_data[:16].decode('utf-8', errors='replace').rstrip('\x00')
    line2 = display_data[17:33].decode('utf-8', errors='replace').rstrip('\x00')

    print(f"LCD Line 1: '{line1}'")
    print(f"LCD Line 2: '{line2}'")

    return "HELLO WORLD!" in line1 or "HELLO WORLD!" in line2

if __name__ == "__main__":
    success = monitor_enable_transitions()
    if success:
        print("\n🎉 LCD PROCESSING IS WORKING!")
    else:
        print("\n😞 LCD processing needs more debugging")
