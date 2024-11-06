#include "JBIR/opt/cleanup.h"
#include "pretty_print.h"

#include <iostream>
#include <algorithm>
#include <unordered_map>

using namespace jb;

// TODO use command line flag
constexpr auto CLEANUP_DEBUG = false;

static void cleanup_phis(BasicBlock* bb, std::unordered_map<BasicBlock*, BasicBlock*>& replaced) {
    bb->for_each(IROp::phi, [&](IRInst *i) {
        for(auto &[pred, v]: i->values) {
            if(replaced.contains(pred))
                pred = replaced[pred];
            // if a phi in bb has bb as a pred then, assuming the IR is well-formed,
            // we can just replace it with it's value
            if(pred == bb) {
                i->op = IROp::id;
                i->src1 = v;
            }
        }
        // auto it = std::find_if(i->values.begin(), i->values.end(),
        //     [&](auto pair){
        //         return pair.first == bb;
        //     });
        // // if a phi in bb has bb as a pred then, assuming the IR is well-formed,
        // // we can just replace it with it's value
        // if(it != i->values.end()) {
        //     i->op = IROp::id;
        //     i->src1 = it->second;
        // }
        return (IRInst*)nullptr; //FIXME
    });
}

void Cleanup::run_pass(Function *function) {
    std::unordered_map<BasicBlock*, BasicBlock*> replaced;
    std::vector<BasicBlock*> worklist = function->blocks;
    std::reverse(worklist.begin(), worklist.end());
    // for each bb
    //     if bb has 1 succ and it has 1 pred
    //         replace bb->term() with succ->insts
    while(!worklist.empty()) {
        auto *bb = worklist.back();
        worklist.pop_back();
        if(replaced.contains(bb))
            continue;
        if(bb->succ.size() == 1 && bb->succ[0]->preds.size() == 1) {
            auto *merge = bb->succ[0];
            bb->succ = bb->succ[0]->succ;
            for(auto *s: bb->succ) {
                auto it = std::find(s->preds.begin(), s->preds.end(), merge);
                assert(it != s->preds.end());
                *it = bb;
            }
            bb->insts.pop_back();
            for(auto *i: merge->insts) {
                bb->insts.push_back(i);
            }
            cleanup_phis(bb, replaced);
            for(auto *s: bb->succ) { // FIXME here, phis in successors
                cleanup_phis(s, replaced);
            }
            std::erase_if(function->blocks, [merge](auto *block){ return block == merge; });
            replaced.emplace(merge, bb);
            worklist.push_back(bb);
            // worklist.insert(worklist.begin(), bb);
        }
    }
}

void Cleanup::run_pass(Module *module) {
    for(auto *fn: module->functions) {
        Cleanup::run_pass(fn);
    }
}
