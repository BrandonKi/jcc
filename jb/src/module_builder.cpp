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

Function *ModuleBuilder::newFn(std::string name, std::vector<Type> parameters, Type ret, CallConv callconv) {
    auto *fn = new Function(name, parameters, ret, callconv);
    module->functions.push_back(fn);

    // virtual registers allocated for params and ret
    // .size() adds the extra 1 needed for the ret vreg
    ssa += (i32)parameters.size();

    function = fn;
    auto *bb = newBB(name + "_" + std::to_string(fn->blocks.size()));

    return fn;
}

BasicBlock *ModuleBuilder::newBB(std::string name) {
    auto *bb = new BasicBlock(name);
    function->blocks.push_back(bb);
    insert_point = bb;
    return bb;
}

void ModuleBuilder::setInsertPoint(BasicBlock *bb) {
    insert_point = bb;
}

IRValue ModuleBuilder::addInst(IROp op, IRValue src1, IRValue src2) {
    IRInst inst;
    if (has_dest(op))
        inst = IRInst(op, next_ssa(), src1, src2);
    else
        inst = IRInst(op, {}, src1, src2);

    insert_point->insts.push_back(inst);
    return inst.dest;
}

IRValue ModuleBuilder::addInst(IROp op, IRValue src) {
    IRInst inst;
    if (has_dest(op))
        inst = IRInst(op, next_ssa(), src, {});
    else
        inst = IRInst(op, {}, src, {});

    insert_point->insts.push_back(inst);
    return inst.dest;
}

IRValue ModuleBuilder::addInst(IROp op, BasicBlock *bb) {
    IRInst inst = IRInst(op, bb);

    insert_point->insts.push_back(inst);
    return inst.dest;
}

IRValue ModuleBuilder::addInst(IROp op, IRValue cond, BasicBlock *bb) {
    IRInst inst = IRInst(op, cond, bb);

    insert_point->insts.push_back(inst);
    return inst.dest;
}

IRValue ModuleBuilder::addInst(IROp op, Function *fn, std::vector<IRValue> params) {
    IRInst inst;
    if (fn->ret.kind != IRValueKind::none)
        inst = IRInst(op, next_ssa(), fn, params);
    else
        inst = IRInst(op, {}, fn, params);

    insert_point->insts.push_back(inst);
    return inst.dest;
}

IRValue ModuleBuilder::addInst(IROp op, std::vector<std::pair<BasicBlock *, IRValue>> values) {
    IRInst inst;
    inst = IRInst(op, next_ssa(), values);

    insert_point->insts.push_back(inst);
    return inst.dest;
}

i32 ModuleBuilder::next_ssa() {
    return ++ssa;
}

// make this no-op instead of doing nothing
IRValue ModuleBuilder::none() {
    return {};
}

IRValue ModuleBuilder::iconst8(IRValue src) {
    return addInst(IROp::iconst8, src);
}

IRValue ModuleBuilder::iconst16(IRValue src) {
    return addInst(IROp::iconst16, src);
}

IRValue ModuleBuilder::iconst32(IRValue src) {
    return addInst(IROp::iconst32, src);
}

IRValue ModuleBuilder::iconst64(IRValue src) {
    return addInst(IROp::iconst64, src);
}

IRValue ModuleBuilder::fconst32(IRValue src) {
    return addInst(IROp::fconst32, src);
}

IRValue ModuleBuilder::fconst64(IRValue src) {
    return addInst(IROp::fconst64, src);
}

IRValue ModuleBuilder::mov(IRValue src) {
    return addInst(IROp::mov, src);
}

IRValue ModuleBuilder::addi(IRValue src1, IRValue src2) {
    return addInst(IROp::addi, src1, src2);
}

IRValue ModuleBuilder::subi(IRValue src1, IRValue src2) {
    return addInst(IROp::subi, src1, src2);
}

IRValue ModuleBuilder::muli(IRValue src1, IRValue src2) {
    return addInst(IROp::muli, src1, src2);
}

IRValue ModuleBuilder::divi(IRValue src1, IRValue src2) {
    return addInst(IROp::divi, src1, src2);
}

IRValue ModuleBuilder::modi(IRValue src1, IRValue src2) {
    return addInst(IROp::modi, src1, src2);
}

IRValue ModuleBuilder::addf(IRValue src1, IRValue src2) {
    return addInst(IROp::addf, src1, src2);
}

IRValue ModuleBuilder::subf(IRValue src1, IRValue src2) {
    return addInst(IROp::subf, src1, src2);
}

IRValue ModuleBuilder::mulf(IRValue src1, IRValue src2) {
    return addInst(IROp::mulf, src1, src2);
}

IRValue ModuleBuilder::divf(IRValue src1, IRValue src2) {
    return addInst(IROp::divf, src1, src2);
}

IRValue ModuleBuilder::modf(IRValue src1, IRValue src2) {
    return addInst(IROp::modf, src1, src2);
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

IRValue ModuleBuilder::br(BasicBlock *bb) {
    return addInst(IROp::br, bb);
}

IRValue ModuleBuilder::brz(IRValue cond, BasicBlock *bb) {
    return addInst(IROp::brz, cond, bb);
}

IRValue ModuleBuilder::brnz(IRValue cond, BasicBlock *bb) {
    return addInst(IROp::brz, cond, bb);
}

IRValue ModuleBuilder::call(Function *fn, std::vector<IRValue> params) {
    return addInst(IROp::call, fn, params);
}

IRValue ModuleBuilder::ret(IRValue src) {
    return addInst(IROp::ret, src);
}

IRValue ModuleBuilder::salloc(IRValue src) {
    return addInst(IROp::salloc, src);
}

IRValue ModuleBuilder::store(IRValue src, IRValue dest) {
    return addInst(IROp::store, src, dest);
}

IRValue ModuleBuilder::load(IRValue src, IRValue dest) {
    return addInst(IROp::load, src, dest);
}

IRValue ModuleBuilder::phi(std::vector<std::pair<BasicBlock *, IRValue>> values) {
    return addInst(IROp::phi, values);
}
