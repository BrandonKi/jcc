// Machine Description File
// pretty much just a bunch of arch specific stuff at the moment
// will be refactored in the future

#pragma once

#include "jb.h"
#include "register_manager.h"
#include "arch/x86_64/mcir.h"

#include <set>

namespace jb::x86_64 {

// TODO assumes win64 calling convention
inline RegisterManager register_manager() {
    RegisterManager rm;
    rm.gpr_mask = {rbx, rbp, rsp, rdi, rsi, r12, r13, r14, r15,
                   rax, rcx, rdx, r8,  r9,  r10, r11};
    // xmm6 | xmm7 | xmm8 |
    // xmm9 | xmm10 | xmm11 |
    // xmm12 | xmm13 | xmm14 |
    // xmm15;

    rm.caller_saved_gpr_mask = {rax, rcx, rdx, r8, r9, r10, r11};
    // xmm0 |
    // xmm1 | xmm2 | xmm3 |
    // xmm4 | xmm5;

    rm.init();
    return rm;
}

} // namespace jb::x86_64
