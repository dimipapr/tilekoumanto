#!/usr/bin/env python3
import sys
import re

def main():
    try:
        with open('size_output.txt', 'r') as f:
            lines = f.readlines()
        
        # Parse the numbers out of the berkeley size output line
        match = re.search(r'\s*(\d+)\s+(\d+)\s+(\d+)', lines[1])
        if not match:
            print("Could not parse size tool output.")
            sys.exit(1)
            
        text, data, bss = map(int, match.groups())
        flash_used = text + data
        sram_used = data + bss
        
        # NUCLEO-F446RE Layout specs
        FLASH_SIZE = 512 * 1024  # 524288 Bytes
        SRAM_SIZE  = 128 * 1024  # 131072 Bytes
        
        flash_pct = (flash_used / FLASH_SIZE) * 100
        sram_pct  = (sram_used / SRAM_SIZE) * 100
        
        print('\n================= HARDWARE MEMORY REPORT =================')
        print(f'  FLASH: {flash_used:7,} / {FLASH_SIZE:,} Bytes ({flash_pct:.2f}%)')
        print(f'  SRAM:  {sram_used:7,} / {SRAM_SIZE:,} Bytes ({sram_pct:.2f}%)')
        print('==========================================================\n')
        
    except Exception as e:
        print(f"Failed to generate memory metrics: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()