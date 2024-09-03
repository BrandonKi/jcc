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

// FIXME temporary, for prolog/epilog to easily work
bool MERGE_RETURNS = true;

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
        auto sym = Symbol{function->id, SymbolType::function, 1, enc.buf.size()};
        bin->symbols.push_back(sym);

        enc.encode_function(function);
    }
    text.bin = enc.buf;

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

void MCIRGen::gen_prolog(MCFunction *mc_fn) {
    MCBasicBlock *prolog = new MCBasicBlock("prolog");

    if (false && get_host_os() == OS::windows) {
        MCInst sub_inst((i8)sub, MCValue((i8)MCValueKind::vreg, Type::ptr), MCValue((i8)MCValueKind::imm, Type::i64));
        sub_inst.DEST.reg = mc_fn->new_vreg();
        sub_inst.SRC1.imm = 40 + mc_fn->stack_space;
        sub_inst.DEST.hint = rsp;
        sub_inst.DEST.is_fixed = true;
        sub_inst.DEST.state = State::def;
        prolog->insts.push_back(sub_inst);
        get_vreg[rsp] = sub_inst.DEST.reg;
    } else if(true || get_host_os() == OS::linux) {
        MCInst push_inst((i8)push, MCValue((i8)MCValueKind::vreg, Type::ptr));
        push_inst.DEST.reg = mc_fn->new_vreg();
        push_inst.DEST.hint = rbp;
        push_inst.DEST.is_fixed = true;
        push_inst.DEST.state = State::def;
        prolog->insts.push_back(push_inst);
        get_vreg[rbp] = push_inst.DEST.reg;
        
        MCInst mov_inst((i8)mov, MCValue((i8)MCValueKind::vreg, Type::ptr), MCValue((i8)MCValueKind::vreg, Type::ptr));
        mov_inst.DEST.reg = get_vreg[rbp];
        mov_inst.SRC1.reg = mc_fn->new_vreg();
        mov_inst.SRC1.hint = rsp;
        mov_inst.SRC1.is_fixed = true;
        prolog->insts.push_back(mov_inst);
        get_vreg[rsp] = mov_inst.SRC1.reg;

        MCInst sub_inst((i8)sub, MCValue((i8)MCValueKind::vreg, Type::ptr), MCValue((i8)MCValueKind::imm, Type::i64));
        sub_inst.DEST.reg = get_vreg[rsp];
        sub_inst.SRC1.imm = mc_fn->stack_space;
        prolog->insts.push_back(sub_inst);
    }

    mc_fn->blocks.insert(mc_fn->blocks.begin(), prolog);
}

// TODO temporary
void MCIRGen::gen_epilog(MCFunction *mc_fn) {
    // MCBasicBlock *epilog = new MCBasicBlock("epilog");

    if(false && get_host_os() == OS::windows) {
        MCInst add_inst((i8)add, MCValue((i8)MCValueKind::vreg, Type::ptr), MCValue((i8)MCValueKind::imm, Type::i64));
        add_inst.DEST.reg = get_vreg[rsp];
        add_inst.SRC1.imm = 40 + mc_fn->stack_space;
        append_inst(mc_fn, add_inst);
    } else if(true || get_host_os() == OS::linux) {
        MCInst add_inst((i8)add, MCValue((i8)MCValueKind::vreg, Type::ptr), MCValue((i8)MCValueKind::imm, Type::i64));
        add_inst.DEST.reg = get_vreg[rsp];
        add_inst.SRC1.imm = mc_fn->stack_space;
        append_inst(mc_fn, add_inst);

        MCInst pop_inst((i8)pop, MCValue((i8)MCValueKind::vreg, Type::ptr));
        pop_inst.DEST.reg = get_vreg[rbp];
        append_inst(mc_fn, pop_inst);
    }

        MCInst ret_inst((i8)ret);
        ret_inst.DEST = mc_fn->ret;
        ret_inst.DEST.hint = rax;
        ret_inst.DEST.is_fixed = true;
        append_inst(mc_fn, ret_inst);

    // mc_fn->blocks.push_back(epilog);
}

