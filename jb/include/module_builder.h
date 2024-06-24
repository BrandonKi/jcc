#pragma once

#include "jb.h"

namespace jb {

class ModuleBuilder {
public:
    ModuleBuilder(std::string);

    // void compile(CompileOptions options = {});

    Function *newFn(std::string, std::vector<Type>, Type, CallConv);
    BasicBlock *newBB(std::string);
    void setInsertPoint(BasicBlock *);

    IRValue addInst(IROp, IRValue, IRValue);
    IRValue addInst(IROp, IRValue);
    IRValue addInst(IROp, BasicBlock *);
    IRValue addInst(IROp, IRValue, BasicBlock *);
    IRValue addInst(IROp, Function *, std::vector<IRValue>);
    IRValue addInst(IROp, std::vector<std::pair<BasicBlock *, IRValue>>);

    IRValue none();
    IRValue iconst8(IRValue);
    IRValue iconst16(IRValue);
    IRValue iconst32(IRValue);
    IRValue iconst64(IRValue);
    IRValue fconst32(IRValue);
    IRValue fconst64(IRValue);

    IRValue mov(IRValue);

    IRValue addi(IRValue, IRValue);
    IRValue subi(IRValue, IRValue);
    IRValue muli(IRValue, IRValue);
    IRValue divi(IRValue, IRValue);
    IRValue modi(IRValue, IRValue);

    IRValue addf(IRValue, IRValue);
    IRValue subf(IRValue, IRValue);
    IRValue mulf(IRValue, IRValue);
    IRValue divf(IRValue, IRValue);
    IRValue modf(IRValue, IRValue);

    IRValue lt(IRValue, IRValue);
    IRValue lte(IRValue, IRValue);
    IRValue gt(IRValue, IRValue);
    IRValue gte(IRValue, IRValue);
    IRValue eq(IRValue, IRValue);

    IRValue br(BasicBlock *bb);
    IRValue brz(IRValue cond, BasicBlock *bb);
    IRValue brnz(IRValue cond, BasicBlock *bb);
    IRValue call(Function *, std::vector<IRValue>);
    IRValue ret(IRValue);

    IRValue salloc(IRValue);
    IRValue store(IRValue, IRValue);
    IRValue load(IRValue, IRValue);

    IRValue phi(std::vector<std::pair<BasicBlock *, IRValue>>);

    Module *module;
    Function *function;

private:
    BasicBlock *insert_point;
    i32 ssa;

    i32 next_ssa();
};

} // namespace jb
