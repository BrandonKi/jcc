#ifndef JAB_PASS_MANAGER_H
#define JAB_PASS_MANAGER_H

#include "jab.h"

#include "passes/analysis_passes.h"
#include "passes/opt_passes.h"

namespace jab {

template<typename Pass>
auto run_analysis_pass(Function* fn) {
	return Pass::run_pass(fn);
}

template<typename Pass>
auto run_analysis_pass(Module* mod) {
	return Pass::run_pass(mod);
}

template<typename Pass>
auto run_opt_pass(Function* fn) {
	return Pass::run_pass(fn);
}

template<typename Pass>
auto run_opt_pass(Module* mod) {
	return Pass::run_pass(mod);
}

} // namespace jab

#endif // JAB_PASS_MANAGER_H
