#pragma once

#include "jb.h"
#include "arch/x86_64/mcir.h"

namespace jb::x86_64 {

class MCIRGen {
public:
    MCIRGen(CompileOptions, Module *);

    void compile();

    std::vector<byte> emit_raw_bin();
    BinaryFile *emit_bin();

    CompileOptions options;
    Module *module;
    MCModule *machine_module;

private:
    MCModule *gen_module();
    MCFunction *gen_function(Function *);
    void gen_inst(MCFunction *, IRInst);
    void gen_imm(MCFunction *, IRInst);
    void gen_mov(MCFunction *, IRInst);
    void gen_bin(MCFunction *, IRInst);
    void gen_branch(MCFunction *, IRInst);
    void gen_call(MCFunction *, IRInst);
    void gen_ret(MCFunction *, IRInst);

    void append_inst(MCFunction *, MCInst);
};

} // namespace jb::x86_64
