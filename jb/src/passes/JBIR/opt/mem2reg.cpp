#include "JBIR/opt/mem2reg.h"
#include "pretty_print.h"

#include <iostream>

using namespace jb;

// TODO use command line flag
constexpr auto MEM2REG_DEBUG = false;

static std::vector<std::pair<BasicBlock*, IRInst *>> find_stack_stores(BasicBlock *b, Reg reg) {
    std::vector<std::pair<BasicBlock*, IRInst *>> stores = {};
    for(auto *p: b->preds) {
        auto *result =
            p->for_each(IROp::stack_store, [=](IRInst *inst){
                if(inst->src1_is_vreg() && inst->src1.vreg == reg)
                    return inst;
                return (IRInst*)nullptr;
            });
        if(result)
            stores.emplace_back(p, result);
        else {
            auto pred_stores = find_stack_stores(p, reg);
            assert(!pred_stores.empty() && "could not find corresponding stack store");
            for(auto &[bb, s]: pred_stores)
                stores.emplace_back(p, s);
        }
    }
    return stores;
}

static void debug_print_loads(std::unordered_map<IRInst*, std::vector<std::pair<BasicBlock*, IRInst *>>> loads) {
    if(MEM2REG_DEBUG) {
        for(auto &[use, defs]: loads) {
            std::cout << str(use) << "\n--------------------\n";
            for(auto &[bb, def]: defs) {
                std::cout << bb->id << ":" << str(def) << "\n";
            }
            std::cout << "\n\n";
        }
    }
}

static void insert_phis(Function *function) {
    std::unordered_map<IRInst*, IRInst *> local_loads = {};
    std::unordered_map<IRInst*, std::vector<std::pair<BasicBlock*, IRInst *>>> loads = {};
    std::unordered_map<Reg, IRInst*> local_stores = {};
    std::unordered_set<Reg> invalid = {};

    for(auto *b: function->blocks) {
        for(auto *i: b->insts) {
            if(i->op == IROp::stack_store) {
                assert(i->src1_is_vreg());
                local_stores[i->src1.vreg] = i;
            }
            else if(i->op == IROp::stack_load) {
                assert(i->src1_is_vreg());
                std::vector<std::pair<BasicBlock*, IRInst *>> stores = {};
                auto it = local_stores.find(i->src1.vreg);
                if(it != local_stores.end()) {
                    local_loads[i] = it->second;
                }
                else if(b->preds.size() >= 1) {
                    stores = find_stack_stores(b, i->src1.vreg);
                    loads[i] = stores;
                }
                else
                    assert(false);
            }
            else if(is_phi(i->op)) {
                for (auto &&[b, v] : i->values) {
                    if(v.kind == IRValueKind::vreg)
                        invalid.insert(v.vreg);
                }  
            }
            else {
                if(i->src1_is_vreg())
                    invalid.insert(i->src1.vreg);
                if(i->src2_is_vreg())
                    invalid.insert(i->src2.vreg);
            }
        }
        local_stores = {};
    }

    debug_print_loads(loads);
    std::unordered_set<Reg> dead_slots = {};

    int it = 1000; // FIXME, proper naming
    for(auto &[use, def]: local_loads) {
        if(invalid.contains(def->src1.vreg))
            continue;
            
        dead_slots.insert(def->src1.vreg);

        // stich use and def together
        use->op = IROp::id;
        use->src1 = def->src2;
        def->op = IROp::noop;
        // keep it valid, since it's used in a later bb
        def->dest = use->dest;
        // if(loads.contains(use)) {
        // }

        ++it;
    }

    for(auto &[use, defs]: loads) {
        for(auto &[bb, def]: defs) {
            if(invalid.contains(def->src1.vreg))
                continue;

            // replace def
            if(def->op == IROp::stack_store) {
                dead_slots.insert(def->src1.vreg);
                def->op = IROp::id;
                def->dest.vreg = it;
                def->src1 = def->src2;
            }

            // replace use
            use->op = IROp::phi;
            use->values.emplace_back(bb, IRValue(use->src2.type, def->dest.vreg));
            
            ++it;
        }
    }

    // FIXME double lookup
    for(auto *b: function->blocks) {
        for(auto *i: b->insts) {
            if(i->op == IROp::slot && dead_slots.contains(i->dest.vreg)) {
                i->op = IROp::noop;
            }
        }
    }
}


static void insert_phis_new(Function *function) {

}

bool Mem2Reg::run_pass(Function *function) {
    insert_phis(function);
    // insert_phis_new(function);

    return false;
}

bool Mem2Reg::run_pass(Module *module) {
    bool changed = false;

    for(auto *f: module->functions)
        changed |= Mem2Reg::run_pass(f);

    return changed;
}
