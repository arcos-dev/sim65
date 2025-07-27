#!/usr/bin/env python3
"""
Test VIA monitoring during Hello World execution
This test specifically monitors VIA registers instead of generic bus state
"""

import sys
import os

# Add the python_bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__)))

from emu65_core import Emu65Core
from programs_6502 import Programs6502

def test_via_monitoring():
    """Test VIA register monitoring during Hello World execution"""

    # Create emulator instance
    emu = Emu65Core()

    # Generate Hello World program
    programs = Programs6502()
    hello_world_info = programs.hello_world()
    hello_world_program = hello_world_info['binary']  # Extract the binary data

    print(f"Generated Hello World program: {len(hello_world_program)} bytes")
    if len(hello_world_program) > 0:
        first_bytes = hello_world_program[:20]
        print(f"First 20 bytes: {' '.join(f'{b:02X}' for b in first_bytes)}")    # Load the program at 0x8000
    emu.load_program(hello_world_program, 0x8000)

    # Reset CPU to start at our program
    emu.reset()

    print("Starting VIA monitoring...")

    # Monitor VIA operations
    via_operations = []
    max_steps = 1000  # Prevent infinite loops

    for step in range(max_steps):
        # Get CPU state before step
        cpu_state = emu.get_cpu_state()
        pc_before = cpu_state.pc

        # Execute one step
        cycles = emu.step()
        if cycles <= 0:
            print(f"Step {step}: CPU halted or error")
            break

        # Get CPU state after step
        cpu_state = emu.get_cpu_state()
        pc_after = cpu_state.pc

        # Read VIA registers directly to see if they changed
        portb_data = emu.read_byte(0x6000)  # VIA PORTB (LCD data)
        porta_control = emu.read_byte(0x6001)  # VIA PORTA (LCD control)

        # Check if this looks like LCD operations
        if porta_control != 0 or portb_data != 0:
            operation = {
                'step': step,
                'pc_before': pc_before,
                'pc_after': pc_after,
                'portb_data': portb_data,
                'porta_control': porta_control,
                'cycles': cycles
            }
            via_operations.append(operation)

            # Decode the control bits
            rs = (porta_control & 0x20) != 0  # RS bit (PA5)
            rw = (porta_control & 0x40) != 0  # RW bit (PA6)
            e = (porta_control & 0x80) != 0   # Enable bit (PA7)

            print(f"Step {step}: PC {pc_before:04X}->{pc_after:04X}, "
                  f"Data=0x{portb_data:02X}, Control=0x{porta_control:02X} "
                  f"(RS={rs}, RW={rw}, E={e})")

            # If we have data with enable high, this might be an LCD write
            if portb_data != 0 and e:
                if rs:  # Data write
                    char = chr(portb_data) if 32 <= portb_data <= 126 else f"\\x{portb_data:02X}"
                    print(f"    -> LCD Data Write: '{char}' (0x{portb_data:02X})")
                else:  # Command write
                    print(f"    -> LCD Command: 0x{portb_data:02X}")

        # Stop if we've returned to the start (program loop completed)
        if step > 0 and pc_after == 0x8000:
            print(f"Program returned to start at step {step}")
            break

    print(f"\nTotal VIA operations detected: {len(via_operations)}")

    # Analyze the operations
    if via_operations:
        print("\nAnalysis of VIA operations:")
        init_commands = 0
        data_writes = 0

        for op in via_operations:
            rs = (op['porta_control'] & 0x20) != 0
            e = (op['porta_control'] & 0x80) != 0

            if e and op['portb_data'] != 0:  # Only count when enable is high
                if rs:  # Data
                    data_writes += 1
                    char = chr(op['portb_data']) if 32 <= op['portb_data'] <= 126 else f"\\x{op['portb_data']:02X}"
                    print(f"  Data: '{char}' (0x{op['portb_data']:02X}) at step {op['step']}")
                else:  # Command
                    init_commands += 1
                    print(f"  Command: 0x{op['portb_data']:02X} at step {op['step']}")

        print(f"\nSummary: {init_commands} init commands, {data_writes} data writes")

        if data_writes == 0:
            print("WARNING: No data writes detected! The program may not be executing correctly.")
            return False
        else:
            print("SUCCESS: VIA operations detected during program execution!")
            return True
    else:
        print("ERROR: No VIA operations detected!")
        return False

if __name__ == "__main__":
    success = test_via_monitoring()
    if success:
        print("\nVIA monitoring test PASSED")
    else:
        print("\nVIA monitoring test FAILED")
        sys.exit(1)
