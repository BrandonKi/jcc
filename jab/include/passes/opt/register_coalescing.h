#ifndef JAB_OPT_REGISTER_COALESCING_PASSES_H
#define JAB_OPT_REGISTER_COALESCING_PASSES_H

// IMPORTANT: This pass takes the IR out of SSA form.
// INFO:
//     Coalesce multiple virtual registers into 1.
//	   Also inserts the return register and coalesces it properly.
//	   This is useful as a prep stage before register allocation.

#include "jab.h"

#include "pass_manager.h"
#include "analysis/liveness.h"

namespace jab {

struct RegisterCoalescing {
	static void run_pass(Function*);
	static void run_pass(Module*);
};

} // namespace jab

#endif // JAB_OPT_REGISTER_COALESCING_PASSES_H
