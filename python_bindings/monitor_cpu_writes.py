#!/usr/bin/env python3
"""
Monitor actual CPU writes to VIA PORTB (0x6000) during execution
"""

import sys
import os

# Add the python_bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__)))

from emu65_core import Emu65Core
from programs_6502 import Programs6502

def monitor_cpu_writes():
    """Monitor CPU writes to specific addresses"""

    # Create emulator instance
    emu = Emu65Core()

    # Generate Hello World program
    programs = Programs6502()
    hello_world_info = programs.hello_world()
    hello_world_program = hello_world_info['binary']

    print("Looking for CPU writes to 0x6000 (PORTB) in binary...")

    # Find all instances of "STA $6000" (8D 00 60) in the binary
    sta_6000_pattern = [0x8D, 0x00, 0x60]

    for i in range(len(hello_world_program) - 2):
        if (hello_world_program[i] == sta_6000_pattern[0] and
            hello_world_program[i+1] == sta_6000_pattern[1] and
            hello_world_program[i+2] == sta_6000_pattern[2]):

            # Look for the preceding LDA instruction
            if i >= 2 and hello_world_program[i-2] == 0xA9:  # LDA immediate
                data_value = hello_world_program[i-1]
                print(f"Found LDA #${data_value:02X} ; STA $6000 at offset {i-2:04X}")

                # Check if this is a printable character
                if 32 <= data_value <= 126:
                    char = chr(data_value)
                    print(f"  Data: 0x{data_value:02X} = '{char}'")
                else:
                    print(f"  Data: 0x{data_value:02X} (non-printable)")

    print("\nNow running emulator to see what actually gets written...")

    # Load the program at 0x8000
    emu.load_program(hello_world_program, 0x8000)

    # Reset CPU to start at our program
    emu.reset()

    max_steps = 1000
    writes_to_portb = []

    for step in range(max_steps):
        # Get state before step
        cpu_state = emu.get_cpu_state()
        pc_before = cpu_state.pc

        # Check if this is a write to PORTB by looking at the instruction
        if (pc_before < 0xFFFF - 2):  # Ensure we can read the instruction
            # Read the instruction at PC
            instruction = emu.read_byte(pc_before)
            if instruction == 0x8D:  # STA absolute
                addr_lo = emu.read_byte(pc_before + 1)
                addr_hi = emu.read_byte(pc_before + 2)
                address = addr_lo | (addr_hi << 8)

                if address == 0x6000:  # Writing to PORTB
                    # Get the accumulator value (what will be written)
                    acc_value = cpu_state.a

                    write_info = {
                        'step': step,
                        'pc': pc_before,
                        'address': address,
                        'value_to_write': acc_value
                    }
                    writes_to_portb.append(write_info)

                    char = chr(acc_value) if 32 <= acc_value <= 126 else f"\\x{acc_value:02X}"
                    print(f"Step {step:3d}: About to write 0x{acc_value:02X} ('{char}') to PORTB from PC {pc_before:04X}")

        # Execute one step
        cycles = emu.step()
        if cycles <= 0:
            break

        # Get state after step
        cpu_state = emu.get_cpu_state()
        pc_after = cpu_state.pc

        # Stop if we've returned to the start (program loop completed)
        if step > 0 and pc_after == 0x8000:
            print(f"\nProgram returned to start at step {step}")
            break

    print(f"\nTotal writes to PORTB detected: {len(writes_to_portb)}")

    if writes_to_portb:
        print("\nData values written to PORTB:")
        expected_chars = "HELLO WORLD!"
        char_index = 0

        for i, write in enumerate(writes_to_portb):
            value = write['value_to_write']
            char = chr(value) if 32 <= value <= 126 else f"\\x{value:02X}"

            print(f"  Write {i+1:2d}: 0x{value:02X} = '{char}' at PC {write['pc']:04X}")

            # Compare with expected character sequence
            if char_index < len(expected_chars):
                expected_char = expected_chars[char_index]
                expected_ascii = ord(expected_char)

                if value == expected_ascii:
                    print(f"           ✓ Matches expected '{expected_char}'")
                    char_index += 1
                else:
                    print(f"           ✗ Expected '{expected_char}' (0x{expected_ascii:02X})")
                    # Don't increment char_index if it doesn't match

if __name__ == "__main__":
    monitor_cpu_writes()
