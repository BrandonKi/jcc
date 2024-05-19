// Machine Description File
// pretty much just a bunch of arch specific stuff at the moment
// will be refactored in the future

#ifndef JAB_X86_64_MDF_H
#define JAB_X86_64_MDF_H

#include "jab.h"
#include "register_manager.h"
#include "arch/x86_64/mdir.h"

#include <set>

namespace jab::x86_64 {

// TODO assumes win64 calling convention
inline RegisterManager register_manager() {
	RegisterManager rm;
	rm.gpr_mask = {
	    rbx, rbp, rdi,
		rsi, r12, r13,
		r14, r15, rax,
		rcx, rdx, r8,
		r9, r10, r11
	};
        // xmm6 | xmm7 | xmm8 |
		// xmm9 | xmm10 | xmm11 |
		// xmm12 | xmm13 | xmm14 |
		// xmm15;

	rm.caller_saved_gpr_mask = {
        rax, rcx, rdx, r8,
		r9, r10, r11
	};
        // xmm0 |
		// xmm1 | xmm2 | xmm3 |
		// xmm4 | xmm5;

    rm.init();
	return rm;
}


// start of CallConv stuff
#define STACK -1

inline MIRegister get_gpr_param(CallConv callconv, MIRegister num) {
	if(callconv == CallConv::win64) {
		switch(num) {
			case 1:
				return rcx;
			case 2:
				return rdx;
			case 3:
				return r8;
			case 4:
				return r9;
			default:
				if(num >= 5)
					return STACK;
				else
					unreachable
		}
	}
	else {
		// unimplemented
		unreachable
	}
}

inline MIRegister get_fpr_param(CallConv callconv, MIRegister num) {
	if(callconv == CallConv::win64) {
		switch(num) {
			case 1:
				return xmm0;
			case 2:
				return xmm1;
			case 3:
				return xmm2;
			case 4:
				return xmm3;
			default:
				if(num >= 5)
					return STACK;
				else
					unreachable
		}
	}
	else {
		// unimplemented
		unreachable
	}
}

// TODO this is not correct
inline i32 get_aggregate_param(CallConv callconv, MIRegister num) {
	return get_gpr_param(callconv, num);
}

inline i32 get_gpr_ret() {
	return rax;
}

inline i32 get_fpr_ret() {
	return xmm0;
}

// TODO can't return aggregates yet
// usually depends on the size of the aggregate
inline i32 get_aggregate_ret() {
	return rax;
}

} // jab::x86_64

#endif // JAB_X86_64_MDF_H
