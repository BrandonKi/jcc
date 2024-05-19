// Cross-platform register manager

#ifndef JAB_REGISTER_MANAGER_H
#define JAB_REGISTER_MANAGER_H

#include "jab.h"

#include <set>
#include <algorithm>

namespace jab {

using RegisterSet = std::set<MIRegister>;

class RegisterManager {
	public:
	    // each target must provide:
		RegisterSet gpr_mask;
        RegisterSet caller_saved_gpr_mask;
	
		RegisterManager();
		// TODO will enable reserving regs across function calls 
//		RegisterManager(reserved_registers);

        void init();

		MIRegister alloc_gpr();
		void alloc_gpr(MIRegister);
		void free_gpr(MIRegister);
		void spill_gpr(MIRegister);

//        foreach_spilled_reg();


        // using two sets for convenience
		// could definitely get away with one 
		RegisterSet used_gpr_set;
		RegisterSet free_gpr_set;
		RegisterSet spilled_gpr_set;
	};
	
} // jab

#endif // JAB_REGISTER_MANAGER_H
