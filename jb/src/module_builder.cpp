#include "module_builder.h"

#include "arch/x86_64/mcir_gen.h"

using namespace jb;

ModuleBuilder::ModuleBuilder(std::string name)
    : module{new Module(name)}, function{nullptr}, insert_point{nullptr}, ssa{0} {}

/*
MachineModule* ModuleBuilder::compile(CompileOptions options) {

        if(options.target_arch == Arch::x64) {
                x86_64::MCIRGen mcir_gen(module);
                return mcir_gen.compile();
        }

        return new MachineModule();
}
*/

Function *ModuleBuilder::newFn(std::string name, std::vector<Type> parameters, Type ret, CallConv callconv,
                               bool make_entry) {
    auto *fn = new Function(name, parameters, ret, callconv);
    module->functions.push_back(fn);

    // virtual registers allocated for params and ret
    // .size() adds the extra 1 needed for the ret vreg
    ssa += (i32)parameters.size();

    function = fn;
    if (make_entry)
        newBB(name + "_" + std::to_string(fn->blocks.size()));

    return fn;
}

BasicBlock *ModuleBuilder::newBB(std::string name) {
    bool done = false;
    while(!done) { // FIXME
        done = true;
        for(auto *bb: function->blocks) {
            if(name == bb->id) {
                name += '_';
                done = false;
                break;
            }
        }
    }
    auto *bb = new BasicBlock(name);
    function->blocks.push_back(bb);
    insert_point = bb;
    return bb;
}

BasicBlock *ModuleBuilder::newBB(std::string name, LoopInfo loop_info, i32 loop_id) {
    BasicBlock *bb = newBB(name);
    bb->loop_info = loop_info;
    bb->loop_id = loop_id;

    return bb;
}

void ModuleBuilder::setInsertPoint(BasicBlock *bb) {
    insert_point = bb;
}

std::vector<IRInst*>& ModuleBuilder::insts() {
    return insert_point->insts;
}

IRValue ModuleBuilder::addInst(IROp op, IRValue src1, Type type) {
    IRInst *inst;
    if (has_dest(op))
        inst = new IRInst(op, next_ssa(), src1, {});
    else
        inst = new IRInst(op, {}, src1, {});
    inst->type = type;
    insert_point->insts.push_back(inst);
    return inst->dest;
}

IRValue ModuleBuilder::addInst(IROp op, IRValue src1, IRValue src2) {
    IRInst *inst;
    if (has_dest(op))
        inst = new IRInst(op, next_ssa(), src1, src2);
    else
        inst = new IRInst(op, {}, src1, src2);

    insert_point->insts.push_back(inst);
    return inst->dest;
}

IRValue ModuleBuilder::addInst(IROp op, IRValue src) {
    IRInst *inst;
    if (has_dest(op))
        inst = new IRInst(op, next_ssa(), src, {});
    else
        inst = new IRInst(op, {}, src, {});

    insert_point->insts.push_back(inst);
    return inst->dest;
}

IRValue ModuleBuilder::addInst(IROp op, BasicBlock *bb) {
    IRInst *inst = new IRInst(op, bb);

    insert_point->insts.push_back(inst);
    return inst->dest;
}

IRValue ModuleBuilder::addInst(IROp op, IRValue cond, BasicBlock *bb1, BasicBlock *bb2) {
    IRInst *inst = new IRInst(op, cond, bb1, bb2);

    insert_point->insts.push_back(inst);
    return inst->dest;
}

IRValue ModuleBuilder::addInst(IROp op, Function *fn, std::vector<IRValue> params) {
    IRInst *inst;
    if (fn->ret.kind != IRValueKind::none)
        inst = new IRInst(op, next_ssa(), fn, params);
    else
        inst = new IRInst(op, {}, fn, params);

    insert_point->insts.push_back(inst);
    return inst->dest;
}

IRValue ModuleBuilder::addInst(IROp op, std::vector<std::pair<BasicBlock *, IRValue>> values) {
    IRInst *inst;
    inst = new IRInst(op, next_ssa(), values);

    insert_point->insts.push_back(inst);
    return inst->dest;
}

i32 ModuleBuilder::next_ssa() {
    return ++ssa;
}

// make this no-op instead of doing nothing
IRValue ModuleBuilder::none() {
    return {};
}

IRConstantInt ModuleBuilder::iconst8(i8 val) {
    return IRConstantInt(val, 8);
}

IRConstantInt ModuleBuilder::iconst16(i16 val) {
    return IRConstantInt(val, 16);
}

IRConstantInt ModuleBuilder::iconst32(i32 val) {
    return IRConstantInt(val, 32);
}

IRConstantInt ModuleBuilder::iconst64(i64 val) {
    return IRConstantInt(val, 64);
}

IRConstantFloat ModuleBuilder::fconst32(float val) {
    return IRConstantFloat(val, 32);
}

IRConstantFloat ModuleBuilder::fconst64(double val) {
    return IRConstantFloat(val, 64);
}

