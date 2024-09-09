#include "JBIR/opt/mem2reg.h"
#include "pretty_print.h"

#include <iostream>

using namespace jb;

// enum NodeOp: i8 {
//     init,
//     def,
//     use,
// };

// std::string str(NodeOp n) {
//     switch(n) {
//     case NodeOp::init:
//         return "init";
//     case NodeOp::def:
//         return "def";
//     case NodeOp::use:
//         return "use";
//     default:
//         assert(false);
//     }
//     return "";
// }

// struct MemNode {
//     NodeOp op;
//     IRInst& inst;
//     BasicBlock *parent;
//     // MemNode *bwd, *fwd;
// };

// static void debug_print_slot_chains(std::unordered_map<Reg, std::vector<MemNode>> slots) {
//     for(auto s: slots) {
//         std::cout << "Reg %" << s.first << ":\n";
//         for(auto m: s.second) {
//             std::cout << str(m.op) << " [" << m.parent->id << "]\n";
//         }
//     }
// }

// std::unordered_map<Reg, std::vector<MemNode>> build_slot_chains(Function *function) {
//     std::unordered_map<Reg, std::vector<MemNode>> slots = {};

//     for(auto *b: function->blocks) {
//         for(auto *ip: b->insts) {
//             auto i = *ip;
//             switch(i.op) {
//             case IROp::slot: {
//                 assert(i.dest_is_vreg());
//                 slots.emplace(i.dest.vreg, std::vector{MemNode{NodeOp::init, i, b}});
//                 break;
//             }
//             case IROp::stack_store: {
//                 assert(i.src1_is_vreg());
//                 slots[i.src1.vreg].push_back(MemNode{NodeOp::def, i, b});
//                 break;
//             }
//             case IROp::stack_load: {
//                 assert(i.src1_is_vreg());
//                 slots[i.src1.vreg].push_back(MemNode{NodeOp::use, i, b});
//                 break;
//             }
//             default:
//                 if(i.src1_is_vreg() && slots.contains(i.src1.vreg))
//                     slots.erase(i.src1.vreg);
//                 if(i.src2_is_vreg() && slots.contains(i.src2.vreg))
//                     slots.erase(i.src2.vreg);
//                 break;
//             }
//         }
//     }

//     return slots;
// }

// static void debug_print_chains(std::unordered_map<IRInst*, std::vector<IRInst*>> chains) {
//     for(auto &[def, uses]: chains) {
//         std::cout << str(*def) << "\n--------------------\n";
//         for(auto use: uses) {
//             std::cout << str(*use) << "\n";
//         }
//         std::cout << "\n\n";
//     }
// }

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
    for(auto &[use, defs]: loads) {
        std::cout << str(*use) << "\n--------------------\n";
        for(auto &[bb, def]: defs) {
            std::cout << bb->id << ":" << str(*def) << "\n";
        }
        std::cout << "\n\n";
    }
}

// static void insert_phis(Function *function, std::unordered_map<Reg, std::vector<MemNode>>& slots) {
// TODO if any operations use the stack slot pointer
// then bail out, it cannot be converted to a vreg without extra analysis
static void insert_phis(Function *function) {
    std::unordered_map<IRInst*, IRInst *> local_loads = {};
    std::unordered_map<IRInst*, std::vector<std::pair<BasicBlock*, IRInst *>>> loads = {};
    std::unordered_map<Reg, IRInst*> local_stores = {};

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
        }
        local_stores = {};
    }

    debug_print_loads(loads);
    std::unordered_set<Reg> dead_slots = {};

    int it = 1000; // FIXME, proper naming
    for(auto &[use, def]: local_loads) {
        dead_slots.insert(def->src1.vreg);

        // stich use and def together
        use->op = IROp::id;
        use->src1 = def->src2;
        def->op = IROp::noop;

        ++it;
    }

    for(auto &[use, defs]: loads) {
        for(auto &[bb, def]: defs) {
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

void Mem2Reg::run_pass(Function *function) {
    // FIXME ignores phis, oops!
    // auto slots = build_slot_chains(function);
    
    // debug_print_slot_chains(slots);

    // insert_phis(function, slots);
    insert_phis(function);
}

void Mem2Reg::run_pass(Module *module) {
    for(auto *f: module->functions)
        Mem2Reg::run_pass(f);
}
