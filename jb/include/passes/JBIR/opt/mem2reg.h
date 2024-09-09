#pragma once

#include "jb.h"

#include <vector>

namespace jb {

// NOTE these don't return anything, they transform existing IR
struct Mem2Reg {
    static void run_pass(Function *);
    static void run_pass(Module *);
};

} // namespace jb
