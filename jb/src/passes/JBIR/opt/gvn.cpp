#include "JBIR/opt/gvn.h"
#include "pretty_print.h"

#include <iostream>
#include <algorithm>

using namespace jb;

// TODO use command line flag
constexpr auto GVN_DEBUG = false;

struct GVNKey {
    i32 val1;
    i32 val2;
    IROp op;

    bool operator==(const GVNKey &other) const {
        return (val1 == other.val1
            && val2 == other.val2
            && op == other.op);
  }
};

template <>
struct std::hash<GVNKey> {
  std::size_t operator()(const GVNKey& k) const {
    return ((size_t)k.val1) ^ ((size_t)k.val2 << 32) ^ ((size_t)k.op << 48);
  }
};

static i32 gvn_hash() {
    return 0;
}

static std::unordered_map<Reg, i32> reg2val = {};
static std::unordered_map<i32, Reg> val2reg = {};

static std::unordered_map<i64, i32> const2val = {};

static bool visit_block(BasicBlock *b, std::unordered_set<BasicBlock*> &visited, std::unordered_map<GVNKey, i32> &values) {
    bool changed = false;
    static i32 val_cnt = 0;
    visited.insert(b);

    for(auto *i: b->insts) {
        if(is_mov(i->op)) {
            if(i->src1.kind == IRValueKind::imm) {
                if(!const2val.contains(i->src1.imm_int.val))
                    const2val[i->src1.imm_int.val] = val_cnt++;
                int val = const2val[i->src1.imm_int.val];
                reg2val[i->dest.vreg] = val;
                val2reg[val] = i->dest.vreg;
            }
        }
        else if(is_bin(i->op)) {
            GVNKey k{reg2val[i->src1.vreg], reg2val[i->src2.vreg], i->op};
            if(!values.contains(k)) {
                values[k] = val_cnt;
                val2reg[val_cnt] = i->dest.vreg;
                ++val_cnt;
            }
            else {
                // replace inst??
                int val = values[k];
                reg2val[i->dest.vreg] = val;
                // val2reg[val] = i->dest.vreg;

                i->op = IROp::id;
                i->src1 = IRValue(i->dest.type, val2reg[val]);
                changed = true;
            }
        }
        else if(is_phi(i->op)) {
            reg2val[i->dest.vreg] = val_cnt;
            val2reg[val_cnt] = i->dest.vreg;
            val_cnt += 1;
            changed = true;
        }
    }

    // FIXME now it works on everything, but not "optimally" (phi operands)
    //
    // FIXED:
    // FIXME, only works on extended basic blocks
    if(b->succ.size() > 1) {
        reg2val = {};
        val2reg = {};
    }

    for(auto *s: b->succ) {
        if(!visited.contains(s)) {
            visit_block(s, visited, values);
        }
    }

    return changed;
}

bool GVN::run_pass(Function *function) {
    std::unordered_set<BasicBlock*> visited;
    std::unordered_map<GVNKey, i32> values;
    bool changed = visit_block(function->blocks[0], visited, values);

    if(GVN_DEBUG) {
        for(auto &[k, v]: values) {
            std::cout << std::vformat("{} = {} {} {}\n", std::make_format_args(v, k.val1, str(k.op), k.val2));
        }

        // shows when new value numbers get assigned
        // for(auto &[k, v]: reg2val) {
        //     std::cout << std::vformat("{} = {}\n", std::make_format_args(k, v));
        // }
    }

    return changed;

}

bool GVN::run_pass(Module *module) {
    bool changed = false;

    for(auto *fn: module->functions) {
        changed |= GVN::run_pass(fn);
    }

    return changed;
}
