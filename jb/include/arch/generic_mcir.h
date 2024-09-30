#pragma once

#include "jb.h"

// enum class MCValueKind : i8 {
//     none,
//     vreg,
//     mcreg,
//     mem,
//     imm,
// };
namespace jb {

enum class GenericMCValueKind : i8 {
    none = -1,
    vreg,
    mcreg,
    imm,
    lbl,
    slot,
    mem,
};

enum State: i8 {
    def,
    use,
    kill,
};

struct MCValue {
    i8 kind;
    State state;
    Type type; // TODO MCType
    Reg hint;
    std::string label; // TODO use a pointer/view
    bool is_fixed = false;

    union {
        Reg reg;
        i64 imm;
        i32 offset;
        i64 ptr;
    };

    MCValue();
    MCValue(i8 kind, Type type);
    MCValue(i8 kind, Type type, Reg reg);
    MCValue(std::string label);
    MCValue(IRValue ir_value);

    bool is_vreg();
    bool is_mcreg();
    bool is_imm();
    bool is_lbl();
    bool is_slot();
    bool is_mem();

    bool has_hint();
};

#define DEST operands[0]
#define SRC1 operands[1]
#define SRC2 operands[2]
#define SRC3 operands[3]
struct MCInst {
    i8 op;
    MCValue operands[4];

    MCInst() : op{-1}, operands{} {}
    MCInst(i8 op) : op{op}, operands{} {}
    MCInst(i8 op, MCValue dest) : op{op}, operands{dest} {}
    MCInst(i8 op, MCValue dest, MCValue src1) : op{op}, operands{dest, src1} {}
    MCInst(i8 op, MCValue dest, MCValue src1, MCValue src2) : op{op}, operands{dest, src1, src2} {}
    MCInst(i8 op, MCValue dest, MCValue src1, MCValue src2, MCValue src3) : op{op}, operands{dest, src1, src2, src3} {}
};

struct MCBasicBlock {
    std::string id;
    std::vector<MCBasicBlock *> preds;
    std::vector<MCValue> params;
    std::vector<MCInst> insts;

    MCBasicBlock(std::string id) : id{id} {}
};

struct MCFunction {
    std::string id;
    std::vector<MCValue> params;
    MCValue ret;
    CallConv callconv;
    std::vector<MCBasicBlock *> blocks;

    Reg vreg_id;
    i64 stack_space = 0;

    MCFunction(Function *);

    Reg new_vreg() {
        return (Reg)vreg_id++;
    }
};

struct MCModule {
    std::string name;
    // TODO need a symtab of some sort
    // TODO also need storage:
    //     * data
    //	   * static
    //     * thread local
    std::vector<MCFunction *> functions;

    MCModule(std::string id) : name{id} {}
    void print();
};

extern Reg get_gpr_param(CallConv, Reg);

extern Reg get_fpr_param(CallConv, Reg);

// TODO this is not correct
extern Reg get_aggregate_param(CallConv, Reg);

extern Reg get_gpr_ret();

extern Reg get_fpr_ret();

extern Reg get_aggregate_ret();

} // namespace jb
