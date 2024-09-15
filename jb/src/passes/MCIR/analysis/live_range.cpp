#include "MCIR/analysis/live_range.h"

#include <map>
#include <utility>
#include <iostream>
#include <iomanip>

using namespace jb;
using namespace jb::x86_64;

static IntervalType to_interval_type(i8 kind) {
    switch((GenericMCValueKind)kind) {
        case GenericMCValueKind::none:
		    assert(false);
        case GenericMCValueKind::vreg:
		    return IntervalType::vreg;
        case GenericMCValueKind::mcreg:
		    return IntervalType::mcreg;
		default:
		    assert(false);
	}

	return (IntervalType)-1;
}

static i32 get_reg_num(MCValue value) {
    if(value.is_vreg())
	    return value.reg;
	else if(value.is_mcreg())
	    return value.reg;
	else
	    assert(false);
	
	return -1;
}

using namespace jb;

using enum IntervalType;

std::vector<Interval> LiveRange::run_pass(MCFunction* fn) {
	std::map<i32, i32> result;
	std::vector<Interval> vec_result;

	// current instruction index
	i32 i = 0;
	
	for(auto param: fn->params) {
		auto interval = Interval {
			.type = to_interval_type((i8)param.kind),
			.reg = get_reg_num(param),
			.start = i,
			.end = i,
		};
		result.emplace(get_reg_num(param), vec_result.size());
		vec_result.push_back(interval);
	}

	for(auto* bb: fn->blocks) {
		for(auto inst: bb->insts) {
			++i;
			
			for(int x = 3; x >= 0; --x) {
				MCValue o = inst.operands[x];
				if(o.is_vreg()) {
					if(!result.contains(o.reg)) {
						auto interval = Interval {
							.type = IntervalType::vreg,
							.reg = o.reg,
							.start = i,
							.end = i,
							.hint = (Reg)o.hint,
							.fixed = o.is_fixed
						};
						result.emplace(o.reg, vec_result.size());
						vec_result.push_back(interval);
					} else {
						Interval& interval = vec_result[result[o.reg]];
						interval.end = i;
					}
				}
			}
		}
	}
    for (const auto& [index, res_index]: result) {
		auto interval = vec_result[res_index];
        std::cout << std::setw(10) << index << " | " << interval.reg << " " << interval.start << " " << interval.end << "\n";
	}
		
	// std::vector<Interval> vec_result;
	// vec_result.reserve(result.size());
    // for (const auto& [index, interval]: result)
	// 	vec_result.push_back(interval);

	return vec_result;
}

// TODO should probably return a map of Function names to Interval sets
// or something along those lines
std::vector<Interval> LiveRange::run_pass(MCModule* mod) {
	return {};
}
