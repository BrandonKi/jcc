#pragma once

#include "jb.h"

#include <vector>

namespace jb {

// NOTE these don't return anything, they just update info on existing structures
struct Liveness {
    static void run_pass(Function *);
    static void run_pass(Module *);
};

} // namespace jb
