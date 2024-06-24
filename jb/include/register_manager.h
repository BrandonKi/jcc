// Cross-platform register manager

#pragma once

#include "jb.h"
// TODO temporary
// #include "arch/x86_64/mcir.h"
#include "arch/generic_mcir.h"

#include <set>
#include <algorithm>

namespace jb {

using RegisterSet = std::set<Reg>;

class RegisterManager {
public:
    // each target must provide:
    RegisterSet gpr_mask;
    RegisterSet caller_saved_gpr_mask;

    RegisterManager();
    // TODO will enable reserving regs across function calls
    //		RegisterManager(reserved_registers);

    void init();

    Reg alloc_gpr();
    Reg alloc_gpr(Reg);
    void free_gpr(Reg);
    void spill_gpr(Reg);

    //        foreach_spilled_reg();

    // using two sets for convenience
    // could definitely get away with one
    RegisterSet used_gpr_set;
    RegisterSet free_gpr_set;
    RegisterSet spilled_gpr_set;
};

} // namespace jb
