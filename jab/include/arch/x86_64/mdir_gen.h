#ifndef JAB_X86_64_MDIR_GEN_H
#define JAB_X86_64_MDIR_GEN_H

#include "jab.h"
#include "arch/x86_64/mdir.h"

namespace jab::x86_64 {

inline Register to_mdreg(IRValue val) {
	return (Register)val.hreg.num;
}

// TODO maybe move this stuff to a better spot?
enum class MCValueKind: i8 {
	none,
	reg,
	mem,
	imm,
};

struct MCValue {
	MCValueKind kind;
	Type type;
	union {
	    Register reg;
		i64 imm;
	};
};

struct MCInst {
	Opcode op;
	Register reg1;
	Register reg2;

	union {
		Register reg3;
		i64 imm;
		Condition cond;
		struct {
			// TODO stuff for mem
			// scale, index, base, etc.
		} mem;
	} extra;
};

struct MCFunction {
	std::string id;
	std::vector<MCValue> params;
	MCValue ret;
	CallConv callconv;
	std::vector<MCInst> insts;

	MCFunction(Function* fn): id{fn->id}, params{}, ret{}, callconv{fn->callconv} {
		using enum MCValueKind;
		// TODO not sure what to do with the type field here or if should even exist
		for(auto param: fn->params) {
			params.push_back({
				.kind = reg,
				.type = param.type,
				.reg = to_mdreg(param)});
		}
		ret = {
			.kind = reg,
			.type = fn->ret.type,
			.reg = Register::rax};
	}

	MCValue param(int);
};

struct MCModule {
	std::string name;
	// TODO need a symtab of some sort
	// TODO also need storage:
	//     * data
	//	   * static
	//     * thread local
	std::vector<MCFunction*> functions;

	MCModule(std::string id): name{id} {}
};
// end of stuff to move

class MDIRGen {
public:
	MDIRGen(CompileOptions, Module*);

	void compile();
	
	std::vector<byte> emit_raw_bin();
	BinaryFile* emit_bin();

private:
	CompileOptions options;
	Module* module;
	MCModule* machine_module;

	MCModule* gen_module();
	MCFunction* gen_function(Function*);
	void gen_inst(MCFunction*, IRInst);
	void gen_imm(MCFunction*, IRInst);
	void gen_mov(MCFunction*, IRInst);
	void gen_bin(MCFunction*, IRInst);
	void gen_branch(MCFunction*, IRInst);
	void gen_call(MCFunction*, IRInst);
	void gen_ret(MCFunction*, IRInst);

	void append_inst(MCFunction*, MCInst);
};

} // namespace jab::x86_64

#endif // JAB_X86_64_MDIR_GEN_H