MCFunction *MCIRGen::gen_function(Function *fn) {
    auto *mc_fn = new MCFunction(fn);

    // int i = 1;
    // for(auto p : mc_fn->params) {
    //     get_gpr_param(mc_fn->callconv, i);
    //     i += 1;
    // }

    for (auto *block : fn->blocks) {
        mc_fn->blocks.push_back(new MCBasicBlock(block->id));
        for (auto ir_inst : block->insts)
            gen_inst(mc_fn, ir_inst);
    }

    if(MERGE_RETURNS) {
        // TODO create exit block here
        auto *exit_bb = new MCBasicBlock(mc_fn->id + "_exit");
        mc_fn->blocks.push_back(exit_bb);

        // TODO temporary
        gen_prolog(mc_fn);
        gen_epilog(mc_fn);

        // MCInst ret_inst((i8)ret);
        // ret_inst.DEST = mc_fn->ret;
        // append_inst(mc_fn, ret_inst);
    }

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
    else if (is_mem_op(ir_inst.op))
        gen_mem_op(mc_fn, ir_inst);
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

static i8 convert_bin_op(IROp op) {
    switch(op) {
    case IROp::iadd:
        return (i8)add;
    case IROp::isub:
        return (i8)sub;
    case IROp::imul:
        return (i8)imul;
    case IROp::idiv:
        return (i8)idiv;
    case IROp::imod:
        return (i8)idiv;

    case IROp::fadd:
    case IROp::fsub:
    case IROp::fmul:
    case IROp::fdiv:
        assert(false);

    case IROp::lt:
    case IROp::lte:
    case IROp::gt:
    case IROp::gte:
    case IROp::eq:
    default:
        assert(false);
    }
    return -1;
}

void MCIRGen::gen_bin(MCFunction *mc_fn, IRInst ir_inst) {
    auto op = ir_inst.op;
    auto dest = ir_inst.dest;
    auto src1 = ir_inst.src1;
    auto src2 = ir_inst.src2;

    assert(dest.kind != Kind::imm);

    switch (op) {
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
    case IROp::eq: {
        i8 bin_op = convert_bin_op(op);
        if (src1.kind == Kind::vreg && src2.kind == Kind::vreg) {
            // TODO could encode as an lea
            MCInst mov_inst((i8)mov, MCValue(dest), MCValue(src1));
            MCInst bin_inst(bin_op, MCValue(dest), MCValue(src2));
            
            if(bin_op == (i8)IROp::idiv) {
                mov_inst.DEST.hint = rax;
                bin_inst.DEST.hint = rax;
            }

            append_inst(mc_fn, mov_inst);
            append_inst(mc_fn, bin_inst);
        }
        // FIXME idk what this case is
        else if (src1.kind == Kind::vreg && src2.kind == Kind::imm) {
            MCInst bin_inst(bin_op, MCValue(src1), MCValue(src2));

            if(bin_op == (i8)IROp::idiv) {
                bin_inst.DEST.hint = rax;
            }

            append_inst(mc_fn, bin_inst);
        }
        else if(src1.kind == Kind::imm && src2.kind == Kind::imm){ // should have been folded
            MCInst mov_inst((i8)mov, MCValue(dest), MCValue(src1));
            MCInst bin_inst(bin_op, MCValue(dest), MCValue(src2));

            if(bin_op == (i8)idiv) {
                MCInst mov_inst2((i8)mov);
                mov_inst2.DEST = MCValue((i8)MCValueKind::vreg, src2.type);
                mov_inst2.DEST.reg = mc_fn->new_vreg();
                mov_inst2.SRC1 = MCValue(src2);
                
                mov_inst.DEST.hint = rax;
                bin_inst.DEST.hint = rax;
                bin_inst.SRC1 = mov_inst2.DEST;
                
                append_inst(mc_fn, mov_inst2);
            }

            append_inst(mc_fn, mov_inst);
            append_inst(mc_fn, bin_inst);
        } else {
            assert(false);
        }
        return;
    }
    default:
        assert(false);
    }
}

void MCIRGen::gen_branch(MCFunction *mc_fn, IRInst ir_inst) {
    auto op = ir_inst.op;
    auto dest = ir_inst.dest;
    auto src1 = ir_inst.src1;
    auto src2 = ir_inst.src2;

    switch(op) {
    case IROp::br: {
        MCInst jmp_inst((i8)jmp);
        jmp_inst.DEST = MCValue((i8)MCValueKind::lbl, Type::i32); // FIXME, i32??
        jmp_inst.DEST.label = dest.lbl.bb->id;

        append_inst(mc_fn, jmp_inst);
        break;
    }
    case IROp::brz: {
        MCInst cmp_inst((i8)cmp);
        cmp_inst.DEST = MCValue(dest);
        cmp_inst.SRC1 = MCValue((i8)MCValueKind::imm, Type::i8);
        cmp_inst.SRC1.imm = 0;
        append_inst(mc_fn, cmp_inst);

        MCInst jz_inst((i8)jz);
        jz_inst.DEST = MCValue((i8)MCValueKind::lbl, Type::i32); // FIXME, i32??
        jz_inst.DEST.label = src1.lbl.bb->id;
        append_inst(mc_fn, jz_inst);

        MCInst jmp_inst((i8)jmp);
        jmp_inst.DEST = MCValue((i8)MCValueKind::lbl, Type::i32); // FIXME, i32??
        jmp_inst.DEST.label = src2.lbl.bb->id;
        append_inst(mc_fn, jmp_inst);
        break;
    }
    case IROp::brnz: {
        MCInst cmp_inst((i8)cmp);
        cmp_inst.DEST = MCValue(dest);
        cmp_inst.SRC1 = MCValue((i8)MCValueKind::imm, Type::i8);
        cmp_inst.SRC1.imm = 0;
        append_inst(mc_fn, cmp_inst);

        MCInst jnz_inst((i8)jnz);
        jnz_inst.DEST = MCValue((i8)MCValueKind::lbl, Type::i32); // FIXME, i32??
        jnz_inst.DEST.label = src1.lbl.bb->id;
        append_inst(mc_fn, jnz_inst);

        MCInst jmp_inst((i8)jmp);
        jmp_inst.DEST = MCValue((i8)MCValueKind::lbl, Type::i32); // FIXME, i32??
        jmp_inst.DEST.label = src2.lbl.bb->id;
        append_inst(mc_fn, jmp_inst);
        break;
    }
    default:
        assert(false);
    }
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
        mov_inst.DEST.is_fixed = true;
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
    mov_inst.SRC1.is_fixed = true;
    mov_inst.DEST = MCValue(dest);
    append_inst(mc_fn, mov_inst);
}

void MCIRGen::gen_ret(MCFunction *mc_fn, IRInst ir_inst) {
    auto op = ir_inst.op;
    auto dest = ir_inst.dest;
    auto src1 = ir_inst.src1;
    auto src2 = ir_inst.src2;

    MCInst mov_inst;
    if (ir_inst.src1.kind == Kind::vreg || ir_inst.src1.kind == Kind::imm) {
        mov_inst.op = (i8)mov;
        mov_inst.DEST = mc_fn->ret;
        mov_inst.SRC1 = MCValue(src1);
        append_inst(mc_fn, mov_inst);
    } else {
        assert(false);
    }

    if(MERGE_RETURNS) {
        MCInst jmp_inst((i8)jmp);
        MCValue label((i8)MCValueKind::lbl, Type::i32); // TODO i32??
        label.label = mc_fn->id + "_exit";
        jmp_inst.DEST = label;

        append_inst(mc_fn, jmp_inst);
    } else {
        // TODO temporary
        gen_prolog(mc_fn);
        gen_epilog(mc_fn);
    }

    // MCInst ret_inst((i8)ret);
    // ret_inst.DEST = mov_inst.DEST;
    // append_inst(mc_fn, ret_inst);
}

// FIXME use ir_inst.type everywhere instead of operands types
void MCIRGen::gen_mem_op(MCFunction *mc_fn, IRInst ir_inst) {
    static std::unordered_map<Reg, i64> map;

    switch(ir_inst.op) {
    case IROp::slot: {
        mc_fn->stack_space += size(to_mc_type(ir_inst.dest.type)) / 8;
        map[ir_inst.dest.vreg] = mc_fn->stack_space;
        break;
    }
    case IROp::store:
    case IROp::stack_store: {
        i64 offset = map[ir_inst.src1.vreg];
        MCInst store_inst((i8)mov);
        store_inst.DEST = MCValue((i8)MCValueKind::slot, Type::i32);
        store_inst.DEST.offset = offset;
        store_inst.SRC1 = MCValue(ir_inst.src2);
        append_inst(mc_fn, store_inst);
        break;
    }
    case IROp::load:
    case IROp::stack_load: {
        i64 offset = map[ir_inst.src1.vreg];
        MCInst load_inst((i8)mov);
        load_inst.DEST = MCValue(ir_inst.dest);
        load_inst.SRC1 = MCValue((i8)MCValueKind::slot, Type::i32);
        load_inst.SRC1.offset = offset;
        append_inst(mc_fn, load_inst);
        break;
    }
    default:
        assert(false);
    }
    return;
}

void MCIRGen::append_inst(MCFunction *mc_fn, MCInst mc_inst) {
    static int counter = 0; // HACK, yeah this whole function really
    if (mc_fn->blocks.empty())
        mc_fn->blocks.push_back(new MCBasicBlock(std::to_string(counter++)));
    mc_fn->blocks.back()->insts.push_back(mc_inst);
}
