# Quick Start: Verilator TPU Simulation

## What is Verilator?

Verilator compiles your Verilog design (`tpu.v`) into a fast C++ simulation model. The generated files are in `obj_dir/`:
- `Vtpu` - The compiled executable (basic, needs enhancement)
- `Vtpu.h` - C++ header with model interface
- `Vtpu__ALL.a` - Static library with the model

## Quick Test

### 1. Build the Test Harness

```bash
make -f Makefile.verilator
```

This compiles `test_tpu.cpp` and links it with the Verilator library.

### 2. Convert Memory Files

The test harness needs binary files, not numpy files:

```bash
# For simplemult test
python3 convert_mem.py simplemult_hostmem.npy simplemult_hostmem.bin --format hostmem
python3 convert_mem.py simplemult_weights.npy simplemult_weights.bin --format weights

# For boston test (MATSIZE=16)
python3 convert_mem.py boston_input.npy boston_input.bin --format hostmem
python3 convert_mem.py boston_weights.npy boston_weights.bin --format weights --matsize 16
```

### 3. Run Simulation

```bash
cd obj_dir
./test_tpu ../simplemult.out ../simplemult_hostmem.bin ../simplemult_weights.bin --trace ../trace.vcd
```

Or use the Makefile targets:

```bash
make -f Makefile.verilator test-simplemult
make -f Makefile.verilator test-boston
```

## Understanding the Output

The test harness will:
1. Load program and memory files
2. Reset the design (10 cycles)
3. Run simulation, printing:
   - Memory read/write operations
   - Weights DRAM transfers
   - Progress every 10,000 cycles
4. Stop when `halt` signal is asserted
5. Print final host memory contents

## View Waveforms

```bash
gtkwave trace.vcd
```

## Files Created

- `test_tpu.cpp` - C++ test harness (drives simulation)
- `convert_mem.py` - Converts numpy files to binary
- `Makefile.verilator` - Build system
- `VERILATOR_TEST.md` - Detailed documentation

## Troubleshooting

**"Verilator library not found"**
- Run Verilator first: `verilator --cc --exe --build tpu.v`

**"Could not open file"**
- Make sure you've converted the memory files with `convert_mem.py`

**Simulation hangs**
- Check `--max-cycles` limit (default: 1M cycles)
- Verify program has HLT instruction

**Memory errors**
- Verify MATSIZE matches between `config.py` and test harness (default: 8)
- Check memory file conversion

## Next Steps

See `VERILATOR_TEST.md` for detailed documentation on:
- Customizing the test harness
- Understanding memory interfaces
- Debugging techniques
- Integration with existing workflow

