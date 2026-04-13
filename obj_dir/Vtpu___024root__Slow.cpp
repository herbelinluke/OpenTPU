// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtpu.h for the primary calling header

#include "Vtpu__pch.h"
#include "Vtpu__Syms.h"
#include "Vtpu___024root.h"

void Vtpu___024root___ctor_var_reset(Vtpu___024root* vlSelf);

Vtpu___024root::Vtpu___024root(Vtpu__Syms* symsp, const char* v__name)
    : VerilatedModule{v__name}
    , vlSymsp{symsp}
 {
    // Reset structure values
    Vtpu___024root___ctor_var_reset(this);
}

void Vtpu___024root::__Vconfigure(bool first) {
    if (false && first) {}  // Prevent unused
}

Vtpu___024root::~Vtpu___024root() {
}
