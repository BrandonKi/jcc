#include "JBIR/analysis/create_cfg.h"

#include <map>
#include <utility>
#include <iostream>

using namespace jb;

void CreateCFG::run_pass(Function* function) {
    for(auto *b: function->blocks) {
        b->preds.clear();
        b->succ.clear();
    }
    for(auto *b: function->blocks) {
        b->update_control_flow();
    }
}

void CreateCFG::run_pass(Module* module) {
    for(auto *fn: module->functions) {
        CreateCFG::run_pass(fn);
    }
}
