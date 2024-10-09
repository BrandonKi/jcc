#include "JBIR/analysis/create_dom_tree.h"

#include <map>
#include <utility>
#include <iostream>

using namespace jb;

std::unordered_map<BasicBlock *, i32> bb2index;

// static BasicBlock *intersect(std::vector<std::vector<BasicBlock*>> &doms, BasicBlock *b1, BasicBlock *b2) {
static BasicBlock *intersect(std::vector<BasicBlock*> &doms, BasicBlock *b1, BasicBlock *b2) {
    BasicBlock *finger1 = b1;
    BasicBlock *finger2 = b2;
    while(finger1 != finger2) {
        while(finger1 < finger2) {
            finger1 = doms[bb2index[finger1]];
        }
        while(finger2 < finger1) {
            finger2 = doms[bb2index[finger2]];
        }
    }
    return finger1;
}

void CreateDomTree::run_pass(Function* function) {
    // std::vector<std::vector<BasicBlock*>> doms;
    std::vector<BasicBlock*> doms;

    // FIXME lazy
    int index = 0;
    for(auto *b: function->blocks) {
        doms.push_back(nullptr);
        bb2index[b] = index++;
    }
    // doms[0].push_back(function->blocks[0]);
    doms[0] = function->blocks[0];

    bool changed = true;
    while(changed) {
        changed = false;
        // for all nodes, b, in reverse postorder (except start node)
        // FIXME assert postorder
        for(int i = function->blocks.size()-1; i > 0; --i) {
            BasicBlock *b = function->blocks[i];
            // new idom ← first (processed) predecessor of b
            BasicBlock *new_idom = b->preds[0];
            // for all other predecessors, p, of b
            for(int j = 1; j < b->preds.size(); ++j) {
                BasicBlock *p = b->preds[j];
                if(!doms[bb2index[p]]) {
                    new_idom = intersect(doms, p, new_idom);
                }
            }
            if(doms[bb2index[b]] != new_idom) {
                doms[bb2index[b]] = new_idom;
                changed = true;
            }
        }
    }

    for(int i = 0; i < doms.size(); ++i) {
        std::cout << std::vformat("{} -> {}\n", std::make_format_args(doms[i]->id, function->blocks[i]->id));
    }
}

void CreateDomTree::run_pass(Module* module) {
    for(auto *fn: module->functions) {
        CreateDomTree::run_pass(fn);
    }
}
