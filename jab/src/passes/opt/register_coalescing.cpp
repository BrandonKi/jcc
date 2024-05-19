#include "opt/register_coalescing.h"

using namespace jab;

// FIXME has no concept of fixed registers
void RegisterCoalescing::run_pass(Function* fn) {
	//first we insert the return register

	for(auto* block: fn->blocks) {
		for(i32 i = 0; i < block->insts.size(); ++i) {
			auto inst = block->insts[i];

			if(inst.op == IROp::ret) {
				auto new_inst = IRInst(IROp::mov, fn->ret, inst.src1, {});
				block->insts.insert(block->insts.begin() + i - 1, new_inst);
			}
		}
	}
	
	auto intervals = run_analysis_pass<Liveness>(fn);

}

void RegisterCoalescing::run_pass(Module* module) {

}
