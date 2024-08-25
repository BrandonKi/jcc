#pragma once

#include "jb.h"

#include "module_builder.h"
#include "jit_env.h"
#include "register_manager.h"
#include "arch/generic_mcir.h"

#include "pass_manager.h"
#include "MCIR/analysis/liveness.h"

#include <map>

namespace jb {

class RegisterAllocator {
public:
    RegisterAllocator(RegisterManager);

    void alloc(MCModule *);
    void alloc(MCFunction *);

private:
    RegisterManager mng;

    std::map<Reg, Interval> active;
    i32 index;

    RegisterManager mng_copy;

    void assign_to_interval(MCFunction *, Interval);
    void assign_to_interval(MCFunction *, Interval, Reg);

    void expire_old_intervals(std::vector<Interval>);

    void assign_fn_arg(MCFunction *, Interval, i32);
};

} // namespace jb
