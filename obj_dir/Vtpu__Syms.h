// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table internal header
//
// Internal details; most calling programs do not need this header,
// unless using verilator public meta comments.

#ifndef VERILATED_VTPU__SYMS_H_
#define VERILATED_VTPU__SYMS_H_  // guard

#include "verilated.h"

// INCLUDE MODEL CLASS

#include "Vtpu.h"

// INCLUDE MODULE CLASSES
#include "Vtpu___024root.h"

// SYMS CLASS (contains all model state)
class alignas(VL_CACHE_LINE_BYTES)Vtpu__Syms final : public VerilatedSyms {
  public:
    // INTERNAL STATE
    Vtpu* const __Vm_modelp;
    VlDeleter __Vm_deleter;
    bool __Vm_didInit = false;

    // MODULE INSTANCE STATE
    Vtpu___024root                 TOP;

    // CONSTRUCTORS
    Vtpu__Syms(VerilatedContext* contextp, const char* namep, Vtpu* modelp);
    ~Vtpu__Syms();

    // METHODS
    const char* name() { return TOP.name(); }
};

#endif  // guard
