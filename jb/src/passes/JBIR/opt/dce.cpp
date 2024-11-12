#include "JBIR/opt/dce.h"
#include "pretty_print.h"

#include <iostream>
#include <algorithm>

using namespace jb;

// TODO use command line flag
constexpr auto DCE_DEBUG = false;

static void visit_block(BasicBlock *b, std::unordered_set<BasicBlock*> &visited) {
    visited.insert(b);
    for(auto *s: b->succ) {
        if(!visited.contains(s)) {
            visit_block(s, visited);
        }
    }
}

static void cleanup_phis(std::vector<BasicBlock*> bbs, std::unordered_set<BasicBlock*> &visited) {
    for(auto *b: visited) {
        b->for_each(IROp::phi, [&](IRInst *i) {
            std::erase_if(i->values, [&](auto pair){
                return !visited.contains(pair.first);
            });
            return (IRInst*)nullptr; //FIXME
        });
    }
}

bool DCE::run_pass(Function *function) {
    std::unordered_set<BasicBlock*> visited;
    visit_block(function->blocks[0], visited);
    std::erase_if(function->blocks, [&](auto *b){ return !visited.contains(b); });
    cleanup_phis(function->blocks, visited);
    // NOTE must fixup cfg or rerun cfg creation

    return visited.size() > function->blocks.size();
}

// TODO
// depending on attributes, some dead functions can be removed
// for example unused static functions in a C translation unit 
bool DCE::run_pass(Module *module) {
    bool changed = false;

    for(auto *fn: module->functions) {
        changed |= DCE::run_pass(fn);
    }

    return changed;
}
