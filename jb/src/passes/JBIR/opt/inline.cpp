#include "JBIR/opt/inline.h"
#include "pretty_print.h"

#include <iostream>
#include <algorithm>
#include <ranges>

using namespace jb;

// TODO use command line flag
constexpr auto INLINE_DEBUG = true;

static void do_inline_sub(Function *caller, BasicBlock *call_parent, IRInst *call, Function *callee) {
    // keep in mind:
    // * must rename everything(params, ssa defs, labels)
    // * data structures use pointers, need to deep copy everything
    // steps:
    // * deep copy callee function
    // * rename all params, ssa defs, and labels
    // * split call_parent into two bbs at the call inst
    // * copy all basic blocks to caller
    // * stitch callee entry/exit blocks into the call site 
    // * copy params into correct caller registers
    // * copy result into correct caller register
    
    // * deep copy callee function
    Function *clone = callee->clone();

    // * rename all params, ssa defs, and labels
    static int ssa_inline_name = 2000; // FIXME, collision
    std::unordered_map<Reg, Reg> rename;
    // params
    for(auto &p: clone->params) {
        rename[p.vreg] = ssa_inline_name;
        p.vreg = ssa_inline_name++;
    }
    // insts
    for(auto *bb: clone->blocks) {
        for(auto *inst: bb->insts) {
            if(inst->dest_is_vreg()) {
                if(rename.contains(inst->dest.vreg)) {
                    inst->dest.vreg = rename[inst->dest.vreg];
                } else {
                    rename[inst->dest.vreg] = ssa_inline_name;
                    inst->dest.vreg = ssa_inline_name++;
                }
            }
            if(inst->src1_is_vreg()) {
                if(rename.contains(inst->src1.vreg)) {
                    inst->src1.vreg = rename[inst->src1.vreg];
                } else {
                    rename[inst->src1.vreg] = ssa_inline_name;
                    inst->src1.vreg = ssa_inline_name++;
                }
            }
            if(inst->src2_is_vreg()) {
                if(rename.contains(inst->src2.vreg)) {
                    inst->src2.vreg = rename[inst->src2.vreg];
                } else {
                    rename[inst->src2.vreg] = ssa_inline_name;
                    inst->src2.vreg = ssa_inline_name++;
                }
            }
        }
    }
    // labels(idk, could add unique id to end of each?)
    // the clone adds it's own ids, so collision is unlikely


    // * split call_parent into two bbs at the call inst
    BasicBlock *suffix = new BasicBlock(call_parent->id + "_suffix");
    bool passed_call = false;
    int call_idx = 0;
    for(int i = 0; i < call_parent->insts.size(); ++i) {
        auto *inst = call_parent->insts[i];
        if(inst == call) {
            passed_call = true;
            call_idx = i;
        } else if(passed_call) {
            suffix->insts.push_back(inst);
        }
    }
    call_parent->insts.resize(call_idx);
    call_parent->insts.push_back(new IRInst(IROp::br, suffix));


    // * copy all basic blocks to caller, between the split blocks
    // find insertion spot
    auto it = std::find(caller->blocks.begin(), caller->blocks.end(), call_parent);
    clone->blocks.push_back(suffix);
    caller->blocks.insert_range(it+1, clone->blocks);
    clone->blocks.pop_back();
    // caller->blocks.insert(saved_it+clone->blocks.size()-1, suffix);


    // * stitch callee entry/exit blocks into the call site
    auto *entry_bb = clone->blocks[0], *exit_bb = clone->blocks.back();
    auto *old_term = clone->blocks.back()->terminator();
    auto ret_val = old_term->src1;
    // old_term->op = IROp::ret;
    // old_term->src1 = 
    call_parent->terminator()->dest.lbl.bb = entry_bb;
    auto *new_term = new IRInst(IROp::br, suffix);
    clone->blocks.back()->insts.back() = new_term;
    // clone->blocks.back()->insts.push_back(new_term);

    // * copy params into correct caller registers
    for(auto &&[dest, src]: std::views::zip(clone->params, call->params)) {
        auto insert_point = call_parent->insts.end() - 1;
        auto *inst = new IRInst(IROp::id, dest, src);
        call_parent->insts.insert(insert_point, inst);
        // call_parent->insts.insert(insert_point, new IRInst(IROp::id, dest, src));
    }


    // * copy result into correct caller register
    suffix->insts.insert(suffix->insts.begin(), new IRInst(IROp::id, call->dest, ret_val));
    suffix->insts.front()->type = call->dest.type;
    // int j = 0;

}

void Inline::run_pass(Function *function) {
    std::vector<BasicBlock*> worklist = function->blocks;
    std::reverse(worklist.begin(), worklist.end());

    while(!worklist.empty()) {
        auto *bb = worklist.back();
        worklist.pop_back();
        for(auto *inst: bb->insts) {
            if(inst->op == IROp::call && inst->src1.lbl.fn->always_inline) {
                auto *callee = inst->src1.lbl.fn;
                // inline here !!!
                if(INLINE_DEBUG) {
                    std::cout << function->id << ":" << str(inst) << " <- " << callee->id << "\n";
                }
                do_inline_sub(function, bb, inst, callee);
                worklist = function->blocks;
                break;
            }
        }
    }
}

void Inline::run_pass(Module *module) {
    for(auto *fn: module->functions) {
        Inline::run_pass(fn);
    }
}
