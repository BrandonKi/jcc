#ifndef JAB_X86_64_MDIR_H
#define JAB_X86_64_MDIR_H

#include "jab.h"

namespace jab::x86_64 {

// TODO make this an enum class
// NOTE the ordering is intentional
enum Register: MIRegister {
    rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi,
    r8, r9, r10, r11, r12, r13, r14, r15,

    eax, ecx, edx, ebx, esp, ebp, esi, edi,
    r8d, r9d, r10d, r11d, r12d, r13d, r14d, r15d,

    ax, cx, dx, bx, sp, bp, si, di,
    r8w, r9w, r10w, r11w, r12w, r13w, r14w, r15w,

	// only includes low regs
    al, cl, dl, bl, ah, ch, dh, bh,
    r8b, r9b, r10b, r11b, r12b, r13b, r14b, r15b,

    xmm0, xmm1, xmm2,  xmm3,  xmm4,  xmm5,  xmm6,  xmm7,
    xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15,

    ymm0, ymm1, ymm2,  ymm3,  ymm4,  ymm5,  ymm6,  ymm7,
    ymm8, ymm9, ymm10, ymm11, ymm12, ymm13, ymm14, ymm15,

	none,
};

inline byte id(Register reg) {
	using enum Register;

	if(rax <= reg && reg <= r15)
		return reg;
	else if(eax <= reg && reg <= r15d)
		return reg - 16;
	else if(ax <= reg && reg <= r15w)
		return reg - 32;
	else if(al <= reg && reg <= r15b)
		return reg - 48;
	else if(xmm0 <= reg && reg <= xmm15)
		return reg - 64;
	else if(ymm0 <= reg && reg <= ymm15)
		return reg - 80;
	else
		unreachable
	return -1;
}

inline i32 size(Register reg) {
	using enum Register;

	if(rax <= reg && reg <= r15)
		return 64;
	else if(eax <= reg && reg <= r15d)
		return 32;
	else if(ax <= reg && reg <= r15w)
		return 16;
	else if(al <= reg && reg <= r15b)
		return 8;
	else if(xmm0 <= reg && reg <= xmm15)
		return 128;
	else if(ymm0 <= reg && reg <= ymm15)
		return 256;
	else
		unreachable
	return -1;
}

// registers r8-r15/r8d-r15d/r8w-r15w/r8b-r15b
inline bool is_extended(Register reg) {
	return (int)reg & 0x08;
}

enum class Condition: i8 {
	above,
	above_equal,
	below,
	below_equal,
	carry,
	equal,
	greater,
	greater_equal,
	lesser,
	lesser_equal,
};

enum class Opcode: i8 {
	mov,
	mov_reg_imm,
	mov_reg_scale,
	mov_scale_imm,
	mov_mem_imm,
	mov_index_imm,

	cmov,

	add,
	add_reg_imm,
	add_reg_scale,
	add_scale_imm,
	add_mem_imm,
	add_index_imm,

	call,
	jmp,
	ret,
	
	push,
	push_mem,
	push_imm,
	pop,
	pop_mem,

	syscall,
	breakpoint,
	nop,
};

}  // namespace jab::x86_64

#endif // JAB_X86_64_MDIR_H
