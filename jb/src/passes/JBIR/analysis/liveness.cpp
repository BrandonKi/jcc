#include "JBIR/analysis/liveness.h"

#include <map>
#include <utility>
#include <iostream>

using namespace jb;

// TODO use command line flag
constexpr auto LIVENESS_DEBUG = true;

// static void Init(BasicBlock *b) {
//     for(auto i: b->insts) {
//         if(i.src1_is_vreg() && !b->defs.contains(i.src1.vreg))
//             b->livein.insert(i.src1.vreg);
//         if(i.src2_is_vreg() && !b->defs.contains(i.src2.vreg))
//             b->livein.insert(i.src2.vreg);
//         if(i.dest_is_vreg())
//             b->defs.insert(i.dest.vreg);
//     }
// }

static std::unordered_set<Reg> PhiUses(BasicBlock *b, BasicBlock *filter=nullptr) {
    std::unordered_set<Reg> phi_uses = {};

    int i = 0;
    while(i < b->insts.size() && is_phi(b->insts[i]->op)) {
        auto inst = *b->insts[i];
        for(auto r: inst.values) {
            if(filter && r.first==filter)
                phi_uses.insert(r.second.vreg);
        }
        ++i;
    }

    return phi_uses;
}

static std::unordered_set<Reg> PhiDefs(BasicBlock *b) {
    std::unordered_set<Reg> phi_defs = {};

    int i = 0;
    while(i < b->insts.size() && is_phi(b->insts[i]->op)) {
        phi_defs.insert(b->insts[i]->dest.vreg);
        ++i;
    }

    return phi_defs;
}

static void compute_livesets(BasicBlock *b, BasicBlock *succ) {
    if(succ) {
        auto phi_uses = PhiUses(succ, b);
        
        for(auto p: phi_uses) {
            b->liveout.insert(p);
            succ->livein.insert(p);
        }

        for(auto l: succ->livein) {
            b->liveout.insert(l);
        }
    }

    for(int ii = b->insts.size()-1; ii >= 0; --ii) {
        auto i = *b->insts[ii];
        if(i.dest_is_vreg()) {
            b->livein.erase(i.dest.vreg);
        }
        if(i.src1_is_vreg()) {
            b->livein.insert(i.src1.vreg);
        }
        if(i.src2_is_vreg()) {
            b->livein.insert(i.src2.vreg);
        }
    }


    for(auto *p: b->preds) {
        compute_livesets(p, b);
    }
}

void Liveness::run_pass(Function* fn) {
    for(auto *b: fn->blocks) {
        // Init(b);
        // FIXME: should be done earlier
        b->update_control_flow();
    }

    compute_livesets(fn->blocks[fn->blocks.size()-1], nullptr);

    if(LIVENESS_DEBUG) {
        for(auto *b: fn->blocks) {
            std::cout << b->id << '\n';
            std::cout << "livein: ";
            for(auto li: b->livein) {
                std::cout << li << " ";
            }
            std::cout << std::endl;
            std::cout << "liveout: ";
            for(auto lo: b->liveout) {
                std::cout << lo << " ";
            }
            std::cout << "\n\n";
        }
    }
}

void Liveness::run_pass(Module* fn) {

}
