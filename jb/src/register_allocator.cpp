#include "register_allocator.h"

using namespace jb;

// TODO remove me
#include <iostream>

RegisterAllocator::RegisterAllocator(RegisterManager mng)
    : mng{mng}, active{}, index{0}, mng_copy{mng} {}

void RegisterAllocator::alloc(MCModule *module) {
    for (auto *fn : module->functions) {
        alloc(fn);

        // FIXME temporary
        mng = mng_copy;
        active = {};
        index = 0;
    }
}

void RegisterAllocator::alloc(MCFunction *function) {
    auto intervals = run_analysis_pass<Liveness>(function);

    i32 arg_num = 0;
    for (auto &i : intervals) {
        index = i.start;
        expire_old_intervals(intervals);

        if (i.start == 0) { // if it's one of the function args
            assign_fn_arg(function, i, arg_num++);
        } else {
            assign_to_interval(function, i);
        }
    }

    index += 1;
    expire_old_intervals(intervals);
}

void RegisterAllocator::assign_to_interval(MCFunction *fn, Interval interval) {
    if (interval.is_fixed()) {
        if (active.contains(interval.hint)) {
            // TODO assumes we have enough regs!!
            // if not we would have to spill
            Interval other = active[interval.hint];
            assign_to_interval(fn, other, (Reg)mng.alloc_gpr());
            active.erase(interval.hint);
            mng.free_gpr(interval.hint);
            assign_to_interval(fn, interval, (Reg)mng.alloc_gpr(interval.hint));
        } else {
            assign_to_interval(fn, interval, (Reg)mng.alloc_gpr(interval.hint));
        }
    } else {
        assign_to_interval(fn, interval, (Reg)mng.alloc_gpr());
    }
}

// FIXME does not account for types at all
// always uses a gpr
void RegisterAllocator::assign_to_interval(MCFunction *fn, Interval interval, Reg reg) {
    // add to active map
    active[reg] = interval;
    std::cout << "assigning a register: " << reg << "\n";

    // start at 1 because 0 means it's a fn param
    i32 i = 1;
    for (auto *bb : fn->blocks) {
        for (auto &inst : bb->insts) {
            if (i >= interval.start && i <= interval.end) {
                if (inst.DEST.is_vreg()) {  // FIXME duplication
                    auto num = inst.DEST.reg;

                    if (interval.reg == num)
                        inst.DEST = MCValue((i8)GenericMCValueKind::mcreg,
                                            inst.DEST.type, reg);
                }
                if (inst.SRC1.is_vreg()) {
                    auto num = inst.SRC1.reg;

                    if (interval.reg == num)
                        inst.SRC1 = MCValue((i8)GenericMCValueKind::mcreg,
                                            inst.SRC1.type, reg);
                }
                if (inst.SRC2.is_vreg()) {
                    auto num = inst.SRC2.reg;

                    if (interval.reg == num)
                        inst.SRC2 = MCValue((i8)GenericMCValueKind::mcreg,
                                            inst.SRC2.type, reg);
                }
            }
            ++i;
        }
    }
}

void RegisterAllocator::expire_old_intervals(std::vector<Interval> intervals) {
    std::vector<Reg> delete_list;
    for (auto &[reg, interval] : active) {
        if (index >= interval.end) {
            std::cout << "expired a register: " << reg << "\n";
            mng.free_gpr(reg);
            delete_list.push_back(reg);
        }
    }
    for (auto reg : delete_list)
        active.erase(reg);
}

void RegisterAllocator::assign_fn_arg(MCFunction *fn, Interval interval,
                                      i32 arg_num) {
    auto &arg = fn->params[arg_num];
    // TODO fix this to work properly
    //	needs to query target arch and calling convention
    //	also needs to actually take type into account
    if ((i32)arg.type <= (i32)Type::i64) {
        // have to add 1 here to match with calling convention stuff
        auto reg = get_gpr_param(CallConv::win64, arg_num + 1);
        arg = IRValue{arg.type, reg};
        mng.alloc_gpr(reg);
        assign_to_interval(fn, interval, reg);
    } else {
        assert(false);
    }
}
