#pragma once

#include "jb.h"
#include "mcir.h"
#include "mcir_gen.h"
#include "../../pretty_print.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

namespace jb::x86_64 {

inline std::string str(MCReg reg) {
    switch (reg) {
    case rax:
        return "rax";
    case rcx:
        return "rcx";
    case rdx:
        return "rdx";
    case rbx:
        return "rbx";
    case rsp:
        return "rsp";
    case rbp:
        return "rbp";
    case rsi:
        return "rsi";
    case rdi:
        return "rdi";
    case r8:
        return "r8";
    case r9:
        return "r9";
    case r10:
        return "r10";
    case r11:
        return "r11";
    case r12:
        return "r12";
    case r13:
        return "r13";
    case r14:
        return "r14";
    case r15:
        return "r15";
    case eax:
        return "eax";
    case ecx:
        return "ecx";
    case edx:
        return "edx";
    case ebx:
        return "ebx";
    case esp:
        return "esp";
    case ebp:
        return "ebp";
    case esi:
        return "esi";
    case edi:
        return "edi";
    case r8d:
        return "r8d";
    case r9d:
        return "r9d";
    case r10d:
        return "r10d";
    case r11d:
        return "r11d";
    case r12d:
        return "r12d";
    case r13d:
        return "r13d";
    case r14d:
        return "r14d";
    case r15d:
        return "r15d";
    case ax:
        return "ax";
    case cx:
        return "cx";
    case dx:
        return "dx";
    case bx:
        return "bx";
    case sp:
        return "sp";
    case bp:
        return "bp";
    case si:
        return "si";
    case di:
        return "di";
    case r8w:
        return "r8w";
    case r9w:
        return "r9w";
    case r10w:
        return "r10w";
    case r11w:
        return "r11w";
    case r12w:
        return "r12w";
    case r13w:
        return "r13w";
    case r14w:
        return "r14w";
    case r15w:
        return "r15w";
    case al:
        return "al";
    case cl:
        return "cl";
    case dl:
        return "dl";
    case bl:
        return "bl";
    case ah:
        return "ah";
    case ch:
        return "ch";
    case dh:
        return "dh";
    case bh:
        return "bh";
    case r8b:
        return "r8b";
    case r9b:
        return "r9b";
    case r10b:
        return "r10b";
    case r11b:
        return "r11b";
    case r12b:
        return "r12b";
    case r13b:
        return "r13b";
    case r14b:
        return "r14b";
    case r15b:
        return "r15b";
    case xmm0:
        return "xmm0";
    case xmm1:
        return "xmm1";
    case xmm2:
        return "xmm2";
    case xmm3:
        return "xmm3";
    case xmm4:
        return "xmm4";
    case xmm5:
        return "xmm5";
    case xmm6:
        return "xmm6";
    case xmm7:
        return "xmm7";
    case xmm8:
        return "xmm8";
    case xmm9:
        return "xmm9";
    case xmm10:
        return "xmm10";
    case xmm11:
        return "xmm11";
    case xmm12:
        return "xmm12";
    case xmm13:
        return "xmm13";
    case xmm14:
        return "xmm14";
    case xmm15:
        return "xmm15";
    case ymm0:
        return "ymm0";
    case ymm1:
        return "ymm1";
    case ymm2:
        return "ymm2";
    case ymm3:
        return "ymm3";
    case ymm4:
        return "ymm4";
    case ymm5:
        return "ymm5";
    case ymm6:
        return "ymm6";
    case ymm7:
        return "ymm7";
    case ymm8:
        return "ymm8";
    case ymm9:
        return "ymm9";
    case ymm10:
        return "ymm10";
    case ymm11:
        return "ymm11";
    case ymm12:
        return "ymm12";
    case ymm13:
        return "ymm13";
    case ymm14:
        return "ymm14";
    case ymm15:
        return "ymm15";
    case none:
        return "none";
    default:
        assert(false);
        return "";
    }
}

inline std::string str(MCValue mc_val) {
    switch (mc_val.kind) {
    case (i32)MCValueKind::none:
        return "###";
    case (i32)MCValueKind::vreg:
        if (mc_val.has_hint())
            return "%" + std::to_string((i32)mc_val.reg) + ":" +
                   str((MCReg)mc_val.hint);
        return "%" + std::to_string((i32)mc_val.reg);
    case (i32)MCValueKind::mcreg:
        return "$" + str((MCReg)mc_val.reg);
    case (i32)MCValueKind::slot:
        return "[" + std::to_string(mc_val.offset) + "]";
    case (i32)MCValueKind::mem:
        return "mem";
    case (i32)MCValueKind::imm:
        return std::to_string(mc_val.imm);
    case (i32)MCValueKind::lbl:
        return mc_val.label;
    default:
        assert(false);
        return "";
    }
}

inline std::string str(Condition condition) {
    using enum Condition;

    switch (condition) {
    case above:
        return "above";
    case above_equal:
        return "above_equal";
    case below:
        return "below";
    case below_equal:
        return "below_equal";
    case carry:
        return "carry";
    case equal:
        return "equal";
    case greater:
        return "greater";
    case greater_equal:
        return "greater_equal";
    case lesser:
        return "lesser";
    case lesser_equal:
        return "lesser_equal";
    default:
        assert(false);
    }
}

// inline std::string str(Opcode op) {
//     using enum Opcode;
//
//     switch (op) {
//     case mov:
//         return "mov";
//     case mov_reg_imm:
//         return "mov_reg_imm";
//     case mov_reg_scale:
//         return "mov_reg_scale";
//     case mov_scale_imm:
//         return "mov_scale_imm";
//     case mov_mem_imm:
//         return "mov_mem_imm";
//     case mov_index_imm:
//         return "mov_index_imm";
//     case cmov:
//         return "cmov";
//     case add:
//         return "add";
//     case add_reg_imm:
//         return "add_reg_imm";
//     case add_reg_scale:
//         return "add_reg_scale";
//     case add_scale_imm:
//         return "add_scale_imm";
//     case add_mem_imm:
//         return "add_mem_imm";
//     case add_index_imm:
//         return "add_index_imm";
//     case call:
//         return "call";
//     case jmp:
//         return "jmp";
//     case ret:
//         return "ret";
//     case push:
//         return "push";
//     case push_mem:
//         return "push_mem";
//     case push_imm:
//         return "push_imm";
//     case pop:
//         return "pop";
//     case pop_mem:
//         return "pop_mem";
//     case syscall:
//         return "syscall";
//     case breakpoint:
//         return "breakpoint";
//     case nop:
//         return "nop";
//     default:
//         assert(false);
//         return "";
//     }
// }

inline std::string str(OpCode op) {

    switch (op) {
#define X(a)                                                           \
    case OpCode::a:                                                            \
        return #a;
#include "insts.inc"
#undef X
    default:
        assert(false);
        return "";
    }
    return "";
}

inline std::string str(MCInst i) {
    using enum OpCode;

    std::string ret_str = str((OpCode)i.op) + ' ';

    switch ((OpCode)i.op) {
    case mov:
        return ret_str + str(i.DEST) + ", " + str(i.SRC1);
    // case mov_imm:
    //     return ret_str + str(i.DEST) + ", " + std::to_string(i.SRC1.imm);
    // case mov_reg_scale:
    //     return ret_str + str(i.DEST);
    // case mov_scale_imm:
    //     return ret_str;
    // case mov_mem_imm:
    //     return ret_str;
    // case mov_index_imm:
    //     return ret_str;
    case cmov:
        return ret_str + str(i.DEST) + ", " + str(i.SRC1);
    case add:
    case sub:
    case imul:
    case idiv:
        return ret_str + str(i.DEST) + ", " + str(i.SRC1);
    // case add_reg_imm:
    //     return ret_str + str(i.DEST) + ", " + std::to_string(i.SRC1.imm);
    // case add_reg_scale:
    //     return ret_str + str(i.DEST);
    // case add_scale_imm:
    // return ret_str;
    // case add_mem_imm:
    // return ret_str;
    // case add_index_imm:
    // return ret_str;
    case call:
        return ret_str + str(i.DEST);
    case jmp:
        return ret_str;
    case ret:
        return ret_str + str(i.DEST);
    case push:
        return ret_str + str(i.DEST);
    // case push_mem:
    // return ret_str;
    // case push_imm:
    // return ret_str + std::to_string(i.SRC1.imm);
    case pop:
        return ret_str + str(i.DEST);
    // case pop_mem:
    // return ret_str;
    case syscall:
        return ret_str;
    case breakpoint:
        return ret_str;
    case nop:
        return ret_str;
    default:
        assert(false);
        return "";
    }

    return "";
}

inline void pretty_print(MCModule *mm) {
    int tab_count = 0;

    std::cout << mm->name << ":\n";
    ++tab_count;
    for (auto *fn : mm->functions) {
        std::string btab_string(tab_count, '\t');
        std::string ntab_string(tab_count + 1, '\t');

        std::string fn_string = btab_string + "[" + str(fn->callconv) + "]\n" +
                                btab_string + fn->id + '(';

        for (auto param : fn->params) {
            fn_string += str(param) + ":" + str(param.type) + ", ";
        }
        if (!fn->params.empty()) {
            fn_string[fn_string.size() - 2] = ')';
            fn_string[fn_string.size() - 1] = ' ';
        } else
            fn_string += ") ";

        fn_string += str(fn->ret) + ":" + str(fn->ret.type);
        std::cout << fn_string << "\n";

        for (auto bb : fn->blocks) {
            for (auto inst : bb->insts) {
                std::cout << ntab_string << str(inst) << "\n";
            }
        }

    }
    std::cout << '\n';
}

inline void pretty_print(std::vector<byte> &buf) {
    std::cout << std::hex;
    for (byte i : buf)
        std::cout << std::setw(2) << std::setfill('0') << (int)i << " ";
    std::cout << std::setw(0) << std::setfill(' ') << std::endl;
}

} // namespace jb::x86_64
