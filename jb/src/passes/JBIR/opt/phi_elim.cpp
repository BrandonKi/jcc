#include "JBIR/opt/phi_elim.h"
#include "pretty_print.h"

#include <iostream>
#include <algorithm>

using namespace jb;

// TODO use command line flag
constexpr auto PHI_ELIM_DEBUG = false;

static void elim_phis(Function *fn) {
    for(auto *bb: fn->blocks) {
        bb->for_each(IROp::phi, [=](IRInst *inst) {
            for(auto &[bb, val]: inst->values) {
                bb->insts.push_back(new IRInst(IROp::mov, inst->dest, val));
                std::iter_swap(bb->insts.end()-1, bb->insts.end()-2);
            }
            inst->op = IROp::noop;
            return nullptr;
        });
    }
}

void PhiElim::run_pass(Function *function) {
    elim_phis(function);
}

void PhiElim::run_pass(Module *module) {
    for(auto *f: module->functions)
        PhiElim::run_pass(f);
}
