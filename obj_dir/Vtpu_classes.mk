# Verilated -*- Makefile -*-
# DESCRIPTION: Verilator output: Make include file with class lists
#
# This file lists generated Verilated files, for including in higher level makefiles.
# See Vtpu.mk for the caller.

### Switches...
# C11 constructs required?  0/1 (always on now)
VM_C11 = 1
# Timing enabled?  0/1
VM_TIMING = 0
# Coverage output mode?  0/1 (from --coverage)
VM_COVERAGE = 0
# Parallel builds?  0/1 (from --output-split)
VM_PARALLEL_BUILDS = 1
# Tracing output mode?  0/1 (from --trace/--trace-fst)
VM_TRACE = 0
# Tracing output mode in VCD format?  0/1 (from --trace)
VM_TRACE_VCD = 0
# Tracing output mode in FST format?  0/1 (from --trace-fst)
VM_TRACE_FST = 0

### Object file lists...
# Generated module classes, fast-path, compile with highest optimization
VM_CLASSES_FAST += \
	Vtpu \
	Vtpu___024root__DepSet_h2e5e179d__0 \
	Vtpu___024root__DepSet_hb46007f4__0 \
	Vtpu___024root__DepSet_hb46007f4__1 \
	Vtpu___024root__DepSet_hb46007f4__2 \
	Vtpu___024root__DepSet_hb46007f4__3 \
	Vtpu___024root__DepSet_hb46007f4__4 \
	Vtpu___024root__DepSet_hb46007f4__5 \
	Vtpu___024root__DepSet_hb46007f4__6 \
	Vtpu___024root__DepSet_hb46007f4__7 \
	Vtpu___024root__DepSet_hb46007f4__8 \
	Vtpu___024root__DepSet_hb46007f4__9 \
	Vtpu___024root__DepSet_hb46007f4__10 \
	Vtpu___024root__DepSet_hb46007f4__11 \
	Vtpu___024root__DepSet_hb46007f4__12 \
	Vtpu___024root__DepSet_hb46007f4__13 \
	Vtpu__main \

# Generated module classes, non-fast-path, compile with low/medium optimization
VM_CLASSES_SLOW += \
	Vtpu___024root__Slow \
	Vtpu___024root__DepSet_h2e5e179d__0__Slow \
	Vtpu___024root__DepSet_hb46007f4__0__Slow \
	Vtpu___024root__DepSet_hb46007f4__1__Slow \
	Vtpu___024root__DepSet_hb46007f4__2__Slow \
	Vtpu___024root__DepSet_hb46007f4__3__Slow \
	Vtpu___024root__DepSet_hb46007f4__4__Slow \
	Vtpu___024root__DepSet_hb46007f4__5__Slow \
	Vtpu___024root__DepSet_hb46007f4__6__Slow \
	Vtpu___024root__DepSet_hb46007f4__7__Slow \
	Vtpu___024root__DepSet_hb46007f4__8__Slow \
	Vtpu___024root__DepSet_hb46007f4__9__Slow \
	Vtpu___024root__DepSet_hb46007f4__10__Slow \

# Generated support classes, fast-path, compile with highest optimization
VM_SUPPORT_FAST += \

# Generated support classes, non-fast-path, compile with low/medium optimization
VM_SUPPORT_SLOW += \
	Vtpu__Syms \

# Global classes, need linked once per executable, fast-path, compile with highest optimization
VM_GLOBAL_FAST += \
	verilated \
	verilated_threads \

# Global classes, need linked once per executable, non-fast-path, compile with low/medium optimization
VM_GLOBAL_SLOW += \


# Verilated -*- Makefile -*-
