#pragma once

#include "jb.h"

#include <vector>

namespace jb {

// NOTE these don't return anything, they transform existing IR
struct Mem2Reg {
    static bool run_pass(Function *);
    static bool run_pass(Module *);
};

} // namespace jb
