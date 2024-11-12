#include "JBIR/opt/peephole.h"
#include "pretty_print.h"

#include <iostream>
#include <algorithm>

using namespace jb;

// TODO use command line flag
constexpr auto PEEPHOLE_DEBUG = false;

bool Peephole::run_pass(Function *function) {
    bool changed = false;
    
    for(auto *b: function->blocks) {
        for(auto *i: b->insts) {
            switch(i->op) {
            case IROp::iadd:
                if(i->src2.kind == IRValueKind::imm && i->src2.imm_int.val == 0) {
                    i->op = IROp::id;
                    changed = true;
                }
                break;
            case IROp::isub:
                if(i->src2.kind == IRValueKind::imm && i->src2.imm_int.val == 0) {
                    i->op = IROp::id;
                    changed = true;
                }
                if(i->src1_is_vreg() && i->src2_is_vreg() && i->src1.vreg == i->src2.vreg) {
                    i->op = IROp::id;
                    i->src1 = IRValue(IRConstantInt(0, 8));
                    changed = true;
                }
                break;
            case IROp::imul:
                if(i->src2.imm_int.val == 0) {
                    i->op = IROp::id;
                    i->src1 = i->src2;
                    changed = true;
                } else if(i->src2.imm_int.val == 2) {
                    i->op = IROp::iadd;
                    i->src2 = i->src1;
                    changed = true;
                }
                break;
            case IROp::idiv:
                if(i->src2.imm_int.val == 1) {
                    i->op = IROp::id;
                    changed = true;
                }
                // can't replace 0/x = 0, could be a div by zero :(
                // else if(i->src1_is_vreg() && i->src2_is_vreg() && i->src1.vreg == i->src2.vreg) {
                //     i->op = IROp::id;
                //     i->src1 = IRValue(IRConstantInt(1, 8));
                // }
                break;
            case IROp::bsl:
                if(i->src2.kind == IRValueKind::imm && i->src2.imm_int.val == 0) {
                    i->op = IROp::id;
                    changed = true;
                }
                break;
            case IROp::bsr:
                if(i->src2.kind == IRValueKind::imm && i->src2.imm_int.val == 0) {
                    i->op = IROp::id;
                    changed = true;
                }
                break;
            case IROp::band:
                if(i->src1_is_vreg() && i->src2_is_vreg() && i->src1.vreg == i->src2.vreg) {
                    i->op = IROp::id;
                    changed = true;
                }
                break;
            case IROp::bor:
                if(i->src1_is_vreg() && i->src2_is_vreg() && i->src1.vreg == i->src2.vreg) {
                    i->op = IROp::id;
                    changed = true;
                }
                break;
            default:
                break;
            }
        }
    }

    return changed;
}

bool Peephole::run_pass(Module *module) {
    bool changed = false;

    for(auto *fn: module->functions) {
        changed |= Peephole::run_pass(fn);
    }

    return changed;
}
