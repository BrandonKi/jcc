#pragma once

#include "jb.h"
// TODO temporary
#include "arch/generic_mcir.h"

#include "passes/analysis_passes.h"
#include "passes/opt_passes.h"

namespace jb {

// FIXME, gg, just make the passes dynamic...
// it's not worth the effort

template <typename Pass>
auto run_analysis_pass(Function *fn) {
    return Pass::run_pass(fn);
}

template <typename Pass>
auto run_analysis_pass(Module *mod) {
    return Pass::run_pass(mod);
}

template <typename Pass>
auto run_opt_pass(Function *fn) {
    return Pass::run_pass(fn);
}

template <typename Pass>
auto run_opt_pass(Module *mod) {
    return Pass::run_pass(mod);
}


template <typename Pass>
auto run_analysis_pass(MCFunction *fn) {
    return Pass::run_pass(fn);
}

template <typename Pass>
auto run_analysis_pass(MCModule *mod) {
    return Pass::run_pass(mod);
}

template <typename Pass>
auto run_opt_pass(MCFunction *fn) {
    return Pass::run_pass(fn);
}

template <typename Pass>
auto run_opt_pass(MCModule *mod) {
    return Pass::run_pass(mod);
}

} // namespace jb
