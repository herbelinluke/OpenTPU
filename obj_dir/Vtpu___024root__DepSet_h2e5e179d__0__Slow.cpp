// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtpu.h for the primary calling header

#include "Vtpu__pch.h"
#include "Vtpu__Syms.h"
#include "Vtpu___024root.h"

#ifdef VL_DEBUG
VL_ATTR_COLD void Vtpu___024root___dump_triggers__stl(Vtpu___024root* vlSelf);
#endif  // VL_DEBUG

VL_ATTR_COLD void Vtpu___024root___eval_triggers__stl(Vtpu___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtpu__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtpu___024root___eval_triggers__stl\n"); );
    // Body
    vlSelf->__VstlTriggered.set(0U, (IData)(vlSelf->__VstlFirstIteration));
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vtpu___024root___dump_triggers__stl(vlSelf);
    }
#endif
}
