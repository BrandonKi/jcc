#include "JBIR/opt/sscp.h"
#include "pretty_print.h"

#include <iostream>
#include <algorithm>

using namespace jb;

// TODO use command line flag
constexpr auto SSCP_DEBUG = false;

#define TRY_FOLD_INT_BINOP(m_op) \
{ \
    if(inst->src1.kind == IRValueKind::imm && inst->src2.kind == IRValueKind::imm) { \
        inst->op = IROp::id; \
        auto lhs = inst->src1.imm_int.val; \
        auto rhs = inst->src2.imm_int.val; \
        inst->src1.imm_int.size = std::max(inst->src1.imm_int.size, inst->src2.imm_int.size); \
        inst->src1.imm_int.val = lhs m_op rhs; \
        return true; \
    } \
    break; \
}

enum class SSCPLat: i8 {
    top = 3,
    con = 2,
    bot = 1,
    invalid = 0,
};

static std::unordered_map<Reg, SSCPLat> lattice;
static std::unordered_map<Reg, i64> const_value;
static std::unordered_map<Reg, IRInst*> def;
static std::unordered_map<Reg, std::vector<IRInst*>> uses;

static bool is_const(IRInst *inst) {
    if(inst->op == IROp::id || inst->op == IROp::mov) {
        return inst->src1.kind == IRValueKind::imm;
    }
    return false;
}

static bool maybe_replace_use(IRInst *inst, Reg n) {
    bool removed = false;
    if(inst->op == IROp::brz || inst->op == IROp::brnz) {
        if(inst->dest_is_vreg() && inst->dest.vreg == n) {
            if(const_value.contains(inst->dest.vreg)) {
                i64 val = const_value[inst->dest.vreg];
                inst->dest.kind = IRValueKind::imm;
                inst->dest.imm_int.size = 64;
                inst->dest.imm_int.val = val;
                removed = true;
            }
        }
    }
    if(inst->op == IROp::phi) {
        for(auto &[b, v]: inst->values) {
            if(v.kind == IRValueKind::vreg && v.vreg == n) {
                if(const_value.contains(v.vreg)) {
                    v.kind = IRValueKind::imm;
                    v.imm_int.size = 64;
                    v.imm_int.val = const_value[v.vreg];
                    removed = true;
                }
            }
        }
    }
    if(inst->src1_is_vreg() && inst->src1.vreg == n) {
        if(const_value.contains(inst->src1.vreg)) {
            i64 val = const_value[inst->src1.vreg];
            inst->src1.kind = IRValueKind::imm;
            inst->src1.imm_int.size = 64;
            inst->src1.imm_int.val = val;
            removed = true;
        } else {
            removed = false;
        }
    }
    if(inst->src2_is_vreg() && inst->src2.vreg == n) {
        if(const_value.contains(inst->src2.vreg)) {
            i64 val = const_value[inst->src2.vreg];
            inst->src2.kind = IRValueKind::imm;
            inst->src2.imm_int.size = 64;
            inst->src2.imm_int.val = val;
            removed = true;
        } else {
            removed = false;
        }
    }
    return removed;
}

static bool maybe_fold_inst(IRInst *inst) {
    switch(inst->op) {
    case IROp::none:
    case IROp::noop:
        break;
    case IROp::mov:
        break;
    case IROp::zx:
    case IROp::sx:
        break;
    case IROp::f2i:
    case IROp::i2f:
        break;
    case IROp::iadd:
        TRY_FOLD_INT_BINOP(+)
    case IROp::isub:
        TRY_FOLD_INT_BINOP(-)
    case IROp::imul:
        TRY_FOLD_INT_BINOP(*)
    case IROp::idiv:
        TRY_FOLD_INT_BINOP(/)
    case IROp::imod: 
        TRY_FOLD_INT_BINOP(%)
    case IROp::fadd:
    case IROp::fsub:
    case IROp::fmul:
    case IROp::fdiv:
        break; // TODO
    case IROp::bsl:
        TRY_FOLD_INT_BINOP(<<)
    case IROp::bsr:
        TRY_FOLD_INT_BINOP(>>)
    case IROp::band:
        TRY_FOLD_INT_BINOP(&)
    case IROp::bor:
        TRY_FOLD_INT_BINOP(|)
    case IROp::bxor:
        TRY_FOLD_INT_BINOP(^)

    case IROp::lt:
        TRY_FOLD_INT_BINOP(<)
    case IROp::lte:
        TRY_FOLD_INT_BINOP(<=)
    case IROp::gt:
        TRY_FOLD_INT_BINOP(>)
    case IROp::gte:
        TRY_FOLD_INT_BINOP(>=)
    case IROp::eq:
        TRY_FOLD_INT_BINOP(==)
    case IROp::br:
        break;
    case IROp::brz:
        if(inst->dest.kind == IRValueKind::imm) {
            auto val = inst->dest.imm_int.val;
            inst->op = IROp::br;
            if(val != 0) {
                inst->dest = inst->src2;
            } else {
                inst->dest = inst->src1;
            }
            inst->src1 = {};
            inst->src2 = {};
        }
        break;
    case IROp::brnz:
        if(inst->dest.kind == IRValueKind::imm) {
            auto val = inst->dest.imm_int.val;
            inst->op = IROp::br;
            if(val == 0) {
                inst->dest = inst->src2;
            } else {
                inst->dest = inst->src1;
            }
            inst->src1 = {};
            inst->src2 = {};
        }
        break;
    case IROp::call:
    case IROp::ret:
    case IROp::slot:
    case IROp::stack_store:
    case IROp::stack_load:
    case IROp::store:
    case IROp::load:
        break;
    case IROp::phi:
        if(inst->values.size() == 1 && inst->values[0].second.kind == IRValueKind::imm) {
            inst->op = IROp::id;
            inst->src1 = inst->values[0].second;
            inst->values = {};
        }
        break;
    case IROp::id:
        break;
    default:
        assert(false);
    }
    
    return false;
}

