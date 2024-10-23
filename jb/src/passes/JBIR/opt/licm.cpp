#include "JBIR/opt/licm.h"
#include "pretty_print.h"

#include <iostream>
#include <algorithm>
#include <queue>

using namespace jb;

// TODO use command line flag
constexpr auto LICM_DEBUG = true;

struct Loop {
    i32 id;
    BasicBlock *entry;
    BasicBlock *cond;
    BasicBlock *body;
    BasicBlock *inc;
    BasicBlock *exit;
};

// FIXME, being lazy, could be more than 10
static std::unordered_map<i32, Loop> loops;

static void collect_loops(Function *function) {
    for(BasicBlock * bb: function->blocks) {
        if(bb->loop_id == -1)
            continue;

        Loop &l = loops[bb->loop_id];
        l.id = bb->loop_id;
        switch(bb->loop_info) {
        case LoopInfo::entry:
            l.entry = bb;
            break;
        case LoopInfo::cond:
            l.cond = bb;
            break;
        case LoopInfo::body:
            l.body = bb;
            break;
        case LoopInfo::inc:
            l.inc = bb;
            break;
        case LoopInfo::exit:
            l.exit = bb;
            break;
        default:
            assert(false);
        }
    }
}

static std::unordered_map<Reg, IRInst*> get_loop_defs(Loop loop) {
    std::vector<BasicBlock*> worklist = {loop.cond, loop.body, loop.inc};

    std::unordered_map<Reg, IRInst*> defs;
    for(BasicBlock *bb: worklist) {
        for(IRInst *inst: bb->insts) {
            if(inst->dest_is_vreg())
                defs.emplace(inst->dest.vreg, inst);
        }
    }

    return defs;
}

static std::unordered_map<Reg, IRInst*> get_invariants(Function *fn, Loop loop) {
    std::vector<BasicBlock*> worklist = fn->blocks;

    std::unordered_set<Reg> ind;
    std::unordered_set<Reg> dep;
    
    for(IRValue p: fn->params) {
        ind.insert(p.vreg);
    }

    // FIXME traverse in correct order, dom tree maybe?
    for(BasicBlock *bb: worklist) {
        if(bb->loop_id == loop.id && !(bb->loop_info == LoopInfo::entry || bb->loop_info == LoopInfo::exit)) {
            for(IRInst *inst: bb->insts) {
                if(inst->dest_is_vreg()) {
                    if(is_phi(inst->op)) {
                        bool invariant = true;
                        // FIXME should wait until all operand bbs have been visited
                        // traverse dom tree?
                        for(auto &[b, v]: inst->values) {
                            if(inst->src1.kind == IRValueKind::imm || (inst->src1_is_vreg() && ind.contains(inst->src1.vreg))) {

                            } else {
                                invariant = false;
                            }
                        }

                        if(invariant) {
                            ind.insert(inst->dest.vreg);
                        } else {
                            dep.insert(inst->dest.vreg);
                        }
                    } else if((inst->src1.kind == IRValueKind::imm || (inst->src1_is_vreg() && ind.contains(inst->src1.vreg)))
                            && (inst->src2.kind == IRValueKind::imm || (inst->src2_is_vreg() && ind.contains(inst->src2.vreg)))) {
                        ind.insert(inst->dest.vreg);
                    } else {
                        dep.insert(inst->dest.vreg);   
                    }
                }
            }
        } else {
            for(IRInst *inst: bb->insts) {
                if(inst->dest_is_vreg())
                    ind.insert(inst->dest.vreg);
            }
        }
    }

    if(LICM_DEBUG) {
        std::cout << "ind: ";
        for(Reg r: ind) {
            std::cout << r << ", ";
        }
        std::cout << "\n";

        std::cout << "dep: ";
        for(Reg r: dep) {
            std::cout << r << ", ";
        }
        std::cout << "\n";
    }

    std::unordered_map<Reg, IRInst*> defs = get_loop_defs(loop);

    std::unordered_map<Reg, IRInst*> intersection = defs;

    if(LICM_DEBUG) {
        std::cout << "defs: ";
        for(Reg r: ind) {
            std::cout << r << ", ";
        }
        std::cout << "\n";
    }

    std::erase_if(intersection, [&ind](auto p) {
        return !ind.contains(p.first);
    });
    // std::set_intersection(defs.begin(), defs.end(), ind.begin(), ind.end(), std::back_inserter(intersection));
    return intersection;
}

void LICM::run_pass(Function *function) {
    collect_loops(function);

    if(LICM_DEBUG) {
        for(auto &[k, v]: loops) {
            std::cout << k << ": " << v.entry->id << ", " 
                << v.cond->id << ", " << v.body->id << ", "
                << v.inc->id << ", " << v.exit->id << "\n";
        }
    }

    for(auto &[k, l]: loops) {
        std::unordered_map<Reg, IRInst*> inv = get_invariants(function, l);

        if(LICM_DEBUG) {
            std::cout << "inv: ";
            for(auto &[def, inst]: inv) {
                std::cout << "\t" << str(inst) << "\n";
            }
            std::cout << std::endl;
        }

        // old logic:
        //      move the invariant defs up into the entry block
        //      also need to make sure wherever this def is moved, it's operands are available
        // issue: could move stuff that was never going to get executed though...
        for(auto &[r, i]: inv) {
            // create invariant block
            BasicBlock *invariant = new BasicBlock(std::string("loop_") + std::to_string(l.id) + "_invariant");
            invariant->insts.push_back(i);
            invariant->insts.push_back(new IRInst(IROp::br, l.body));
            // add after cond block in blocks list
            auto it1 = std::find(function->blocks.begin(), function->blocks.end(), l.cond);
            function->blocks.insert(it1, invariant);
            // make cond jump to invariant block or exit block
            l.cond->insts.back() = new IRInst(IROp::brnz, l.cond->insts.back()->dest, invariant, l.exit);
            // add dup of cond block cond_dup after inc, and jump to body
            auto it2 = std::find(function->blocks.begin(), function->blocks.end(), l.inc);
            BasicBlock *cond_dup = new BasicBlock(*l.cond);
            cond_dup->id = l.cond->id + "_dup_" + std::to_string(l.id);
            cond_dup->insts.back() = new IRInst(IROp::brnz, cond_dup->insts.back()->dest, l.body, l.exit);
            function->blocks.insert(it2, cond_dup);
            // make inc jump to the new cond_dup block
            l.inc->insts.back() = new IRInst(IROp::br, cond_dup);
            // erase instruction from old location            
            std::erase_if(l.body->insts, [i](IRInst *other){ return i == other; });
        }
    }
}

void LICM::run_pass(Module *module) {
    for(auto *fn: module->functions) {
        LICM::run_pass(fn);
    }
}
