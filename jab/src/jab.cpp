// TODO break this file up into a bunch of seperate files
// implements:
//	 Module
//   Function
//	 BasicBlock
//	 IRInst
//	 IRValue

#include "jab.h"

using namespace jab;


// start of Module impl

Module::Module(std::string id): name{id} {

}

// start of Function impl

Function::Function(std::string name, std::vector<Type> param_types, Type ret_type, CallConv callconv):
	id{name},
	params{},
	ret{},
	callconv{callconv}
{
	int ssa = 0;
	for(auto t: param_types) {
		params.push_back(IRValue(t, ssa++));
	}

	ret = IRValue(ret_type, ssa);
}

IRValue Function::param(int index) {
	return params[index];
}

// start of BasicBlock impl

BasicBlock::BasicBlock(std::string name):
	id{name},
	preds{},
	params{},
	insts{}
{

}

// start of IRInst impl

IRInst::IRInst() {

}

IRInst::IRInst(IROp op, IRValue dest): op{op}, dest{dest} {

}

IRInst::IRInst(IROp op, i32 dest, IRValue src1, IRValue src2):
	op{op},
	dest{IRValueKind::vreg, Type::i32, dest},
	src1{src1},
	src2{src2}
{

}

IRInst::IRInst(IROp op, IRValue dest, IRValue src1, IRValue src2):
	op{op},
	dest{dest},
	src1{src1},
	src2{src2}
{

}

IRInst::IRInst(IROp op, i32 dest, Function* fn, std::vector<IRValue> params):
	op{op},
	dest{IRValueKind::vreg, Type::i32, dest},
	fn{fn},
	src2{},
	params{params}
{

}

IRInst::IRInst(IROp op, IRValue dest, Function* fn, std::vector<IRValue> params):
	op{op},
	dest{},
	fn{fn},
	src2{},
	params{params}
{

}

// start of IRValue impl

IRValue::IRValue():
	kind{IRValueKind::none},
	type{Type::none},
	vreg{}
{

}

IRValue::IRValue(i64 imm):
	kind{IRValueKind::imm},
	type{Type::i64},
	imm{imm}
{

}

IRValue::IRValue(Type type):
	kind{IRValueKind::vreg},
	type{type},
	vreg{}
{

}

IRValue::IRValue(Type type, i32 vreg):
	kind{IRValueKind::vreg},
	type{type},
	vreg{VReg{vreg}}
{

}

IRValue::IRValue(Type type, HReg hreg):
	kind{IRValueKind::hreg},
	type{type},
	hreg{hreg}
{

}

IRValue::IRValue(IRValueKind kind, Type type, int num):
	kind{kind},
	type{type}
{
	switch(kind) {
		case IRValueKind::vreg:
			vreg = VReg{num};
			break;
		case IRValueKind::hreg:
			hreg = HReg{num};
			break;
		default:
			unreachable
	}
}
