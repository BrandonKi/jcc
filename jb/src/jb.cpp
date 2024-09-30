// TODO break this file up into a bunch of seperate files
// implements:
//	 Module
//   Function
//	 BasicBlock
//	 IRInst
//	 IRValue

#include "jb.h"

#include "pretty_print.h"

using namespace jb;

// start of Module impl

Module::Module(std::string id) : name{id} {}

void Module::print() {
    pretty_print(this);
}

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

BasicBlock::BasicBlock(std::string name) : id{name}, preds{}, params{}, insts{}, liveout{} {}

// start of IRInst impl

IRInst::IRInst() {}

IRInst::IRInst(IROp op, IRValue dest) : op{op}, dest{dest} {}

IRInst::IRInst(IROp op, i32 dest, IRValue src1,
               IRValue src2) // HACK fix the dest types
    : op{op}, dest{IRValueKind::vreg, std::max(src1.type, src2.type), dest}, src1{src1}, src2{src2} {}

IRInst::IRInst(IROp op, IRValue dest, IRValue src1) : op{op}, dest{dest}, src1{src1}, src2{} {}

IRInst::IRInst(IROp op, IRValue dest, IRValue src1, IRValue src2) : op{op}, dest{dest}, src1{src1}, src2{src2} {}

IRInst::IRInst(IROp op, BasicBlock *bb) : op{op}, dest{bb}, src1{}, src2{} {}

IRInst::IRInst(IROp op, IRValue cond, BasicBlock *bb1, BasicBlock *bb2) : op{op}, dest{cond}, src1{bb1}, src2{bb2} {}

IRInst::IRInst(IROp op, i32 dest, Function *fn, std::vector<IRValue> params)
    : op{op}, dest{IRValueKind::vreg, fn->ret.type, dest}, src1{fn}, src2{}, params{params} {}

IRInst::IRInst(IROp op, IRValue dest, Function *fn, std::vector<IRValue> params)
    : op{op}, dest{}, src1{fn}, src2{}, params{params} {}

IRInst::IRInst(IROp op, i32 dest, std::vector<std::pair<BasicBlock *, IRValue>> values)
    : op{op}, dest{values[0].second.type, dest}, src1{}, src2{}, values{values} {}  // TODO fix type

// start of IRValue impl

IRValue::IRValue() : kind{IRValueKind::none}, type{Type::none}, vreg{} {}

IRValue::IRValue(Type type) : kind{IRValueKind::vreg}, type{type}, vreg{} {}

IRValue::IRValue(Type type, i32 vreg) : kind{IRValueKind::vreg}, type{type}, vreg{vreg} {}

// TODO, not i64, info is in the constant
IRValue::IRValue(IRConstantInt imm_int) : kind{IRValueKind::imm}, type{Type::i64}, imm_int{imm_int} {}

// TODO, not f64, info is in the constant
IRValue::IRValue(IRConstantFloat imm_float) : kind{IRValueKind::imm}, type{Type::f64}, imm_float{imm_float} {}

IRValue::IRValue(IRLabel lbl): kind{IRValueKind::lbl}, type{Type::none}, lbl{lbl} {}

IRValue::IRValue(IRValueKind kind, Type type, int num) : kind{kind}, type{type} {
    switch (kind) {
    case IRValueKind::vreg:
        vreg = num;
        break;
    default:
        assert(false);
    }
}

IRConstantInt::IRConstantInt() : val{0}, size{0} {}

IRConstantInt::IRConstantInt(i64 val, i8 size) : val{val}, size{size} {}

IRConstantFloat::IRConstantFloat() : val{0.0}, size{0} {}

IRConstantFloat::IRConstantFloat(f64 val, i8 size) : val{val}, size{size} {}

IRLabel::IRLabel() : kind{IRLabelKind::none}, bb{nullptr} {}

IRLabel::IRLabel(BasicBlock *bb) : kind{IRLabelKind::basic_block}, bb{bb} {}

IRLabel::IRLabel(Function *fn) : kind{IRLabelKind::function}, fn{fn} {}
