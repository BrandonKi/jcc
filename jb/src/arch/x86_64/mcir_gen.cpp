#include "arch/x86_64/mcir_gen.h"

#include "pretty_print.h"

#include "arch/x86_64/encode.h"
#include "arch/x86_64/pretty_print.h"

using namespace jb;
using namespace x86_64;
using Kind = IRValueKind;
using enum MCReg;
using enum OpCode;

// start of CallConv stuff
#define STACK -1

Reg jb::get_gpr_param(CallConv callconv, Reg num) {
    if (callconv == CallConv::win64) {
        switch (num) {
        case 1:
            return rcx;
        case 2:
            return rdx;
        case 3:
            return r8;
        case 4:
            return r9;
        default:
            if (num >= 5)
                return (Reg)STACK;
            else
                assert(false);
        }
    } else {
        // unimplemented
        assert(false);
    }

    return (Reg)-1;
}

Reg jb::get_fpr_param(CallConv callconv, Reg num) {
    if (callconv == CallConv::win64) {
        switch (num) {
        case 1:
            return xmm0;
        case 2:
            return xmm1;
        case 3:
            return xmm2;
        case 4:
            return xmm3;
        default:
            if (num >= 5)
                return (Reg)STACK;
            else
                assert(false);
        }
    } else {
        // unimplemented
        assert(false);
    }

    return (Reg)-1;
}

// TODO this is not correct
Reg jb::get_aggregate_param(CallConv callconv, Reg num) {
    return get_gpr_param(callconv, num);
}

Reg jb::get_gpr_ret() {
    return rax;
}

Reg jb::get_fpr_ret() {
    return xmm0;
}

// TODO can't return aggregates yet
// usually depends on the size of the aggregate
Reg jb::get_aggregate_ret() {
    return rax;
}

MCIRGen::MCIRGen(CompileOptions options, Module *module) : options{options}, module{module}, machine_module{nullptr} {}

void MCIRGen::compile() {
    gen_module();
}

std::vector<byte> MCIRGen::emit_raw_bin() {
    pretty_print(machine_module);
    Encoder encode(machine_module);
    return encode.raw_bin();
}

BinaryFile *MCIRGen::emit_bin() {
    auto bin = new BinaryFile{machine_module->name};

    // TODO need to fill 3 special sections .text/.data/.bss

    /*
      std::string name;
      std::vector<Section> sections;
      std::vector<Symbol> symbols;
    */

    // .text section
    auto text = Section{".text"};
    Encoder enc(machine_module);

    for (auto *function : machine_module->functions) {
        auto sym = Symbol{function->id, SymbolType::function, 1, text.bin.size()};
        bin->symbols.push_back(sym);

        enc.encode_function(text.bin, function);
    }

    // .data section
    auto data = Section{".data"};
    string_append(data.bin, std::string("Hello World\0"));

    // .bss section
    auto bss = Section{".bss"};

    bin->sections.push_back(text);
    bin->sections.push_back(data);
    bin->sections.push_back(bss);

    u32 index = 1;
    for (auto &section : bin->sections) {
        auto sym = Symbol{section.name, SymbolType::internal, index++, section.bin.size()};
        bin->symbols.push_back(sym);
    }

    return bin;
}

MCModule *MCIRGen::gen_module() {
    auto *mm = new MCModule(module->name);
    for (auto *fn : module->functions) {
        mm->functions.push_back(gen_function(fn));
    }
    machine_module = mm;
    return mm;
}

MCFunction *MCIRGen::gen_function(Function *fn) {
    auto *mc_fn = new MCFunction(fn);

    // int i = 1;
    // for(auto p : mc_fn->params) {
    // 	get_gpr_param(mc_fn->callconv, i);
    // 	i += 1;
    // }

    for (auto *block : fn->blocks)
        for (auto ir_inst : block->insts)
            gen_inst(mc_fn, ir_inst);

    return mc_fn;
}

void MCIRGen::gen_inst(MCFunction *mc_fn, IRInst ir_inst) {
    // if (is_imm(ir_inst.op))
    //     gen_imm(mc_fn, ir_inst);
    // else if (is_mov(ir_inst.op))
    if (is_mov(ir_inst.op))
        gen_mov(mc_fn, ir_inst);
    else if (is_bin(ir_inst.op))
        gen_bin(mc_fn, ir_inst);
    else if (is_branch(ir_inst.op))
        gen_branch(mc_fn, ir_inst);
    else if (is_call(ir_inst.op))
        gen_call(mc_fn, ir_inst);
    else if (is_ret(ir_inst.op))
        gen_ret(mc_fn, ir_inst);
    else
        assert(false);
}

void MCIRGen::gen_imm(MCFunction *mc_fn, IRInst ir_inst) {}
// void MCIRGen::gen_imm(MCFunction *mc_fn, IRInst ir_inst) {
//     auto op = ir_inst.op;
//     auto dest = ir_inst.dest;
//     auto src1 = ir_inst.src1;
//     auto src2 = ir_inst.src2;

//     switch (ir_inst.op) {
//     case IROp::iconst8:
//     case IROp::iconst16:
//     case IROp::iconst32:
//     case IROp::iconst64: {
//         MCInst mcinst((i8)mov_imm, MCValue(dest), MCValue(src1));
//         append_inst(mc_fn, mcinst);
//         return;
//     }
//     case IROp::fconst32:
//     case IROp::fconst64:
//         // TODO not implemented yet
//         assert(false);
//         return;
//     default:
//         assert(false);
//     }
// }