static std::vector<Reg> get_uses(IRInst *inst) {
    std::vector<Reg> result;
    if(inst->op == IROp::brz || inst->op == IROp::brnz) {
        if(inst->dest_is_vreg()) {
            result.push_back(inst->dest.vreg);
        }
    } else if(inst->op == IROp::phi) {
        for(auto &[b, v]: inst->values) {
            if(v.kind == IRValueKind::vreg)
                result.push_back(v.vreg);
        }
    } else {
        if(inst->src1_is_vreg()) {
            result.push_back(inst->src1.vreg);
        }
        if(inst->src2_is_vreg()) {
            result.push_back(inst->src2.vreg);
        }
    }

    return result;
}

bool SSCP::run_pass(Function *function) {
    bool changed = false;
    std::vector<Reg> worklist;

    for(auto p: function->params) {
        if(p.kind == IRValueKind::vreg)
            lattice[p.vreg] = SSCPLat::bot;
    }
    
    for(auto *b: function->blocks) {
        for(auto *i: b->insts) {
            changed |= maybe_fold_inst(i);

            if(i->op == IROp::brz || i->op == IROp::brnz) {

            } else if(i->has_dest() && i->dest_is_vreg()) {
                def[i->dest.vreg] = i;
                uses[i->dest.vreg] = {};
                if(i->op == IROp::phi) {
                    lattice[i->dest.vreg] = SSCPLat::top;
                } else if(is_const(i)) {
                    lattice[i->dest.vreg] = SSCPLat::con;
                    const_value[i->dest.vreg] = i->src1.imm_int.val;
                    worklist.push_back(i->dest.vreg);
                } else if(i->op == IROp::load || i->op == IROp::stack_load) {
                    lattice[i->dest.vreg] = SSCPLat::bot;
                    worklist.push_back(i->dest.vreg);
                } else {
                    lattice[i->dest.vreg] = SSCPLat::top;
                }
            }

            auto us = get_uses(i);
            for(Reg u: us) {
                uses[u].push_back(i);
            }
        }
    }

    while(!worklist.empty()) {
        auto n = worklist.back();
        worklist.pop_back();

        int replaced = 0;
        for(auto &use: uses[n]) {
            if(use->has_dest() && use->dest_is_vreg()) {
                Reg m = use->dest.vreg;
                if(lattice[m] != SSCPLat::bot) {
                    IROp o = use->op;
                    if(maybe_replace_use(use, n))
                        replaced += 1;
                    changed |= maybe_fold_inst(use);
                    if(o != use->op) {
                        const_value[m] = use->src1.imm_int.val;
                        worklist.push_back(m);
                    }
                }
            } else if(is_ret(use->op)) {
                if(maybe_replace_use(use, n))
                    replaced += 1;
            }
        }
        if(replaced == uses[n].size())
            def[n]->op = IROp::noop;
    }

    return changed;
}

// TODO
// some function calls do result in constants given constant/semi-constant params
// so their values can be propagated
bool SSCP::run_pass(Module *module) {
    bool changed = false;

    for(auto *fn: module->functions) {
        changed |= SSCP::run_pass(fn);
    }

    return changed;
}
