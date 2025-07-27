#!/usr/bin/env python3
"""
Detailed analysis of VIA control signals during LCD writes
"""

import sys
import os

# Add the python_bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__)))

from emu65_core import Emu65Core
from programs_6502 import Programs6502

def analyze_via_control():
    """Analyze VIA control signals in detail"""

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

    print("Detailed VIA control signal analysis...")
    print("Focusing on data writes only...\n")

    max_steps = 1000
    data_writes = []

    for step in range(max_steps):
        # Get state before step
        cpu_state = emu.get_cpu_state()
        pc_before = cpu_state.pc

        # Execute one step
        cycles = emu.step()
        if cycles <= 0:
            break

        # Get state after step
        cpu_state = emu.get_cpu_state()
        pc_after = cpu_state.pc

        # Read VIA registers
        portb_data = emu.read_byte(0x6000)  # VIA PORTB (LCD data)
        porta_control = emu.read_byte(0x6001)  # VIA PORTA (LCD control)

        # Check if this is a data write (RS=1, E=1, and data != 0)
        rs = (porta_control & 0x20) != 0  # RS bit (PA5)
        rw = (porta_control & 0x40) != 0  # RW bit (PA6)
        e = (porta_control & 0x80) != 0   # Enable bit (PA7)

        if rs and e and portb_data != 0 and not rw:  # Data write with enable high
            data_write = {
                'step': step,
                'pc_before': pc_before,
                'pc_after': pc_after,
                'portb_data': portb_data,
                'porta_control': porta_control,
                'rs': rs,
                'rw': rw,
                'e': e
            }
            data_writes.append(data_write)

            # Analyze the data byte
            char = chr(portb_data) if 32 <= portb_data <= 126 else f"\\x{portb_data:02X}"

            print(f"Data Write #{len(data_writes):2d} at step {step:3d}:")
            print(f"  PC: {pc_before:04X} -> {pc_after:04X}")
            print(f"  Data: 0x{portb_data:02X} = '{char}'")
            print(f"  Control: 0x{porta_control:02X} (RS={rs}, RW={rw}, E={e})")

            # Check if this data matches expected ASCII
            expected_chars = "HELLO WORLD!"
            if len(data_writes) <= len(expected_chars):
                expected_char = expected_chars[len(data_writes)-1]
                expected_ascii = ord(expected_char)

                print(f"  Expected: '{expected_char}' (0x{expected_ascii:02X})")
                if portb_data == expected_ascii:
                    print(f"  ✓ CORRECT")
                else:
                    diff = portb_data - expected_ascii
                    print(f"  ✗ WRONG (diff: {diff}, bit pattern: {portb_data:08b} vs {expected_ascii:08b})")

                    # Check specific bit differences
                    if (expected_ascii & 0x40) and not (portb_data & 0x40):
                        print(f"    * Bit 6 (0x40) is missing!")
                    if (expected_ascii & 0x20) and not (portb_data & 0x20):
                        print(f"    * Bit 5 (0x20) is missing!")

            print()

        # Stop if we've returned to the start (program loop completed)
        if step > 0 and pc_after == 0x8000:
            print(f"Program returned to start at step {step}")
            break

    print(f"\nTotal data writes captured: {len(data_writes)}")

    # Summary analysis
    if data_writes:
        print("\nSummary of data corruption:")
        expected_chars = "HELLO WORLD!"

        for i, data_write in enumerate(data_writes):
            if i < len(expected_chars):
                expected_char = expected_chars[i]
                expected_ascii = ord(expected_char)
                actual_ascii = data_write['portb_data']

                if actual_ascii != expected_ascii:
                    diff = actual_ascii - expected_ascii
                    print(f"  Char {i+1:2d}: got 0x{actual_ascii:02X}, expected 0x{expected_ascii:02X} ('{expected_char}'), diff={diff}")

                    # Bit analysis
                    missing_bits = expected_ascii & ~actual_ascii
                    extra_bits = actual_ascii & ~expected_ascii

                    if missing_bits:
                        print(f"           Missing bits: 0x{missing_bits:02X}")
                    if extra_bits:
                        print(f"           Extra bits: 0x{extra_bits:02X}")

if __name__ == "__main__":
    analyze_via_control()
