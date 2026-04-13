// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Model implementation (design independent parts)

#include "Vtpu__pch.h"

//============================================================
// Constructors

Vtpu::Vtpu(VerilatedContext* _vcontextp__, const char* _vcname__)
    : VerilatedModel{*_vcontextp__}
    , vlSymsp{new Vtpu__Syms(contextp(), _vcname__, this)}
    , clk{vlSymsp->TOP.clk}
    , rst{vlSymsp->TOP.rst}
    , weights_dram_valid{vlSymsp->TOP.weights_dram_valid}
    , assertion0{vlSymsp->TOP.assertion0}
    , assertion1{vlSymsp->TOP.assertion1}
    , halt{vlSymsp->TOP.halt}
    , hostmem_re{vlSymsp->TOP.hostmem_re}
    , hostmem_we{vlSymsp->TOP.hostmem_we}
    , weights_dram_read{vlSymsp->TOP.weights_dram_read}
    , weights_dram_in{vlSymsp->TOP.weights_dram_in}
    , hostmem_rdata{vlSymsp->TOP.hostmem_rdata}
    , hostmem_waddr{vlSymsp->TOP.hostmem_waddr}
    , hostmem_wdata{vlSymsp->TOP.hostmem_wdata}
    , raddr{vlSymsp->TOP.raddr}
    , weights_dram_raddr{vlSymsp->TOP.weights_dram_raddr}
    , rootp{&(vlSymsp->TOP)}
{
    // Register model with the context
    contextp()->addModel(this);
}

Vtpu::Vtpu(const char* _vcname__)
    : Vtpu(Verilated::threadContextp(), _vcname__)
{
}

//============================================================
// Destructor

Vtpu::~Vtpu() {
    delete vlSymsp;
}

//============================================================
// Evaluation function

#ifdef VL_DEBUG
void Vtpu___024root___eval_debug_assertions(Vtpu___024root* vlSelf);
#endif  // VL_DEBUG
void Vtpu___024root___eval_static(Vtpu___024root* vlSelf);
void Vtpu___024root___eval_initial(Vtpu___024root* vlSelf);
void Vtpu___024root___eval_settle(Vtpu___024root* vlSelf);
void Vtpu___024root___eval(Vtpu___024root* vlSelf);

void Vtpu::eval_step() {
    VL_DEBUG_IF(VL_DBG_MSGF("+++++TOP Evaluate Vtpu::eval_step\n"); );
#ifdef VL_DEBUG
    // Debug assertions
    Vtpu___024root___eval_debug_assertions(&(vlSymsp->TOP));
#endif  // VL_DEBUG
    vlSymsp->__Vm_deleter.deleteAll();
    if (VL_UNLIKELY(!vlSymsp->__Vm_didInit)) {
        vlSymsp->__Vm_didInit = true;
        VL_DEBUG_IF(VL_DBG_MSGF("+ Initial\n"););
        Vtpu___024root___eval_static(&(vlSymsp->TOP));
        Vtpu___024root___eval_initial(&(vlSymsp->TOP));
        Vtpu___024root___eval_settle(&(vlSymsp->TOP));
    }
    VL_DEBUG_IF(VL_DBG_MSGF("+ Eval\n"););
    Vtpu___024root___eval(&(vlSymsp->TOP));
    // Evaluate cleanup
    Verilated::endOfEval(vlSymsp->__Vm_evalMsgQp);
}

//============================================================
// Events and timing
bool Vtpu::eventsPending() { return false; }

uint64_t Vtpu::nextTimeSlot() {
    VL_FATAL_MT(__FILE__, __LINE__, "", "%Error: No delays in the design");
    return 0;
}

//============================================================
// Utilities

const char* Vtpu::name() const {
    return vlSymsp->name();
}

//============================================================
// Invoke final blocks

void Vtpu___024root___eval_final(Vtpu___024root* vlSelf);

VL_ATTR_COLD void Vtpu::final() {
    Vtpu___024root___eval_final(&(vlSymsp->TOP));
}

//============================================================
// Implementations of abstract methods from VerilatedModel

const char* Vtpu::hierName() const { return vlSymsp->name(); }
const char* Vtpu::modelName() const { return "Vtpu"; }
unsigned Vtpu::threads() const { return 1; }
void Vtpu::prepareClone() const { contextp()->prepareClone(); }
void Vtpu::atClone() const {
    contextp()->threadPoolpOnClone();
}

//============================================================
// Trace configuration

VL_ATTR_COLD void Vtpu::trace(VerilatedVcdC* tfp, int levels, int options) {
    vl_fatal(__FILE__, __LINE__, __FILE__,"'Vtpu::trace()' called on model that was Verilated without --trace option");
}
