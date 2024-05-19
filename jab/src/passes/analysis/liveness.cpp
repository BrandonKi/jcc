#include "analysis/liveness.h"

#include <map>
#include <utility>
#include <iostream>
#include <iomanip>

using namespace jab;

static IntervalType to_interval_type(IRValueKind kind) {
    switch(kind) {
        case IRValueKind::none:
		    unreachable
        case IRValueKind::vreg:
		    return IntervalType::vreg;
        case IRValueKind::hreg:
		    return IntervalType::hreg;
		default:
		    unreachable
	}
}

static i32 get_reg_num(IRValue value) {
    if(value.is_vreg())
	    return value.vreg.num;
	else if(value.is_hreg())
	    return value.hreg.num;
	else
	    unreachable
}

using namespace jab;

using enum IntervalType;

std::vector<Interval> Liveness::run_pass(Function* fn) {
	std::map<i32, Interval> result;

	// current instruction index
	i32 i = 0;
	
	for(auto param: fn->params) {
		result.emplace(get_reg_num(param), Interval {
			.type = to_interval_type(param.kind),
			.reg = get_reg_num(param),
			.start = i,
			.end = i,
		});
	}

	for(auto* bb: fn->blocks) {
		// TODO add intervals for bb params here
		for(auto inst: bb->insts) {
			++i;
			
			if(inst.dest_is_vreg()) {
				auto dest = inst.dest.vreg.num;

				result.emplace(dest, Interval {
				    .type = IntervalType::vreg,
					.reg = dest,
					.start = i,
					.end = i,
				});
			}
			if(inst.src1_is_vreg()) {
				auto src1 = inst.src1.vreg.num;
				assert(result.contains(src1));
				
				result[src1] = Interval {
				    .type = IntervalType::vreg,
					.reg = result[src1].reg,
					.start = result[src1].start,
					.end = i,
				};
			}
			if(inst.src2_is_vreg()) {
				auto src2 = inst.src2.vreg.num;
				assert(result.contains(src2));
				
				result[src2] = Interval {
				    .type = IntervalType::vreg,
					.reg = result[src2].reg,
					.start = result[src2].start,
					.end = i,
				};
			}
		}
	}
    for (const auto& [index, interval]: result)
        std::cout << std::setw(10) << index << " | " << interval.reg << " " << interval.start << " " << interval.end << "\n";
		
	std::vector<Interval> vec_result;
	vec_result.reserve(result.size());
    for (const auto& [index, interval]: result)
		vec_result.push_back(interval);

	return vec_result;
}

// TODO should probably return a map of Module names to Interval sets
// or something along those lines
std::vector<Interval> Liveness::run_pass(Module* mod) {
	return {};
}