IRValue ModuleBuilder::mov(IRValue src) {
    return addInst(IROp::mov, src);
}

IRValue ModuleBuilder::zx(IRValue src) {
    return addInst(IROp::zx, src);
}

IRValue ModuleBuilder::sx(IRValue src) {
    return addInst(IROp::sx, src);
}

IRValue ModuleBuilder::f2i(IRValue src) {
    return addInst(IROp::f2i, src);
}

IRValue ModuleBuilder::i2f(IRValue src) {
    return addInst(IROp::i2f, src);
}

IRValue ModuleBuilder::iadd(IRValue src1, IRValue src2) {
    return addInst(IROp::iadd, src1, src2);
}

IRValue ModuleBuilder::isub(IRValue src1, IRValue src2) {
    return addInst(IROp::isub, src1, src2);
}

IRValue ModuleBuilder::imul(IRValue src1, IRValue src2) {
    return addInst(IROp::imul, src1, src2);
}

IRValue ModuleBuilder::idiv(IRValue src1, IRValue src2) {
    return addInst(IROp::idiv, src1, src2);
}

IRValue ModuleBuilder::imod(IRValue src1, IRValue src2) {
    return addInst(IROp::imod, src1, src2);
}

IRValue ModuleBuilder::fadd(IRValue src1, IRValue src2) {
    return addInst(IROp::fadd, src1, src2);
}

IRValue ModuleBuilder::fsub(IRValue src1, IRValue src2) {
    return addInst(IROp::fsub, src1, src2);
}

IRValue ModuleBuilder::fmul(IRValue src1, IRValue src2) {
    return addInst(IROp::fmul, src1, src2);
}

IRValue ModuleBuilder::fdiv(IRValue src1, IRValue src2) {
    return addInst(IROp::fdiv, src1, src2);
}

IRValue ModuleBuilder::lt(IRValue src1, IRValue src2) {
    return addInst(IROp::lt, src1, src2);
}

IRValue ModuleBuilder::lte(IRValue src1, IRValue src2) {
    return addInst(IROp::lte, src1, src2);
}

IRValue ModuleBuilder::gt(IRValue src1, IRValue src2) {
    return addInst(IROp::gt, src1, src2);
}

IRValue ModuleBuilder::gte(IRValue src1, IRValue src2) {
    return addInst(IROp::gte, src1, src2);
}

IRValue ModuleBuilder::eq(IRValue src1, IRValue src2) {
    return addInst(IROp::eq, src1, src2);
}

IRValue ModuleBuilder::neq(IRValue src1, IRValue src2) {
    return addInst(IROp::neq, src1, src2);
}

IRValue ModuleBuilder::bsl(IRValue src1, IRValue src2) {
    return addInst(IROp::bsl, src1, src2);
}

IRValue ModuleBuilder::bsr(IRValue src1, IRValue src2) {
    return addInst(IROp::bsr, src1, src2);
}

IRValue ModuleBuilder::band(IRValue src1, IRValue src2) {
    return addInst(IROp::band, src1, src2);
}

IRValue ModuleBuilder::bor(IRValue src1, IRValue src2) {
    return addInst(IROp::bor, src1, src2);
}

IRValue ModuleBuilder::bxor(IRValue src1, IRValue src2) {
    return addInst(IROp::bxor, src1, src2);
}

IRValue ModuleBuilder::br(BasicBlock *bb) {
    return addInst(IROp::br, bb);
}

IRValue ModuleBuilder::brz(IRValue cond, BasicBlock *bb1, BasicBlock *bb2) {
    return addInst(IROp::brz, cond, bb1, bb2);
}

IRValue ModuleBuilder::brnz(IRValue cond, BasicBlock *bb1, BasicBlock *bb2) {
    return addInst(IROp::brnz, cond, bb1, bb2);
}

IRValue ModuleBuilder::call(Function *fn, std::vector<IRValue> params) {
    return addInst(IROp::call, fn, params);
}

IRValue ModuleBuilder::ret(IRValue src) {
    return addInst(IROp::ret, src);
}

IRValue ModuleBuilder::slot(IRValue src) {
    return addInst(IROp::slot, src);
}

IRValue ModuleBuilder::stack_store(IRValue src1, IRValue src2) {
    return addInst(IROp::stack_store, src1, src2);
}

IRValue ModuleBuilder::stack_load(IRValue src, Type type) {
    return addInst(IROp::stack_load, src, type);
}

IRValue ModuleBuilder::store(IRValue src1, IRValue src2) {
    return addInst(IROp::store, src1, src2);
}

IRValue ModuleBuilder::load(IRValue src, Type type) {
    return addInst(IROp::load, src, type);
}

IRValue ModuleBuilder::phi(std::vector<std::pair<BasicBlock *, IRValue>> values) {
    return addInst(IROp::phi, values);
}

IRValue ModuleBuilder::id(IRValue value) {
    return addInst(IROp::id, value);
}