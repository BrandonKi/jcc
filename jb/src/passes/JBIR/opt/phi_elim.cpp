#include "JBIR/opt/phi_elim.h"
#include "pretty_print.h"

#include <iostream>

using namespace jb;

// TODO use command line flag
constexpr auto PHI_ELIM_DEBUG = false;

void PhiElim::run_pass(Function *function) {
    // insert_phis(function);
}

void PhiElim::run_pass(Module *module) {
    for(auto *f: module->functions)
        PhiElim::run_pass(f);
}
