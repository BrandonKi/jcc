#include "baseline_interp.h"

using namespace jb;

Interp::Interp(CompileOptions options, Module *module) : module{module} {}

i32 Interp::run() {
    for (auto *fn : module->functions) {
        symtab[fn->id] = fn;
    }

    auto *main_fn = symtab["main"];
    i32 res = run_fn(main_fn, {}).v_i32;
    return res;
}

InterpValue Interp::run_fn(Function *fn, std::vector<InterpValue> params) {
    fn_ptr = fn;
    labels.clear();
    vregs.clear();

    for (int i = 0; i < params.size(); ++i) {
        vregs[i] = params[i];
    }

    for (auto *bb : fn->blocks) {
        labels[bb->id] = bb;
    }

    prev_bb = nullptr;
    bb_ptr = fn->blocks[0];
    i_ptr = 0;
    while (fn_ret.type == Type::none) {
        IRInst inst = bb_ptr->insts[i_ptr];
        switch ((IROp)inst.op) {
#define X(o)                                                                                                           \
    case IROp::o:                                                                                                      \
        run_inst_##o(inst);                                                                                            \
        break;
#include "jbir_ops.inc"
#undef X
        default:
            assert(false);
        }

        i_ptr += 1;
    }

    return fn_ret;
}

InterpValue Interp::eval(IRValue val) {
    switch (val.kind) {
    case IRValueKind::none:
        assert(false);
    case IRValueKind::vreg:
        return vregs[val.vreg];
    case IRValueKind::imm:
        if (is_int(val.type))
            return InterpValue(val.type, val.imm_int);
        if (is_float(val.type))
            return InterpValue(val.type, val.imm_float);
    case IRValueKind::lbl:
        assert(false);
    default:
        assert(false);
    }

    return {};
}

InterpValue Interp::run_inst_none(IRInst inst) {
    assert(false);
    return {};
}

InterpValue Interp::run_inst_mov(IRInst inst) {
    vregs[inst.dest.vreg] = eval(inst.src1);
    return {};
}

InterpValue Interp::run_inst_zx(IRInst inst) {
    vregs[inst.dest.vreg] = eval(inst.src1);
    return {};
}

InterpValue Interp::run_inst_sx(IRInst inst) {
    vregs[inst.dest.vreg] = eval(inst.src1);
    return {};
}

InterpValue Interp::run_inst_f2i(IRInst inst) {
    auto dest = eval(inst.src1);
    dest.type = Type::i64;
    dest.v_i64 = (i64)dest.v_f64;
    vregs[inst.dest.vreg] = dest;
    return {};
}

InterpValue Interp::run_inst_i2f(IRInst inst) {
    auto dest = eval(inst.src1);
    dest.type = Type::f64;
    dest.v_f64 = (f64)dest.v_i64;
    vregs[inst.dest.vreg] = dest;
    return {};
}

#define BINOP(s1, op, s2)                                                                                              \
    auto &dest = vregs[inst.dest.vreg];                                                                                \
    Type t = std::max(s1.type, s2.type);                                                                               \
    dest = InterpValue((i64)(s1.v_i64 op s2.v_i64));

#define BINOP_FLOAT(s1, op, s2)                                                                                        \
    auto &dest = vregs[inst.dest.vreg];                                                                                \
    Type t = std::max(s1.type, s2.type);                                                                               \
    dest = InterpValue(s1.v_f64 op s2.v_f64);

InterpValue Interp::run_inst_iadd(IRInst inst) {
    BINOP(eval(inst.src1), +, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_isub(IRInst inst) {
    BINOP(eval(inst.src1), -, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_imul(IRInst inst) {
    BINOP(eval(inst.src1), *, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_idiv(IRInst inst) {
    BINOP(eval(inst.src1), /, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_imod(IRInst inst) {
    BINOP(eval(inst.src1), %, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_fadd(IRInst inst) {
    BINOP_FLOAT(eval(inst.src1), +, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_fsub(IRInst inst) {
    BINOP_FLOAT(eval(inst.src1), -, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_fmul(IRInst inst) {
    BINOP_FLOAT(eval(inst.src1), *, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_fdiv(IRInst inst) {
    BINOP_FLOAT(eval(inst.src1), /, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_lt(IRInst inst) {
    BINOP(eval(inst.src1), <, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_lte(IRInst inst) {
    BINOP(eval(inst.src1), <=, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_gt(IRInst inst) {
    BINOP(eval(inst.src1), >, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_gte(IRInst inst) {
    BINOP(eval(inst.src1), >=, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_eq(IRInst inst) {
    BINOP(eval(inst.src1), ==, eval(inst.src2))
    return {};
}

InterpValue Interp::run_inst_br(IRInst inst) {
    prev_bb = bb_ptr;
    bb_ptr = inst.src1.lbl.bb;
    i_ptr = -1;
    return {};
}

InterpValue Interp::run_inst_brz(IRInst inst) {
    return {};
}

InterpValue Interp::run_inst_brnz(IRInst inst) {
    return {};
}

// TODO push/pop_state functions
InterpValue Interp::run_inst_call(IRInst inst) {
    auto old_labels = labels;
    auto old_vregs = vregs;
    auto old_fn_ptr = fn_ptr;
    auto old_bb_ptr = bb_ptr;
    auto old_prev_bb = prev_bb;
    auto old_i_ptr = i_ptr;

    std::vector<InterpValue> params;
    for (auto &p : inst.params)
        params.push_back(eval(p));
    vregs[inst.dest.vreg] = run_fn(inst.src1.lbl.fn, params);

    labels = old_labels;
    vregs = old_vregs;
    fn_ptr = old_fn_ptr;
    bb_ptr = old_bb_ptr;
    prev_bb = old_prev_bb;
    i_ptr = old_i_ptr;

    return {};
}

InterpValue Interp::run_inst_ret(IRInst inst) {

    fn_ret = eval(inst.src1);

    return {};
}

InterpValue Interp::run_inst_slot(IRInst inst) {
    stack.push_back(InterpValue(inst.src1.type));
    vregs[inst.dest.vreg] = InterpValue(Type::ptr, stack.size() - 1);
    return {};
}

InterpValue Interp::run_inst_store(IRInst inst) {
    stack[vregs[inst.src1.vreg].v_ptr] = eval(inst.src2);
    return {};
}

InterpValue Interp::run_inst_load(IRInst inst) {
    vregs[inst.dest.vreg] = stack[eval(inst.src1).v_ptr];
    return {};
}

InterpValue Interp::run_inst_phi(IRInst inst) {
    for (auto &[bb, val] : inst.values) {
        if (bb == prev_bb) {
            vregs[inst.dest.vreg] = eval(val);
            break;
        }
    }
    return {};
}
