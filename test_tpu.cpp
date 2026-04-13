// Verilator test harness for TPU simulation
// Usage: ./test_tpu <program.bin> <hostmem.bin> <weights.bin> [--trace trace.vcd] [--max-cycles N]

#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vtpu.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdint>
#include <cstring>
#include <iomanip>

// Configuration constants
const int INSTRUCTION_WIDTH_BYTES = 14;
const int MATSIZE = 8;  // Should match config.py
const int TILESIZE = MATSIZE * MATSIZE;  // 64 weights per tile
const int NCHUNKS = (TILESIZE + 63) / 64;  // Number of 64-byte chunks per tile

// Memory models
std::map<uint64_t, uint64_t> hostmem;
std::map<uint64_t, uint64_t> weightsmem;  // Address -> tile (512 bits = 64 bytes)
std::vector<uint8_t> program;

// Current state for weight transfer
uint64_t current_weight_addr = 0;
int current_chunk = NCHUNKS;  // Start at NCHUNKS (no active transfer)

// Helper function to read binary file
std::vector<uint8_t> read_binary_file(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        exit(1);
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read((char*)buffer.data(), size)) {
        std::cerr << "Error: Could not read file " << filename << std::endl;
        exit(1);
    }
    return buffer;
}

// Load host memory from binary file
// Format: Each entry is 8 bytes (64 bits), stored as uint64_t in little-endian
void load_hostmem(const char* filename) {
    std::vector<uint8_t> data = read_binary_file(filename);
    size_t num_entries = data.size() / 8;
    
    for (size_t i = 0; i < num_entries; i++) {
        uint64_t value = 0;
        // Read as little-endian
        for (int j = 0; j < 8; j++) {
            value |= ((uint64_t)data[i * 8 + j]) << (j * 8);
        }
        hostmem[i] = value;
    }
    
    std::cout << "Loaded " << num_entries << " host memory entries" << std::endl;
}

// Load weights memory from binary file
// Format: Each tile is TILESIZE bytes (64 bytes for MATSIZE=8)
// Stored as 512-bit values (8 chunks of 64 bits each)
void load_weightsmem(const char* filename) {
    std::vector<uint8_t> data = read_binary_file(filename);
    size_t num_tiles = data.size() / TILESIZE;
    
    for (size_t tile_idx = 0; tile_idx < num_tiles; tile_idx++) {
        uint64_t tile_value = 0;
        // Pack tile into 512-bit value (64 bytes = 8 x 64-bit chunks)
        // Read bytes in reverse order to match Python's concat_tile behavior
        for (int byte_idx = TILESIZE - 1; byte_idx >= 0; byte_idx--) {
            uint8_t byte_val = data[tile_idx * TILESIZE + byte_idx];
            tile_value = (tile_value << 8) | byte_val;
        }
        weightsmem[tile_idx] = tile_value;
    }
    
    std::cout << "Loaded " << num_tiles << " weight tiles" << std::endl;
}

// Load program from binary file
void load_program(const char* filename) {
    program = read_binary_file(filename);
    std::cout << "Loaded " << program.size() << " bytes of program" << std::endl;
    std::cout << "  (" << (program.size() / INSTRUCTION_WIDTH_BYTES) << " instructions)" << std::endl;
}

// Extract a chunk from a tile (for weights DRAM transfer)
uint64_t get_chunk_from_tile(uint64_t tile, int chunk_idx) {
    if (chunk_idx >= NCHUNKS) {
        return 0;
    }
    // Extract 64-bit chunk (512 bits total = 8 chunks)
    int shift = (NCHUNKS - chunk_idx - 1) * 64;
    return (tile >> shift) & 0xFFFFFFFFFFFFFFFFULL;
}

// Print memory contents (for debugging)
void print_hostmem() {
    std::cout << "\nHost Memory Contents:" << std::endl;
    for (const auto& entry : hostmem) {
        std::cout << "  [" << std::hex << std::setw(8) << std::setfill('0') 
                  << entry.first << "] = 0x" 
                  << std::setw(16) << entry.second << std::dec << std::endl;
    }
}

