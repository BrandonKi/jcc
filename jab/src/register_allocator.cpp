#include "register_allocator.h"

using namespace jab;

// TODO remove me
#include <iostream>

RegisterAllocator::RegisterAllocator(RegisterManager mng):
	mng{mng},
	active{},
	index{0}
{

}

void RegisterAllocator::alloc(Module* module) {
    for(auto* fn: module->functions) {
        alloc(fn);
	}
}

void RegisterAllocator::alloc(Function* function) {
    auto intervals = run_analysis_pass<Liveness>(function);

	i32 arg_num = 0;
	for(auto& i: intervals) {
		index = i.start;
		expire_old_intervals(intervals);

		if(i.start == 0) {	// if it's one of the function args
			assign_fn_arg(function, i, arg_num++);
		}
		else {
			assign_to_interval(function, i);
		}
	}

}

void RegisterAllocator::assign_to_interval(Function* fn, Interval interval) {
	assign_to_interval(fn, interval, mng.alloc_gpr());
}

// FIXME does not account for types at all
// always uses a gpr
void RegisterAllocator::assign_to_interval(Function* fn, Interval interval, MIRegister mireg) {
	// add to active map
	active[mireg] = interval;

	auto reg = HReg{mireg};

	// start at 1 because 0 means it's a fn param
	i32 i = 1;
	for(auto* bb: fn->blocks) {
		for(auto& inst: bb->insts) {
			if(i >= interval.start && i <= interval.end) {
				if(inst.dest_is_vreg()) {
					auto num = inst.dest.vreg.num;

					if(interval.reg == num)
						inst.dest = IRValue(inst.dest.type, reg);
				}
				if(inst.src1_is_vreg()) {
					auto num = inst.src1.vreg.num;

					if(interval.reg == num)
						inst.src1 = IRValue(inst.src1.type, reg);
				}
				if(inst.src2_is_vreg()) {
					auto num = inst.src2.vreg.num;

					if(interval.reg == num)
						inst.src2 = IRValue(inst.src2.type, reg);
				}
			}
			++i;
		}
	}
}

void RegisterAllocator::expire_old_intervals(std::vector<Interval> intervals) {
	std::vector<MIRegister> delete_list;
	for(auto& [reg, interval]: active) {
		if(index > interval.end) {
			std::cout << "expired a register: " << reg << "\n";
			mng.free_gpr(reg);
			delete_list.push_back(reg);
		}
	}
	for(auto reg: delete_list)
		active.erase(reg);
}

void RegisterAllocator::assign_fn_arg(Function* fn, Interval interval, i32 arg_num) {
	auto& arg = fn->params[arg_num];
	// TODO fix this to work properly
	//	needs to query target arch and calling convention
	//	also needs to take type into account for real
	if((i32)arg.type <= (i32)Type::i64) {
		// have to add 1 here to match with calling convention stuff
		auto reg = x86_64::get_gpr_param(CallConv::win64, arg_num + 1);
		arg = IRValue{arg.type, HReg{reg}};
		mng.alloc_gpr(reg);
		assign_to_interval(fn, interval, reg);
	}
	else {
		unreachable
	}
}
