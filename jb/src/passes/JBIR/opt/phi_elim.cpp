#include "JBIR/opt/phi_elim.h"
#include "pretty_print.h"

#include <iostream>
#include <algorithm>

using namespace jb;

// TODO use command line flag
constexpr auto PHI_ELIM_DEBUG = false;

static BasicBlock *maybe_split_crit_edge(Function *fn, BasicBlock *src, BasicBlock *dest) {
    static int counter = 0;
    BasicBlock *mid = src;
    if(src->succ.size() > 1 && dest->preds.size() > 1) {
        mid = new BasicBlock("crit_" + std::to_string(counter++));
        auto *term = src->insts.back();
        if(term->has_src1() && term->src1.lbl.bb == dest)
            term->src1.lbl.bb = mid;
        if(term->has_src2() && term->src2.lbl.bb == dest)
            term->src2.lbl.bb = mid;

        // is there a better way to fix up the cfg?
        src->succ.push_back(mid);
        std::erase_if(src->succ, [=](auto *p){ return p == dest; });
        dest->preds.push_back(mid);
        std::erase_if(dest->preds, [=](auto *p){ return p == src; });
        mid->preds.push_back(src);
        mid->insts.push_back(new IRInst(IROp::br, dest));
        mid->succ.push_back(dest);

        auto it = std::find(fn->blocks.begin(), fn->blocks.end(), dest);
        if(it != fn->blocks.end())
            fn->blocks.insert(it, mid);
    }
    return mid;
}

// FIXME new ssa names
static void visit_block(Function *fn, BasicBlock *b, std::unordered_set<BasicBlock*> &&visited) {
    visited.insert(b);

    b->for_each(IROp::phi, [=](IRInst *inst) {
        auto isolate = IRValue(inst->dest.type, 300+inst->dest.vreg);
        for(auto &[bb, val]: inst->values) {
            bb = maybe_split_crit_edge(fn, bb, b);
            bb->insts.push_back(new IRInst(IROp::mov, isolate, val));
            std::iter_swap(bb->insts.end()-1, bb->insts.end()-2);
        }
        inst->op = IROp::mov;
        inst->src1 = isolate;
        return nullptr;
    });

    for(int i = 0; i < b->succ.size(); ++i) {
        auto *s = b->succ[i];
        if(!visited.contains(s)) {
            visit_block(fn, s, std::move(visited));
        }
    }
}

// FIXME maybe turn split crit edges into a pass
static void elim_phis(Function *fn) {
    visit_block(fn, fn->blocks[0], {});
}

void PhiElim::run_pass(Function *function) {
    elim_phis(function);
}

void PhiElim::run_pass(Module *module) {
    for(auto *f: module->functions)
        PhiElim::run_pass(f);
}