int main(int argc, char** argv) {
    // Parse command line arguments
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] 
                  << " <program.bin> <hostmem.bin> <weights.bin> [--trace trace.vcd] [--max-cycles N]" 
                  << std::endl;
        return 1;
    }
    
    const char* prog_file = argv[1];
    const char* hostmem_file = argv[2];
    const char* weights_file = argv[3];
    const char* trace_file = nullptr;
    uint64_t max_cycles = 1000000;  // Default max cycles
    
    // Parse optional arguments
    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "--trace") == 0 && i + 1 < argc) {
            trace_file = argv[++i];
        } else if (strcmp(argv[i], "--max-cycles") == 0 && i + 1 < argc) {
            max_cycles = strtoull(argv[++i], nullptr, 10);
        }
    }
    
    // Load files
    std::cout << "=== TPU Verilator Test Harness ===" << std::endl;
    load_program(prog_file);
    load_hostmem(hostmem_file);
    load_weightsmem(weights_file);
    
    // Initialize Verilator
    Verilated::debug(0);
    Verilated::randReset(2);
    Verilated::traceEverOn(true);
    
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    const std::unique_ptr<Vtpu> topp{new Vtpu{contextp.get()}};
    
    // Setup VCD tracing
    std::unique_ptr<VerilatedVcdC> tfp;
    if (trace_file) {
        tfp = std::make_unique<VerilatedVcdC>();
        topp->trace(tfp.get(), 99);
        tfp->open(trace_file);
        std::cout << "Tracing to " << trace_file << std::endl;
    }
    
    // Initialize inputs
    topp->clk = 0;
    topp->rst = 1;  // Start in reset
    topp->hostmem_rdata = 0;
    topp->weights_dram_in = 0;
    topp->weights_dram_valid = 0;
    
    // Reset for a few cycles
    std::cout << "\nResetting..." << std::endl;
    for (int i = 0; i < 10; i++) {
        topp->clk = !topp->clk;
        topp->eval();
        if (tfp) tfp->dump(contextp->time());
        contextp->timeInc(1);
    }
    topp->rst = 0;  // Release reset
    std::cout << "Reset released" << std::endl;
    
    // Main simulation loop
    uint64_t cycle = 0;
    bool halted = false;
    
    std::cout << "\nStarting simulation..." << std::endl;
    
    while (cycle < max_cycles && !halted) {
        // Toggle clock
        topp->clk = !topp->clk;
        
        // Evaluate on both edges for proper simulation
        topp->eval();
        
        if (tfp) {
            tfp->dump(contextp->time());
        }
        
        // Handle memory interfaces on positive clock edge
        if (topp->clk) {
            cycle++;
            
            // Handle host memory read
            if (topp->hostmem_re) {
                uint64_t addr = topp->raddr;
                if (hostmem.find(addr) != hostmem.end()) {
                    topp->hostmem_rdata = hostmem[addr];
                    std::cout << "  [Cycle " << cycle << "] Host mem READ: addr=0x" 
                              << std::hex << addr << " data=0x" << hostmem[addr] << std::dec << std::endl;
                } else {
                    std::cerr << "  WARNING: Host mem read from uninitialized address 0x" 
                              << std::hex << addr << std::dec << std::endl;
                    topp->hostmem_rdata = 0;
                }
            }
            
            // Handle host memory write
            if (topp->hostmem_we) {
                uint64_t addr = topp->hostmem_waddr;
                uint64_t data = topp->hostmem_wdata;
                hostmem[addr] = data;
                std::cout << "  [Cycle " << cycle << "] Host mem WRITE: addr=0x" 
                          << std::hex << addr << " data=0x" << data << std::dec << std::endl;
            }
            
            // Handle weights DRAM read request
            if (topp->weights_dram_read) {
                uint64_t addr = topp->weights_dram_raddr & 0xFFFFFFFFFULL;  // Mask to 40 bits
                if (weightsmem.find(addr) != weightsmem.end()) {
                    current_weight_addr = addr;
                    current_chunk = 0;
                    std::cout << "  [Cycle " << cycle << "] Weights DRAM READ: addr=" 
                              << addr << std::endl;
                } else {
                    std::cerr << "  WARNING: Weights DRAM read from uninitialized address " 
                              << addr << std::endl;
                    current_chunk = NCHUNKS;  // No valid transfer
                }
            }
            
            // Handle weights DRAM data transfer
            if (current_chunk < NCHUNKS) {
                uint64_t tile = weightsmem[current_weight_addr];
                uint64_t chunk = get_chunk_from_tile(tile, current_chunk);
                topp->weights_dram_in = chunk;
                topp->weights_dram_valid = 1;
                std::cout << "  [Cycle " << cycle << "] Weights DRAM chunk " << current_chunk 
                          << "/" << NCHUNKS << ": 0x" << std::hex << chunk << std::dec << std::endl;
                current_chunk++;
                if (current_chunk >= NCHUNKS) {
                    topp->weights_dram_valid = 0;
                }
            } else {
                topp->weights_dram_valid = 0;
            }
            
            // Check for halt
            if (topp->halt) {
                halted = true;
                std::cout << "\n*** HALT signal asserted at cycle " << cycle << " ***" << std::endl;
            }
            
            // Check assertions
            if (topp->assertion0 || topp->assertion1) {
                std::cerr << "\n*** ASSERTION FAILED at cycle " << cycle << " ***" << std::endl;
                std::cerr << "  assertion0: " << (topp->assertion0 ? "FAIL" : "OK") << std::endl;
                std::cerr << "  assertion1: " << (topp->assertion1 ? "FAIL" : "OK") << std::endl;
            }
            
            // Progress indicator
            if (cycle % 10000 == 0) {
                std::cout << "  Cycle " << cycle << "..." << std::endl;
            }
        }
        
        contextp->timeInc(1);
    }
    
    if (!halted && cycle >= max_cycles) {
        std::cerr << "\n*** WARNING: Simulation stopped at max cycles (" << max_cycles << ") ***" << std::endl;
    }
    
    // Finalize
    if (tfp) {
        tfp->close();
    }
    
    topp->final();
    
    std::cout << "\n=== Simulation Complete ===" << std::endl;
    std::cout << "Total cycles: " << cycle << std::endl;
    std::cout << "\nFinal Host Memory:" << std::endl;
    print_hostmem();
    
    return halted ? 0 : 1;
}