void MCIRGen::gen_mov(MCFunction *mc_fn, IRInst ir_inst) {
    assert(false);
}

void MCIRGen::gen_bin(MCFunction *mc_fn, IRInst ir_inst) {
    auto op = ir_inst.op;
    auto dest = ir_inst.dest;
    auto src1 = ir_inst.src1;
    auto src2 = ir_inst.src2;

    assert(dest.kind != Kind::imm);

    switch (ir_inst.op) {
    case IROp::iadd:
    case IROp::isub:
    case IROp::imul:
    case IROp::idiv:
    case IROp::imod:

    case IROp::fadd:
    case IROp::fsub:
    case IROp::fmul:
    case IROp::fdiv:

    case IROp::lt:
    case IROp::lte:
    case IROp::gt:
    case IROp::gte:
    case IROp::eq:
        if (src1.kind == Kind::vreg && src2.kind == Kind::vreg) {
            // TODO could encode as an lea
            MCInst mov_inst((i8)mov, MCValue(dest), MCValue(src1));
            append_inst(mc_fn, mov_inst);

            MCInst add_inst((i8)add, MCValue(dest), MCValue(src2));
            append_inst(mc_fn, add_inst);
        }
        // FIXME idk what this case is
        else if (src1.kind == Kind::vreg && src2.kind == Kind::imm) {
            MCInst add_inst((i8)add, MCValue(src1), MCValue(src2));
            append_inst(mc_fn, add_inst);
        }
        // FIXME idk what this case is
        else { // src1.kind == imm
            MCInst add_inst((i8)add, MCValue(dest), MCValue(src1));
            append_inst(mc_fn, add_inst);
        }
        return;
    default:
        assert(false);
    }
}

void MCIRGen::gen_branch(MCFunction *mc_fn, IRInst ir_inst) {
    auto op = ir_inst.op;
    auto dest = ir_inst.dest;
    auto src1 = ir_inst.src1;
    auto src2 = ir_inst.src2;

    MCInst jmp_inst((i8)jmp);
    jmp_inst.SRC1 = MCValue(src1);

    append_inst(mc_fn, jmp_inst);
}

void MCIRGen::gen_call(MCFunction *mc_fn, IRInst ir_inst) {
    auto op = ir_inst.op;
    auto dest = ir_inst.dest;
    auto fn = ir_inst.src1.lbl.fn;
    auto &params = ir_inst.params;

    // TODO this is where we need to move everything to/from registers
    // using the hints

    for (int i = 0; i < params.size(); ++i) {
        auto &p = params[i];
        assert((i32)p.type <= (i32)Type::i64);
        auto reg = get_gpr_param(CallConv::win64, i + 1);
        MCInst mov_inst((i8)mov);
        mov_inst.DEST = MCValue((i8)MCValueKind::vreg, p.type);
        mov_inst.DEST.reg = mc_fn->new_vreg();
        mov_inst.DEST.hint = reg;
        mov_inst.SRC1 = MCValue(p);
        append_inst(mc_fn, mov_inst);
    }

    MCInst mcinst((i8)call);
    MCValue label((i8)MCValueKind::lbl, Type::i32); // TODO i32??
    label.label = ir_inst.src1.lbl.fn->id;
    mcinst.DEST = label;
    append_inst(mc_fn, mcinst);

    MCInst mov_inst((i8)mov);
    mov_inst.SRC1 = MCValue((i8)MCValueKind::vreg, ir_inst.dest.type);
    mov_inst.SRC1.reg = mc_fn->new_vreg();
    mov_inst.SRC1.hint = rax;
    mov_inst.DEST = MCValue(dest);
    append_inst(mc_fn, mov_inst);
}

void MCIRGen::gen_ret(MCFunction *mc_fn, IRInst ir_inst) {
    auto op = ir_inst.op;
    auto dest = ir_inst.dest;
    auto src1 = ir_inst.src1;
    auto src2 = ir_inst.src2;

    MCInst mov_inst((i8)mov);
    if (ir_inst.src1.kind == Kind::vreg) {
        mov_inst.DEST = MCValue((i8)MCValueKind::vreg, ir_inst.src1.type);
        mov_inst.DEST.reg = mc_fn->new_vreg();
        mov_inst.DEST.hint = rax;
        mov_inst.SRC1 = MCValue(src1);
        append_inst(mc_fn, mov_inst);

    } else if (ir_inst.src1.kind == Kind::imm) {    // TODO why is this separate?
        mov_inst.DEST = MCValue((i8)MCValueKind::vreg, ir_inst.dest.type);
        mov_inst.DEST.reg = mc_fn->new_vreg();
        mov_inst.DEST.hint = rax;
        mov_inst.SRC1 = MCValue(src1);
        append_inst(mc_fn, mov_inst);
    } else
        assert(false);

    MCInst ret_inst((i8)ret);
    ret_inst.DEST = mov_inst.DEST;
    // ret_inst.SRC1 = mov_inst.DEST;
    append_inst(mc_fn, ret_inst);
}

void MCIRGen::append_inst(MCFunction *mc_fn, MCInst mc_inst) {
    static int counter = 0; // HACK, yeah this whole function really
    if (mc_fn->blocks.empty())
        mc_fn->blocks.push_back(new MCBasicBlock(std::to_string(counter++)));
    mc_fn->blocks.back()->insts.push_back(mc_inst);
}
