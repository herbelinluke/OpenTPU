#!/usr/bin/env python3
"""
Convert numpy memory files to binary format for Verilator test harness.

This script converts the numpy array files used by the Python simulators
into binary files that can be easily loaded by the C++ test harness.

Usage:
    python3 convert_mem.py <input.npy> <output.bin> [--format FORMAT]
    
Formats:
    - hostmem: For host memory (8 bytes per entry, uint64_t)
    - weights: For weights memory (TILESIZE bytes per tile)
"""

import argparse
import numpy as np
import sys

def convert_hostmem(input_file, output_file):
    """Convert host memory numpy array to binary format."""
    data = np.load(input_file)
    
    # Ensure it's the right dtype
    if data.dtype != np.int8:
        print(f"Warning: Converting {data.dtype} to int8")
        data = data.astype(np.int8)
    
    # Reshape if needed - each row is a vector
    if len(data.shape) == 2:
        # Flatten to 1D, but preserve row structure
        data = data.flatten()
    
    # Each entry should be 8 bytes (64 bits)
    # Pad or truncate as needed
    output_data = []
    for i in range(len(data)):
        # Create 64-bit value from 8 bytes
        value = 0
        for j in range(min(8, len(data) - i)):
            byte_val = int(data[i + j]) & 0xFF
            value |= (byte_val << (j * 8))
        output_data.append(value)
    
    # Write as binary (little-endian uint64_t)
    with open(output_file, 'wb') as f:
        for value in output_data:
            f.write(value.to_bytes(8, byteorder='little'))
    
    print(f"Converted {len(output_data)} host memory entries")
    print(f"  Input shape: {data.shape}, dtype: {data.dtype}")
    print(f"  Output: {len(output_data)} x 8 bytes = {len(output_data) * 8} bytes")

def convert_weights(input_file, output_file, matsize=8):
    """Convert weights numpy array to binary format."""
    data = np.load(input_file)
    
    # Ensure it's the right dtype
    if data.dtype != np.int8:
        print(f"Warning: Converting {data.dtype} to int8")
        data = data.astype(np.int8)
    
    tilesize = matsize * matsize
    
    # Reshape: each tile should be matsize x matsize
    if len(data.shape) == 3:
        # Shape is (num_tiles, matsize, matsize)
        num_tiles = data.shape[0]
        tiles = data
    elif len(data.shape) == 2:
        # Assume it's already flattened tiles
        num_tiles = data.shape[0]
        tiles = data.reshape(num_tiles, matsize, matsize)
    else:
        raise ValueError(f"Unexpected weights shape: {data.shape}")
    
    # Write tiles as binary
    with open(output_file, 'wb') as f:
        for tile_idx in range(num_tiles):
            tile = tiles[tile_idx]
            # Flatten tile and write bytes
            for row in tile:
                for val in row:
                    byte_val = int(val) & 0xFF
                    f.write(byte_val.to_bytes(1, byteorder='little'))
    
    print(f"Converted {num_tiles} weight tiles")
    print(f"  Input shape: {data.shape}, dtype: {data.dtype}")
    print(f"  Tile size: {matsize}x{matsize} = {tilesize} bytes")
    print(f"  Output: {num_tiles} tiles x {tilesize} bytes = {num_tiles * tilesize} bytes")

def main():
    parser = argparse.ArgumentParser(
        description='Convert numpy memory files to binary format for Verilator test harness'
    )
    parser.add_argument('input', help='Input numpy file (.npy)')
    parser.add_argument('output', help='Output binary file (.bin)')
    parser.add_argument('--format', choices=['hostmem', 'weights'], required=True,
                       help='Format of the memory file')
    parser.add_argument('--matsize', type=int, default=8,
                       help='Matrix size for weights (default: 8)')
    
    args = parser.parse_args()
    
    try:
        if args.format == 'hostmem':
            convert_hostmem(args.input, args.output)
        elif args.format == 'weights':
            convert_weights(args.input, args.output, args.matsize)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()

