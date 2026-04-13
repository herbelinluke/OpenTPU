# Verilator TPU Simulation Guide

This guide explains how to use Verilator to simulate the TPU design and how to test it.

## Overview

Verilator compiles your Verilog design (`tpu.v`) into a C++ simulation model. The generated files are in `obj_dir/`:

- `Vtpu` - The compiled executable (if built with `--exe`)
- `Vtpu.h` - C++ header file with the model interface
- `Vtpu.cpp` - C++ implementation of the model
- `Vtpu__main.cpp` - Basic main() function (minimal, needs enhancement)
- Various object files (`.o`) and dependency files (`.d`)

## Building the Test Harness

The test harness (`test_tpu.cpp`) provides a complete simulation environment that:
- Loads program binaries and memory files
- Drives clock and reset signals
- Handles host memory and weights DRAM interfaces
- Monitors halt and assertion signals
- Optionally generates VCD waveform traces

### Compile the Test Harness

```bash
cd obj_dir
g++ -I. -I/usr/share/verilator/include -I/usr/share/verilator/include/vltstd \
    ../test_tpu.cpp Vtpu__ALL.a -o test_tpu \
    -lverilated -lverilated_vcd
```

Or use the Makefile:

```bash
cd obj_dir
make -f Vtpu.mk
# Then link test_tpu.cpp separately, or modify Vtpu__main.cpp
```

## Converting Memory Files

The Python simulators use numpy `.npy` files, but the C++ test harness needs binary files. Use the conversion script:

```bash
# Convert host memory
python3 convert_mem.py simplemult_hostmem.npy simplemult_hostmem.bin --format hostmem

# Convert weights memory  
python3 convert_mem.py simplemult_weights.npy simplemult_weights.bin --format weights
```

## Running the Simulation

```bash
./test_tpu <program.bin> <hostmem.bin> <weights.bin> [options]
```

### Options:
- `--trace <file.vcd>` - Generate VCD waveform trace
- `--max-cycles <N>` - Maximum simulation cycles (default: 1000000)

### Example:

```bash
# Convert memory files first
python3 convert_mem.py simplemult_hostmem.npy simplemult_hostmem.bin --format hostmem
python3 convert_mem.py simplemult_weights.npy simplemult_weights.bin --format weights

# Run simulation
./test_tpu simplemult.out simplemult_hostmem.bin simplemult_weights.bin --trace trace.vcd
```

## Understanding the Output

The test harness will:
1. Load the program and memory files
2. Reset the design for 10 cycles
3. Run the simulation, printing:
   - Host memory read/write operations
   - Weights DRAM transfers
   - Cycle count every 10,000 cycles
4. Stop when `halt` signal is asserted
5. Print final host memory contents

### Memory Interface Details

**Host Memory:**
- `hostmem_re` - Read enable (TPU reads from host)
- `raddr` - Read address (when `hostmem_re` is high)
- `hostmem_rdata` - Read data (driven by test harness)
- `hostmem_we` - Write enable (TPU writes to host)
- `hostmem_waddr` - Write address
- `hostmem_wdata` - Write data

**Weights DRAM:**
- `weights_dram_read` - Read request (TPU requests weights)
- `weights_dram_raddr` - Read address (40-bit)
- `weights_dram_valid` - Data valid (test harness indicates data ready)
- `weights_dram_in` - Weight data (512 bits = 64 bytes per transfer)

Weights are transferred in chunks: for MATSIZE=8, each tile is 64 bytes (8x8), transferred as 8 chunks of 64 bits each.

## Verifying Results

Compare the final host memory with the Python simulator output:

```bash
# Run Python simulator
python3 sim.py simplemult.out simplemult_hostmem.npy simplemult_weights.npy

# Compare outputs
# The C++ test harness prints final memory, compare with sim8.npy or sim32.npy
```

## Debugging

### VCD Waveform Viewing

Use GTKWave or another VCD viewer:

```bash
gtkwave trace.vcd
```

### Common Issues

1. **Simulation hangs**: Check `--max-cycles` limit, verify program has HLT instruction
2. **Memory errors**: Verify memory files are converted correctly, check addresses
3. **Assertion failures**: Check `assertion0` and `assertion1` signals in waveform

### Modifying the Test Harness

The test harness (`test_tpu.cpp`) can be customized:
- Add instruction memory loading (currently assumes instructions are pre-loaded in hardware)
- Add more detailed logging
- Implement different memory models
- Add performance counters

## Integration with Existing Workflow

The Verilator simulation complements the existing Python simulators:

1. **Functional Simulator** (`sim.py`) - High-level, fast, for algorithm verification
2. **PyRTL Simulator** (`runtpu.py`) - Cycle-accurate Python simulation
3. **Verilator Simulator** (`test_tpu`) - Fast C++ simulation, closest to hardware

Use Verilator for:
- Performance benchmarking
- Long-running simulations
- Integration with C++ test frameworks
- Pre-synthesis verification

