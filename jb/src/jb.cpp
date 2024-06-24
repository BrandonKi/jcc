// TODO break this file up into a bunch of seperate files
// implements:
//	 Module
//   Function
//	 BasicBlock
//	 IRInst
//	 IRValue

#include "jb.h"

using namespace jb;

// start of Module impl

Module::Module(std::string id) : name{id} {}

// start of Function impl

Function::Function(std::string name, std::vector<Type> param_types, Type ret_type, CallConv callconv)
    : id{name}, params{}, ret{}, callconv{callconv} {
    i32 vreg_id = 0;
    for (auto t : param_types) {
        params.push_back(IRValue(t, vreg_id++));
    }

    ret = IRValue(ret_type, vreg_id);
}

IRValue Function::param(int index) {
    return params[index];
}

// start of BasicBlock impl

BasicBlock::BasicBlock(std::string name) : id{name}, preds{}, params{}, insts{} {}

// start of IRInst impl

IRInst::IRInst() {}

IRInst::IRInst(IROp op, IRValue dest) : op{op}, dest{dest} {}

IRInst::IRInst(IROp op, i32 dest, IRValue src1,
               IRValue src2) // HACK fix the dest types
    : op{op}, dest{IRValueKind::vreg, std::max(src1.type, src2.type), dest}, src1{src1}, src2{src2} {}

IRInst::IRInst(IROp op, IRValue dest, IRValue src1, IRValue src2) : op{op}, dest{dest}, src1{src1}, src2{src2} {}

IRInst::IRInst(IROp op, BasicBlock *bb) : op{op}, dest{}, bb{bb}, src2{} {}

IRInst::IRInst(IROp op, IRValue cond, BasicBlock *bb) : op{op}, dest{}, bb{bb}, src2{cond} {}

IRInst::IRInst(IROp op, i32 dest, Function *fn, std::vector<IRValue> params)
    : op{op}, dest{IRValueKind::vreg, fn->ret.type, dest}, fn{fn}, src2{}, params{params} {}

IRInst::IRInst(IROp op, IRValue dest, Function *fn, std::vector<IRValue> params)
    : op{op}, dest{}, fn{fn}, src2{}, params{params} {}

IRInst::IRInst(IROp op, IRValue dest, std::vector<std::pair<BasicBlock *, IRValue>> values)
    : op{op}, dest{dest}, src1{}, src2{}, values{values} {}

// start of IRValue impl

IRValue::IRValue() : kind{IRValueKind::none}, type{Type::none}, vreg{} {}

IRValue::IRValue(i64 imm) : kind{IRValueKind::imm}, type{Type::i64}, imm{imm} {}

IRValue::IRValue(Type type) : kind{IRValueKind::vreg}, type{type}, vreg{} {}

IRValue::IRValue(Type type, i32 vreg) : kind{IRValueKind::vreg}, type{type}, vreg{vreg} {}

IRValue::IRValue(IRValueKind kind, Type type, int num) : kind{kind}, type{type} {
    switch (kind) {
    case IRValueKind::vreg:
        vreg = num;
        break;
    default:
        assert(false);
    }
}
