#pragma once

#include "jb.h"

#include "arch/generic_mcir.h"

namespace jb::x86_64 {

// clang-format off
// TODO make this an enum class
// NOTE the ordering is intentional
enum MCReg: Reg {
    none = -1,
    rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi,
    r8, r9, r10, r11, r12, r13, r14, r15,

    eax, ecx, edx, ebx, esp, ebp, esi, edi,
    r8d, r9d, r10d, r11d, r12d, r13d, r14d, r15d,

    ax, cx, dx, bx, sp, bp, si, di,
    r8w, r9w, r10w, r11w, r12w, r13w, r14w, r15w,

	// FIXME this is missing some regs
    al, cl, dl, bl, ah, ch, dh, bh,
    r8b, r9b, r10b, r11b, r12b, r13b, r14b, r15b,

    xmm0, xmm1, xmm2,  xmm3,  xmm4,  xmm5,  xmm6,  xmm7,
    xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15,

    ymm0, ymm1, ymm2,  ymm3,  ymm4,  ymm5,  ymm6,  ymm7,
    ymm8, ymm9, ymm10, ymm11, ymm12, ymm13, ymm14, ymm15,
};
// clang-format on

// NOTE depends on enum order
inline byte id(MCReg reg) {
    using enum MCReg;

    if (rax <= reg && reg <= r15)
        return reg;
    else if (eax <= reg && reg <= r15d)
        return reg - 16;
    else if (ax <= reg && reg <= r15w)
        return reg - 32;
    else if (al <= reg && reg <= r15b)
        return reg - 48;
    else if (xmm0 <= reg && reg <= xmm15)
        return reg - 64;
    else if (ymm0 <= reg && reg <= ymm15)
        return reg - 80;
    else
        assert(false);
    return -1;
}

// NOTE depends on enum order
inline i32 size(MCReg reg) {
    using enum MCReg;

    if (rax <= reg && reg <= r15)
        return 64;
    else if (eax <= reg && reg <= r15d)
        return 32;
    else if (ax <= reg && reg <= r15w)
        return 16;
    else if (al <= reg && reg <= r15b)
        return 8;
    else if (xmm0 <= reg && reg <= xmm15)
        return 128;
    else if (ymm0 <= reg && reg <= ymm15)
        return 256;
    else
        assert(false);
    return -1;
}

// NOTE depends on enum order
// registers r8-r15/r8d-r15d/r8w-r15w/r8b-r15b
inline bool is_extended(MCReg reg) {
    return (int)reg & 0x08;
}

enum class Condition : i8 {
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

enum class MCType { none = 0, byte = 1, word = 2, dword = 4, qword = 8 };

inline int size(MCType mc_type) {
    return (int)mc_type * 8;
}

inline MCType to_mc_type(Type type) {
    switch (type) {
    case Type::none:
        return MCType::none;
    case Type::i8:
        return MCType::byte;
    case Type::i16:
        return MCType::word;
    case Type::i32:
    case Type::f32:
        return MCType::dword;
    case Type::i64:
    case Type::f64:
    case Type::ptr:
        return MCType::qword;
    default:
        assert(false);
    }
    return MCType::none;
}

enum class OpCode : i8 {
    label = -1,
#define X(a) a,
#include "insts.inc"
#undef X
};

// enum class EncodingKind {
//     NONE = -1,
//     RX_0 = 0,
//     RX_1 = 1,
//     RX_2 = 2,
//     RX_3 = 3,
//     RX_4 = 4,
//     RX_5 = 5,
//     RX_6 = 6,
//     RX_7 = 7,
//     BASIC,
//     OFFSET,
//     REG_RM,
//     MOD_R_RM,
//     PREFIX_0x0f,
// };

// struct OpCodeDesc {
//     i32 op; // FIXME unecessary size
//     EncodingKind enc_kind;
//     i8 num_operands;
// };

// static const OpCodeDesc OpCodeTable[] = {
// #define X(a, b, c, d, e) OpCodeDesc{e, EncodingKind::c, d},
// #include "insts.inc"
// #undef X
// };

enum class MCValueKind : i8 {
    none = (i8)GenericMCValueKind::none,
    vreg = (i8)GenericMCValueKind::vreg,
    mcreg = (i8)GenericMCValueKind::mcreg,
    imm = (i8)GenericMCValueKind::imm,
    lbl = (i8)GenericMCValueKind::lbl,
    slot = (i8)GenericMCValueKind::slot,
    mem = (i8)GenericMCValueKind::mem,
};

} // namespace jb::x86_64
