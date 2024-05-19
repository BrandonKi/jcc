#ifndef JAB_OPT_TWO_ADDRESS_H
#define JAB_OPT_TWO_ADDRESS_H

// INFO:
//     Transform three address instructions into two address instructions.
//	   For example, it does the following transformation
//	       %2 = add %0, %1
//		   to
//	       %0 = add %0, %1
//		   %2 = mov %0

#include "jab.h"

#include "pass_manager.h"

namespace jab {

struct TwoAddress {
	static void run_pass(Function*);
	static void run_pass(Module*);
};

} // namespace jab

#endif // JAB_OPT_TWO_ADDRESS_H
