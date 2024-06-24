#pragma once

// NOTE: This needs to be converted to the new architecture before being used
// 
// IMPORTANT: This pass takes the IR out of SSA form.
// INFO:
//     Coalesce multiple virtual registers into 1.
//	   Also inserts the return register and coalesces it properly.
//	   This is useful as a prep stage before register allocation.

#include "jb.h"

#include "pass_manager.h"
#include "MCIR/analysis/liveness.h"

namespace jb {

struct RegisterCoalescing {
    static void run_pass(MCFunction *);
    static void run_pass(MCModule *);
};

} // namespace jb
