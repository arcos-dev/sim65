#!/usr/bin/env python3
"""
Debug what hello_world() actually returns
"""

import sys
import os

# Add the python_bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__)))

from programs_6502 import Programs6502

def debug_hello_world():
    """Debug the hello_world() function output"""

    programs = Programs6502()
    hello_world_program = programs.hello_world()

    print(f"Type of hello_world_program: {type(hello_world_program)}")
    print(f"Value: {hello_world_program}")
    print(f"Length: {len(hello_world_program)}")

    if hasattr(hello_world_program, '__iter__'):
        print("It's iterable")
        try:
            for i, item in enumerate(hello_world_program):
                print(f"  [{i}]: {item} (type: {type(item)})")
                if i >= 10:  # Limit output
                    break
        except Exception as e:
            print(f"Error iterating: {e}")

    # Try to convert to bytes
    try:
        if isinstance(hello_world_program, str):
            print("Converting string to bytes...")
            hello_world_bytes = hello_world_program.encode('latin-1')
            print(f"Bytes: {hello_world_bytes[:20]}")
        elif hasattr(hello_world_program, 'encode'):
            print("Has encode method, trying...")
            hello_world_bytes = hello_world_program.encode()
            print(f"Bytes: {hello_world_bytes[:20]}")
        else:
            print("Cannot convert to bytes")
    except Exception as e:
        print(f"Error converting to bytes: {e}")

if __name__ == "__main__":
    debug_hello_world()
