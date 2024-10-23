#pragma once

#include "jb.h"

namespace jb {

class ModuleBuilder {
public:
    ModuleBuilder(std::string);

    // void compile(CompileOptions options = {});

    Function *newFn(std::string, std::vector<Type>, Type, CallConv, bool make_entry = true);
    BasicBlock *newBB(std::string);
    BasicBlock *newBB(std::string, LoopInfo, i32);
    void setInsertPoint(BasicBlock *);

    std::vector<IRInst*>& insts();

    IRValue addInst(IROp, IRValue, Type);
    IRValue addInst(IROp, IRValue, IRValue);
    IRValue addInst(IROp, IRValue);
    IRValue addInst(IROp, BasicBlock *);
    IRValue addInst(IROp, IRValue, BasicBlock *, BasicBlock *);
    IRValue addInst(IROp, Function *, std::vector<IRValue>);
    IRValue addInst(IROp, std::vector<std::pair<BasicBlock *, IRValue>>);

    IRConstantInt iconst8(i8);
    IRConstantInt iconst16(i16);
    IRConstantInt iconst32(i32);
    IRConstantInt iconst64(i64);
    IRConstantFloat fconst32(float);
    IRConstantFloat fconst64(double);

    IRValue none();

    IRValue mov(IRValue);

    IRValue zx(IRValue);
    IRValue sx(IRValue);
    IRValue f2i(IRValue);
    IRValue i2f(IRValue);

    IRValue iadd(IRValue, IRValue);
    IRValue isub(IRValue, IRValue);
    IRValue imul(IRValue, IRValue);
    IRValue idiv(IRValue, IRValue);
    IRValue imod(IRValue, IRValue);

    IRValue fadd(IRValue, IRValue);
    IRValue fsub(IRValue, IRValue);
    IRValue fmul(IRValue, IRValue);
    IRValue fdiv(IRValue, IRValue);
    IRValue fmod(IRValue, IRValue);

    IRValue lt(IRValue, IRValue);
    IRValue lte(IRValue, IRValue);
    IRValue gt(IRValue, IRValue);
    IRValue gte(IRValue, IRValue);
    IRValue eq(IRValue, IRValue);
    IRValue neq(IRValue, IRValue);

    IRValue bsl(IRValue, IRValue);
    IRValue bsr(IRValue, IRValue);
    IRValue band(IRValue, IRValue);
    IRValue bor(IRValue, IRValue);
    IRValue bxor(IRValue, IRValue);

    IRValue br(BasicBlock *bb);
    IRValue brz(IRValue cond, BasicBlock *bb1, BasicBlock *bb2);
    IRValue brnz(IRValue cond, BasicBlock *bb1, BasicBlock *bb2);
    IRValue call(Function *, std::vector<IRValue>);
    IRValue ret(IRValue);

    IRValue slot(IRValue);
    IRValue stack_store(IRValue, IRValue);
    IRValue stack_load(IRValue, Type);
    IRValue store(IRValue, IRValue);
    IRValue load(IRValue, Type);

    IRValue phi(std::vector<std::pair<BasicBlock *, IRValue>>);
    
    IRValue id(IRValue);

    Module *module;
    Function *function;
    BasicBlock *insert_point;

private:
    i32 ssa;

    i32 next_ssa();
};

} // namespace jb
