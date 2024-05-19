#ifndef JAB_ANALYSIS_LIVENESS_H
#define JAB_ANALYSIS_LIVENESS_H

#include "jab.h"

#include <vector>

namespace jab {

enum class IntervalType: i8 {
	vreg,
	hreg
};

struct Interval {
    IntervalType type;
	MIRegister reg;
	i32 start;
	i32 end;

	bool is_vreg() {
        return type == IntervalType::vreg;
	}

	bool is_hreg() {
        return type == IntervalType::hreg;
	}
};

struct Liveness {
	static std::vector<Interval> run_pass(Function*);
	static std::vector<Interval> run_pass(Module*);
};

} // namespace jab

#endif // JAB_ANALYSIS_LIVENESS_H
