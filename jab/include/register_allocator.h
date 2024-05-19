#ifndef JAB_REGISTER_ALLOCATOR_H
#define JAB_REGISTER_ALLOCATOR_H

#include "jab.h"

#include "module_builder.h"
#include "jit_env.h"
#include "register_manager.h"
#include "arch/x86_64/mdir_gen.h"
#include "arch/x86_64/mdf.h"

#include "pass_manager.h"
#include "analysis/liveness.h"

#include <map>

namespace jab {

class RegisterAllocator {
public:
    RegisterAllocator(RegisterManager);
	
	void alloc(Module*);
	void alloc(Function*);

private:
	RegisterManager mng;

	std::map<MIRegister, Interval> active;
	i32 index;

	void assign_to_interval(Function*, Interval);
	void assign_to_interval(Function*, Interval, MIRegister);

	void expire_old_intervals(std::vector<Interval>);


	void assign_fn_arg(Function*, Interval, i32);
};

} // namespace jab

#endif // JAB_REGISTER_ALLOCATOR_H
