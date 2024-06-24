#pragma once

#include "jb.h"
// TODO temporary
#include "arch/x86_64/mcir.h"

#include <vector>

namespace jb {

enum class IntervalType : i8 { vreg, mcreg };

struct Interval {
    IntervalType type;
    Reg reg;
    i32 start;
    i32 end;
    Reg hint = -1;

    bool is_vreg() {
        return type == IntervalType::vreg;
    }

    bool is_mreg() {
        return type == IntervalType::mcreg;
    }

    bool has_hint() { return hint != x86_64::MCReg::none; }
};

struct Liveness {
    static std::vector<Interval> run_pass(MCFunction *);
    static std::vector<Interval> run_pass(MCModule *);
};

} // namespace jb
