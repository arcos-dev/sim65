#!/usr/bin/env python3
"""
Decode the actual characters being written to LCD
"""

def decode_lcd_data():
    """Decode the LCD data we captured"""

    # Data values from the VIA monitoring test
    lcd_data = [0x08, 0x05, 0x0C, 0x0C, 0x0F, 0x20, 0x17, 0x0F, 0x12, 0x0C, 0x04, 0x21]

    print("Expected 'HELLO WORLD!':")
    expected = "HELLO WORLD!"
    for i, char in enumerate(expected):
        print(f"  [{i:2d}]: '{char}' = 0x{ord(char):02X}")

    print("\nActual LCD data captured:")
    for i, value in enumerate(lcd_data):
        char = chr(value) if 32 <= value <= 126 else f"\\x{value:02X}"
        print(f"  [{i:2d}]: 0x{value:02X} = '{char}'")

    print("\nDecoding the pattern...")

    # Let's see if there's a pattern - maybe the program is writing something else
    # Try different interpretations
    print("\nDirect ASCII interpretation:")
    decoded = ''.join(chr(b) if 32 <= b <= 126 else f"\\x{b:02X}" for b in lcd_data)
    print(f"  '{decoded}'")

    # Check if it's some kind of encoding
    print("\nTrying to map to 'HELLO WORLD!':")
    hello_world = "HELLO WORLD!"
    if len(lcd_data) == len(hello_world):
        for i, (actual, expected) in enumerate(zip(lcd_data, hello_world)):
            expected_val = ord(expected)
            diff = actual - expected_val
            print(f"  [{i:2d}]: got 0x{actual:02X}, expected 0x{expected_val:02X} ('{expected}'), diff={diff}")
    else:
        print(f"  Length mismatch: got {len(lcd_data)} bytes, expected {len(hello_world)}")

if __name__ == "__main__":
    decode_lcd_data()
