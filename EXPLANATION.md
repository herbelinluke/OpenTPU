# OpenTPU Architecture and Math Explanation

This document explains the mathematics and architecture of the OpenTPU system, helping you understand what's happening when you run the tests.

## Table of Contents
1. [The Core Math: Matrix Multiplication](#the-core-math-matrix-multiplication)
2. [Neural Network Inference](#neural-network-inference)
3. [TPU Architecture Overview](#tpu-architecture-overview)
4. [The Systolic Array](#the-systolic-array)
5. [Data Flow Through the Pipeline](#data-flow-through-the-pipeline)
6. [Key Files to Explore](#key-files-to-explore)

---

## The Core Math: Matrix Multiplication

### What the TPU Does

The TPU performs **matrix multiplication**, which is the core operation in neural network inference. Given:
- **Input matrix** `X` of shape `(N, M)` - N vectors, each with M elements
- **Weight matrix** `W` of shape `(M, K)` - M×K weights

The TPU computes: **Y = X × W**, resulting in shape `(N, K)`

### Example (Simple Matrix Multiply Test)

For `MATSIZE=8`, you're computing:
- Input: 8 vectors, each with 8 elements → `(8, 8)` matrix
- Weights: `(8, 8)` matrix
- Output: `(8, 8)` matrix

Each output element `Y[i][j]` is computed as:
```
Y[i][j] = Σ(k=0 to 7) X[i][k] × W[k][j]
```

This is a **dot product** between row `i` of X and column `j` of W.

### Why This Matters for Neural Networks

In a neural network layer:
- **X** = input activations (from previous layer or input data)
- **W** = learned weights (parameters)
- **Y** = output activations (before activation function)

After matrix multiply, you typically apply an **activation function**:
- **ReLU**: `max(0, x)` - sets negative values to 0
- **Sigmoid**: `1/(1+e^(-x))` - squashes values to [0,1]

---

## Neural Network Inference

### Boston Housing Example

The Boston housing test runs a **3-layer neural network**:

1. **Layer 1**: Input (13 features) → Hidden (8 neurons)
   - Input: `(10, 13)` - 10 samples, 13 features each
   - Weights: `(13, 8)` - 13 inputs → 8 outputs
   - Output: `(10, 8)`

2. **Layer 2**: Hidden (8) → Hidden (8)
   - Input: `(10, 8)` (from Layer 1, after ReLU)
   - Weights: `(8, 8)`
   - Output: `(10, 8)`

3. **Layer 3**: Hidden (8) → Output (1)
   - Input: `(10, 8)` (from Layer 2, after ReLU)
   - Weights: `(8, 1)` - final prediction
   - Output: `(10, 1)` - predicted house prices

### The Math Flow

```
Input → [Matrix Multiply] → Accumulator → [ReLU] → Unified Buffer → [Matrix Multiply] → ...
```

Each layer:
1. Reads input vectors from Unified Buffer
2. Multiplies by weight matrix (stored in Weight DRAM)
3. Accumulates results (can add to previous values)
4. Applies activation function (ReLU)
5. Stores back to Unified Buffer for next layer

---

## TPU Architecture Overview

The TPU has several key components:

### 1. **Unified Buffer (UB)**
- **Purpose**: On-chip SRAM storing input/output vectors
- **Size**: Configurable (default ~96K vectors)
- **Data**: 8-bit integers (or 32-bit floats in raw mode)
- **Usage**: Holds activations between layers

**File**: `tpu.py` lines 38-42

### 2. **Matrix Multiply Unit (MMU)**
- **Core**: Systolic array of MACs (Multiply-Accumulate units)
- **Size**: `MATSIZE × MATSIZE` (8×8 or 16×16 in your tests)
- **Operation**: Performs matrix multiplication in parallel

**Files**: `matrix.py` (entire file), `tpu.py` lines 53-58

### 3. **Accumulator Buffers**
- **Purpose**: Stores 32-bit intermediate results from matrix multiply
- **Size**: Configurable (default ~4K vectors)
- **Data**: 32-bit integers (can accumulate without overflow)

**File**: `matrix.py` lines 165-221

### 4. **Weight DRAM**
- **Purpose**: Off-chip memory storing weight matrices
- **Access**: Via Weight FIFO (4-entry buffer)
- **Data**: 8-bit integers per weight

**Files**: `matrix.py` lines 224-325 (FIFO), `tpu.py` lines 139-146

### 5. **Activation Unit**
- **Purpose**: Applies activation functions (ReLU, Sigmoid, or pass-through)
- **Input**: 32-bit values from accumulators
- **Output**: 8-bit values back to Unified Buffer

**Files**: `activate.py`, `tpu.py` lines 61-74

### 6. **Host Memory**
- **Purpose**: Main memory interface (input data, final outputs)
- **Access**: Via RHM (Read Host Memory) and WHM (Write Host Memory) instructions

**File**: `tpu.py` lines 78-136

---

## The Systolic Array

### What is a Systolic Array?

A **systolic array** is a 2D grid of processing elements (MACs) that process data in a coordinated, rhythmic way - like a heartbeat ("systolic").

### How It Works

```
Input vectors enter from the LEFT
Weights are stored in each MAC
Partial sums flow DOWN
Results exit from the BOTTOM
```

### Visual Example (4×4 array):

```
Cycle 1:     Cycle 2:     Cycle 3:     Cycle 4:
[W] [W] [W] [W]  [W] [W] [W] [W]  [W] [W] [W] [W]  [W] [W] [W] [W]
 ↓   ↓   ↓   ↓    ↓   ↓   ↓   ↓    ↓   ↓   ↓   ↓    ↓   ↓   ↓   ↓
[x0] → → →    [x1] → → →    [x2] → → →    [x3] → → →
 ↓   ↓   ↓   ↓    ↓   ↓   ↓   ↓    ↓   ↓   ↓   ↓    ↓   ↓   ↓   ↓
[0]  → → →    [p0] → → →    [p1] → → →    [p2] → → →
 ↓   ↓   ↓   ↓    ↓   ↓   ↓   ↓    ↓   ↓   ↓   ↓    ↓   ↓   ↓   ↓
[0]  → → →    [0]  → → →    [p0] → → →    [p1] → → →
 ↓   ↓   ↓   ↓    ↓   ↓   ↓   ↓    ↓   ↓   ↓   ↓    ↓   ↓   ↓   ↓
[0]  → → →    [0]  → → →    [0]  → → →    [p0] → → →
```

**Key Points**:
- Each MAC multiplies: `input × weight`
- Adds to partial sum from above: `new_sum = old_sum + (input × weight)`
- Data flows **diagonally** so values align correctly

### Why Diagonal?

The **systolic setup** (`matrix.py` lines 327-414) creates a diagonal delay pattern:
- First element of vector → immediate
- Second element → 1 cycle delay
- Third element → 2 cycle delay
- etc.

This ensures that when element `i` reaches column `j`, the partial sum from row `i` has also reached column `j`.

### The MAC Unit

Each MAC (`matrix.py` lines 8-90) performs:
```python
product = input_value × weight_value  # 8-bit × 8-bit = 16-bit
result = partial_sum + product        # 32-bit accumulation
```

**Two Weight Buffers**: Each MAC has two weight buffers to allow loading new weights while using current ones (double-buffering).

---

## Data Flow Through the Pipeline

### Example: Simple Matrix Multiply Test

Let's trace through `simplemult.a`:

#### Step 1: Load Input Data
```
RHM 0, 0, 8
```
- **Read Host Memory**: Copy 8 vectors from host memory address 0
- **To Unified Buffer**: Store at UB address 0
- **Latency**: 8 cycles (one per vector)

#### Step 2: Load Weights
```
RW 0
```
- **Read Weights**: Load weight tile from DRAM address 0
- **To Weight FIFO**: Store in 4-entry FIFO buffer
- **Latency**: `(MATSIZE²)/64` cycles (64-byte DRAM transfers)

#### Step 3: Matrix Multiply
```
MMC.S 0, 0, 8
```
- **Matrix Multiply/Convolution**: Process 8 vectors
- **From UB**: Read vectors starting at address 0
- **To Accumulator**: Store results starting at address 0
- **Flags**: `.S` = Switch weights (use next tile from FIFO)
- **Latency**: `L + 2×MATSIZE` cycles (L=8 vectors, MATSIZE=8 → 24 cycles)

**What happens inside**:
1. Vectors read from UB → systolic setup (diagonal delay)
2. Enter systolic array from left
3. Each MAC: `accum += input × weight`
4. Results accumulate down columns
5. Exit bottom → accumulator buffers

#### Step 4: Activation
```
ACT.R 0, 0, 8
```
- **Activate**: Apply ReLU to 8 accumulator values
- **From Accumulator**: Read starting at address 0
- **To UB**: Write back starting at address 0
- **Function**: `.R` = ReLU (max(0, x))
- **Latency**: `L + 1` cycles (9 cycles)

**ReLU Math**: `output = max(0, input)` - clamps negative values to 0

#### Step 5: Write Results
```
WHM 0, 0, 8
```
- **Write Host Memory**: Copy 8 vectors from UB address 0
- **To Host Memory**: Store at address 0
- **Latency**: 8 cycles

### Example: Boston Housing (Multi-Layer)

The Boston test (`boston.a`) repeats the pattern 3 times:

```
RHM 0, 0, 10          # Load 10 input samples (13 features each)
RW 0                  # Load Layer 1 weights (13×8)
MMC.SO 0, 0, 10       # Layer 1: (10×13) × (13×8) → (10×8)
ACT.R 0, 0, 10        # ReLU activation
RW 1                  # Load Layer 2 weights (8×8)
MMC.SO 0, 0, 10       # Layer 2: (10×8) × (8×8) → (10×8)
ACT.R 0, 0, 10        # ReLU activation
RW 2                  # Load Layer 3 weights (8×1)
MMC.SO 0, 0, 10       # Layer 3: (10×8) × (8×1) → (10×1)
ACT.R 0, 0, 10        # ReLU activation
WHM 0, 0, 10          # Write final predictions
```

**Note**: The `.O` flag means "overwrite" accumulator (don't add to previous value).

---

## Key Files to Explore

### Understanding the Math

1. **`sim.py`** (Functional Simulator)
   - **Lines 113-132**: `matrix_multiply_convolve()` - Shows the math:
     ```python
     out = np.matmul(inp, weight_mat)  # Standard matrix multiply
     ```
   - **Lines 75-94**: `act()` - Activation functions (ReLU, Sigmoid)
   - **Best for**: Understanding what computation is being performed

2. **`matrix.py`** (Hardware Implementation)
   - **Lines 8-90**: `MAC()` - Single Multiply-Accumulate unit
   - **Lines 93-162**: `MMArray()` - Systolic array of MACs
   - **Lines 327-414**: `systolic_setup()` - Diagonal delay pattern
   - **Best for**: Understanding how the hardware computes

### Understanding the Pipeline

3. **`tpu.py`** (Top-Level TPU)
   - **Lines 23-32**: Instruction memory and PC (program counter)
   - **Lines 38-42**: Unified Buffer declaration
   - **Lines 48**: Instruction decoder
   - **Lines 53-58**: Matrix Multiply Unit connection
   - **Lines 61-74**: Activation Unit connection
   - **Lines 78-136**: Host memory read/write logic
   - **Best for**: Understanding overall data flow

4. **`decoder.py`**
   - **Lines 9-93**: Instruction decoding logic
   - Parses binary instructions into control signals
   - **Best for**: Understanding instruction format

### Understanding Execution

5. **`runtpu.py`** (Hardware Simulator)
   - **Lines 120-182**: Simulation loop
   - Handles memory reads/writes, weight loading
   - **Best for**: Understanding how instructions execute

6. **`isa.py`**
   - Instruction set architecture definition
   - Opcodes, flags, instruction encoding
   - **Best for**: Understanding instruction format

### Test Files

7. **`simplemult.a`** / **`boston.a`**
   - Assembly programs showing instruction sequences
   - **Best for**: Understanding program structure

---

## Key Concepts Summary

### 1. **Quantization**
- Neural networks trained in 32-bit floats
- TPU operates on **8-bit integers** for efficiency
- Conversion happens in `tf_nn.py` / `simple_nn.py`
- Simulator can run in both modes (`--raw` flag)

### 2. **Systolic Processing**
- Data flows through array in coordinated pattern
- Enables high parallelism (all MACs working simultaneously)
- Reduces memory bandwidth (weights stay in place)

### 3. **Double Buffering**
- Two weight buffers per MAC
- Allows loading next weights while using current ones
- Prevents pipeline stalls

### 4. **Accumulation**
- Matrix multiply produces 32-bit results
- Can accumulate multiple products (for convolution)
- `.O` flag overwrites; default adds to existing value

### 5. **Pipeline Latency**
- Each instruction has fixed latency
- NOPs pad execution to handle delays
- No dynamic scheduling (deterministic execution)

---

## Next Steps for Exploration

1. **Modify `config.py`**: Change `MATSIZE` to see how array size affects performance
2. **Add print statements**: In `sim.py` to see intermediate values
3. **Trace execution**: Use `runtpu.py` and examine the VCD trace file
4. **Create your own test**: Write a simple `.a` file to test specific operations
5. **Read the Google TPU paper**: Understand the original design decisions

---

## Common Questions

**Q: Why so many NOPs?**
A: The hardware has fixed latencies. NOPs ensure previous instructions complete before starting new ones. The compiler (you, writing assembly) must schedule correctly.

**Q: Why 8-bit integers?**
A: Reduces memory bandwidth and power consumption. Neural networks are often robust to quantization.

**Q: What's the difference between `runtpu.py` and `sim.py`?**
A: `runtpu.py` simulates the actual hardware (PyRTL). `sim.py` is a functional simulator (numpy) - faster but less accurate to hardware timing.

**Q: How does the systolic array handle matrices larger than MATSIZE?**
A: The matrix is "tiled" - processed in chunks. Each tile is MATSIZE×MATSIZE. Results are accumulated across tiles.

---

This should give you a solid foundation for understanding the OpenTPU! Start with `sim.py` to understand the math, then dive into `matrix.py` to see how it's implemented in hardware.
