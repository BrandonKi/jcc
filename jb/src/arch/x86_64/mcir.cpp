#include "arch/x86_64/mcir.h"

#include "arch/x86_64/pretty_print.h"

using namespace jb;
using namespace jb::x86_64;

void MCModule::print() {
    x86_64::pretty_print(this);
}

MCValue::MCValue() : kind{-1}, state{State::use}, type{Type::none}, hint{-1} {}
MCValue::MCValue(i8 kind, Type type) : kind{kind}, state{State::use}, type{type}, hint{-1} {}
MCValue::MCValue(i8 kind, Type type, Reg reg) : kind{kind}, state{State::use}, type{type}, hint{-1}, reg{reg} {}
MCValue::MCValue(std::string label) : kind{(i8)MCValueKind::lbl}, state{State::use}, type{}, hint{-1} {}
MCValue::MCValue(IRValue ir_value) {
    this->state = State::use;
    this->type = ir_value.type;
    this->hint = MCReg::none;
    switch (ir_value.kind) {
    case IRValueKind::none:
        assert(false);
    case IRValueKind::imm:
        this->kind = (i8)MCValueKind::imm;
        assert(ir_value.type == Type::i64); // TODO fix type
        this->imm = ir_value.imm_int.val;
        break;
    case IRValueKind::vreg:
        this->kind = (i8)MCValueKind::vreg;
        this->reg = (MCReg)ir_value.vreg;
        break;
    case IRValueKind::lbl:
        this->kind = (i8)MCValueKind::lbl;
        this->label = ir_value.lbl.bb->id;
        break;
    default:
        assert(false);
    }
}

bool MCValue::is_vreg() {
    return kind == (i8)MCValueKind::vreg;
}

bool MCValue::is_mcreg() {
    return kind == (i8)MCValueKind::mcreg;
}

bool MCValue::is_imm() {
    return kind == (i8)MCValueKind::imm;
}

bool MCValue::is_lbl() {
    return kind == (i8)MCValueKind::lbl;
}

bool MCValue::is_slot() {
    return kind == (i8)MCValueKind::slot;
}

bool MCValue::is_mem() {
    return kind == (i8)MCValueKind::mem;
}

bool MCValue::has_hint() {
    return hint != (i8)MCReg::none;
}

MCFunction::MCFunction(Function *fn) : id{fn->id}, params{}, ret{}, callconv{fn->callconv}, vreg_id{100} {
    // TODO Type to MCType conversion
    for (auto param : fn->params) {
        params.push_back(MCValue(param));
    }

    ret = MCValue(fn->ret);
}
