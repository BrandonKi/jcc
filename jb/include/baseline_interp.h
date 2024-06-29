#pragma once

#include "jb.h"

#include <unordered_map>

namespace jb {

struct InterpValue {
    Type type;

    union {
        i8 v_i8;
        i16 v_i16;
        i32 v_i32;
        i64 v_i64;
        u8 v_u8;
        u16 v_u16;
        u32 v_u32;
        u64 v_u64;
        f32 v_f32;
        f64 v_f64;
        i64 v_ptr;
    };

    InterpValue() : type{Type::none}, v_i8{0} {}
    InterpValue(Type type) : type{type}, v_i8{0} {}
    InterpValue(i64 v) : type{Type::i64}, v_i64{v} {} // TODO remove
    InterpValue(f64 f) : type{Type::f64}, v_f64{f} {} // TODO remove
    InterpValue(Type t, IRConstantInt i) : type{t} {
        v_i64 = i.val;
    }
    InterpValue(Type t, IRConstantFloat f) : type{t} {
        v_f64 = f.val;
    }
    InterpValue(Type t, i64 ptr) : type{t} {
        v_ptr = ptr;
    }
};

class Interp {
public:
    Interp(CompileOptions, Module *);

    i32 run();
    InterpValue run_fn(Function *, std::vector<InterpValue>);

private:
    Module *module;
    std::unordered_map<std::string, Function *> symtab;
    std::unordered_map<std::string, BasicBlock *> labels;
    std::unordered_map<int, InterpValue> vregs;
    Function *fn_ptr;
    InterpValue fn_ret;
    BasicBlock *bb_ptr;
    BasicBlock *prev_bb;
    i64 i_ptr;

    std::vector<InterpValue> stack;

    InterpValue eval(IRValue val);

#define X(o) InterpValue run_inst_##o(IRInst);
#include "jbir_ops.inc"
#undef X
};

} // namespace jb
