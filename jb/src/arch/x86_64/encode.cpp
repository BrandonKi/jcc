#include "arch/x86_64/encode.h"

#include "arch/x86_64/mcir.h"
#include "arch/x86_64/pretty_print.h"

using namespace jb;
using namespace x86_64;

Encoder::Encoder(MCModule *module) : module{module} {}

// TODO remove, this is just for debugging
std::vector<byte> Encoder::raw_bin() {
    for (auto *fn : module->functions)
        encode_function(fn);

    // for(auto r: module->relocs)

    pretty_print(buf);
    return buf;
}

void Encoder::encode_function(MCFunction *fn) {
    labels[fn->id] = buf.size();

    for (auto bb : fn->blocks) {
        labels[bb->id] = buf.size();
        for (auto inst : bb->insts) {
            encode_inst(inst);
        }
    }

    // FIXME, remove relocs after
    for(auto r: relocs) {
        assert(labels.contains(r.label));
        patch<4>(r.pos, -r.pos - 0x04 + labels[r.label]);
    }
}

void Encoder::encode_inst(MCInst inst) {

    switch ((OpCode)inst.op) {
    case OpCode::mov:
        mov(inst.DEST, inst.SRC1);
        return;
    // case mov_imm:
    //     mov_imm(inst.DEST, inst.SRC1.imm);
        return;
    case OpCode::cmov:
        assert(false);
        // cmov(inst.DEST, inst.SRC1, inst.cond);
        return;

    case OpCode::add:
        add(inst.DEST, inst.SRC1);
        return;
    case OpCode::sub:
        sub(inst.DEST, inst.SRC1);
        return;
    case OpCode::imul:
        imul(inst.DEST, inst.SRC1);
        return;
    case OpCode::idiv:
        idiv(inst.DEST, inst.SRC1);
        return;
    case OpCode::cmp:
        cmp(inst.DEST, inst.SRC1);
        return;

    case OpCode::call:
        call(inst.DEST);
        return;
    case OpCode::jz:
        jz(inst.DEST);
        break;
    case OpCode::jnz:
        jnz(inst.DEST);
        break;
    case OpCode::jmp:
        jmp(inst.DEST);
        return;
    case OpCode::ret:
        ret();
        return;

    case OpCode::push:
        push(inst.DEST);
        return;
    // case push_imm:
    //     push_imm(inst.SRC1.imm);
    //     return;
    case OpCode::pop:
        pop(inst.DEST);
        return;

    case OpCode::syscall:
        syscall();
        return;
    case OpCode::breakpoint:
        breakpoint();
        return;
    case OpCode::nop:
        nop(1);
        return;
    default:
        assert(false);
    }
}



// void Encoder::inst_old(MCInst inst) {
//     if (inst.op == (i8)OpCode::label) {
//         labels[inst.DEST.label] = buf.size();
//         return;
//     }

//     MCType mc_type = to_mc_type(inst.DEST.type);

//     OpCodeDesc desc = OpCodeTable[inst.op];
//     byte opcode = desc.op;

//     if (uses_data_type_info(desc.enc_kind)) {
//         opcode += (mc_type > MCType::byte);

//         byte rex_prefix = get_rex_prefix(inst.DEST, inst.SRC1);
//         switch (mc_type) {
//         case MCType::byte:
//             break;
//         case MCType::word:
//             emit<byte>(0x66);
//             break;
//         case MCType::dword:
//             break;
//         case MCType::qword:
//             rex_prefix |= rex_w;
//             break;
//         default:
//             if (desc.num_operands > 0)
//                 assert(false);
//         }
//         emit_if_nz<byte>(rex_prefix);
//     }

//     if (desc.enc_kind == EncodingKind::PREFIX_0x0f)
//         emit<byte>(0x0f);

//     if (desc.enc_kind == EncodingKind::REG_RM)
//         opcode += id((MCReg)inst.DEST.reg);

//     emit<byte>(opcode);

//     if (desc.num_operands == 0)
//         return;

//     if (need_modrm_byte(desc.enc_kind)) {
//         byte mod = 0b00, reg = 0b000, rm = 0b000;

//         if (desc.enc_kind == EncodingKind::MOD_R_RM) {
//             mod = 0b11;
//             reg = id((MCReg)inst.SRC1.reg);
//         } else if (is_rx(desc.enc_kind)) {
//             mod = 0b11;
//             reg = (i8)desc.enc_kind;
//         }
//         rm = id((MCReg)inst.DEST.reg);

//         emit<byte>(modrm(mod, reg, rm));
//     }

//     if (desc.num_operands == 1) {
//         if (inst.DEST.kind == (i8)MCValueKind::imm)
//             emit_imm(inst.DEST, mc_type);
//         // TODO check if enc::offset
//         else if (inst.DEST.kind == (i8)MCValueKind::lbl) {
//             emit<u32>(0);
//             emit_rel32(buf.size() - 0x04, inst.DEST);
//         }
//     }

//     if (desc.num_operands == 2 && inst.SRC1.kind == (i8)MCValueKind::imm)
//         emit_imm(inst.SRC1, mc_type);

//     // switch (desc.num_operands) {}
// }

void Encoder::emit_imm(MCValue imm, MCType mc_type) {
    switch (mc_type) {
    case MCType::byte: {
        emit<byte>(imm.imm);
        break;
    }
    case MCType::word: {
        emit<i16>(imm.imm);
        break;
    }
    case MCType::dword: {
        emit<i32>(imm.imm);
        break;
    }
    case MCType::qword: {
        // emit<i64>(imm.imm);
        // FIXME, most ops can only use 32 bit immediates
        emit<i32>(imm.imm);
        break;
    }
    default:
        assert(false);
    }
}

void Encoder::emit_rel32(i64 pos, MCValue lbl) {
    // FIXME, should represent 32-bit disp
    assert(to_mc_type(lbl.type) == MCType::dword);

    if (labels.contains(lbl.label)) { // FIXME double lookup
        patch<4>(pos, -pos - 0x04 + labels[lbl.label]);
    } else {
        relocs.push_back(Reloc{RelocKind::rel32, pos, lbl.label});
    }
}

void Encoder::mov(MCValue dest_value, MCValue src_value) {
    if(src_value.is_imm()) {
        mov_imm(dest_value, src_value.imm);
        return;
    } else if(src_value.is_slot()) {
        mov_mem(dest_value, src_value.offset);
        return;
    }

    auto rex_prefix = get_rex_prefix(dest_value, src_value);

    auto val = dest_value;
    if(!dest_value.is_mcreg())
        val = src_value;
    
    switch (size((MCReg)val.reg)) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x88);
        break;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x89);
        break;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x89);
        break;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0x89);
        break;
    case 128:
    case 256:
    default:
        assert(false);
    }

    if(dest_value.is_slot()) {
        emit<byte>(modrm_disp8(id((MCReg)src_value.reg), rbp));
        emit<i8>((i8)-dest_value.offset);
    } else {
        emit<byte>(modrm_direct(id((MCReg)src_value.reg), id((MCReg)dest_value.reg)));
    }
}

// TODO pick the smallest immediate encoding
void Encoder::mov_imm(MCValue dest_value, i64 imm) {
    assert(dest_value.is_mcreg() || dest_value.is_slot());

    // MCReg dest = (MCReg)dest_value.reg;
    auto rex_prefix = 0;
    i32 mc_size = 0;
    if(dest_value.is_mcreg()) {
        rex_prefix = get_rex_prefix_dest((MCReg)dest_value.reg);
        // mc_size = size((M));
    }

    // FIXME is this the right size?
    switch (size(dest_value.type)*8) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        if(dest_value.is_mcreg()) {
            emit<byte>(0xb0 | id((MCReg)dest_value.reg));
        } else if(dest_value.is_slot()) {
            emit<byte>(0xc6);
            emit<byte>(modrm_disp8(0b000, rbp));
            emit<i8>((i8)-dest_value.offset);
        }
        emit<i8>(imm);
        return;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        if(dest_value.is_mcreg()) {
            emit<byte>(0xb8 | id((MCReg)dest_value.reg));
        } else if(dest_value.is_slot()) {
            emit<byte>(0xc7);
            emit<byte>(modrm_disp8(0b000, rbp));
            emit<i8>((i8)-dest_value.offset);
        }
        emit<i16>(imm);
        return;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        if(dest_value.is_mcreg()) {
            emit<byte>(0xb8 | id((MCReg)dest_value.reg));
        } else if(dest_value.is_slot()) {
            emit<byte>(0xc7);
            emit<byte>(modrm_disp8(0b000, rbp));
            emit<i8>((i8)-dest_value.offset);
        }
        emit<i32>(imm);
        return;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        if(dest_value.is_mcreg()) {
            emit<byte>(0xb8 | id((MCReg)dest_value.reg));
        } else if(dest_value.is_slot()) {
            emit<byte>(0xc7);
            emit<byte>(modrm_disp8(0b000, rbp));
            emit<i8>((i8)-dest_value.offset);
        }
        emit<i64>(imm);
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::mov_mem(MCValue dest_value, i32 offset) {
    assert(dest_value.is_mcreg());

    auto rex_prefix = get_rex_prefix_dest((MCReg)dest_value.reg);
    
    switch (size((MCReg)dest_value.reg)) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x8a);
        break;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x8b);
        break;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x8b);
        break;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0x8b);
        break;
    case 128:
    case 256:
    default:
        assert(false);
    }

    emit<byte>(modrm_disp8(id((MCReg)dest_value.reg), rbp));
    emit<i8>((i8)-offset);
}

static byte get_cmov_opcode(Condition cond) {
    using enum Condition;

    switch (cond) {
    case above:
        return 0x47;
    case above_equal:
        return 0x43;
    case below:
        return 0x42;
    case below_equal:
        return 0x46;
    case carry:
        return 0x42;
    case equal:
        return 0x44;
    case greater:
        return 0x4f;
    case greater_equal:
        return 0x4d;
    case lesser:
        return 0x4c;
    case lesser_equal:
        return 0x4e;
    default:
        assert(false);
        return -1;
    }
}

void Encoder::cmov(MCValue dest_value, MCValue src_value, Condition cond) {
    assert(dest_value.is_mcreg() && src_value.is_mcreg());

    MCReg dest = (MCReg)dest_value.reg;
    MCReg src = (MCReg)src_value.reg;

    auto rex_prefix = get_rex_prefix(dest_value, src_value);
    auto op = get_cmov_opcode(cond);

    switch (size(dest)) {
    case 8:
        assert(false);
    case 16:
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x0f);
        emit<byte>(op);
        emit<byte>(modrm_direct(id(dest), id(src)));
        return;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0x0f);
        emit<byte>(op);
        emit<byte>(modrm_direct(id(dest), id(src)));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::add(MCValue dest_value, MCValue src_value) {
    assert(dest_value.is_mcreg());

    if(src_value.is_imm()) {
        add_reg_imm(dest_value, src_value.imm);
        return;
    }

    MCReg dest = (MCReg)dest_value.reg;
    MCReg src = (MCReg)src_value.reg;
    auto rex_prefix = get_rex_prefix(dest_value, src_value);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x00);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x01);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x01);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0x01);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::add_reg_imm(MCValue dest_value, i64 imm) {
    assert(dest_value.is_mcreg());

    MCReg dest = (MCReg)dest_value.reg;
    auto rex_prefix = get_rex_prefix_dest(dest);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x80);
        emit<byte>(modrm_direct(0, dest));
        emit<i8>(imm);
        return;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x81);
        emit<byte>(modrm_direct(0, dest));
        emit<i32>(imm);
        return;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x81);
        emit<byte>(modrm_direct(0, dest));
        emit<i32>(imm);
        return;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0x81);
        emit<byte>(modrm_direct(0, dest));
        emit<i32>(imm);
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::sub(MCValue dest_value, MCValue src_value) {
    assert(dest_value.is_mcreg());

    if(src_value.is_imm()) {
        sub_reg_imm(dest_value, src_value.imm);
        return;
    }

    MCReg dest = (MCReg)dest_value.reg;
    MCReg src = (MCReg)src_value.reg;
    auto rex_prefix = get_rex_prefix(dest_value, src_value);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x2A);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x2B);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x2B);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0x2B);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::sub_reg_imm(MCValue dest_value, i64 imm) {
    assert(dest_value.is_mcreg());

    MCReg dest = (MCReg)dest_value.reg;
    auto rex_prefix = get_rex_prefix_dest(dest);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x80);
        emit<byte>(modrm_direct(5, dest));
        emit<i8>(imm);
        return;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x81);
        emit<byte>(modrm_direct(5, dest));
        emit<i32>(imm);
        return;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x81);
        emit<byte>(modrm_direct(5, dest));
        emit<i32>(imm);
        return;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0x81);
        emit<byte>(modrm_direct(5, dest));
        emit<i32>(imm);
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::imul(MCValue dest_value, MCValue src_value) {
    assert(dest_value.is_mcreg());

    if(src_value.is_imm()) {
        imul_reg_imm(dest_value, src_value.imm);
        return;
    }

    MCReg dest = (MCReg)dest_value.reg;
    MCReg src = (MCReg)src_value.reg;
    auto rex_prefix = get_rex_prefix(dest_value, src_value);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x0F);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x0F);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x0F);
        emit<byte>(modrm_direct(id(dest), id(src)));
        return;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0x0F);
        emit<byte>(modrm_direct(id(dest), id(src)));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::imul_reg_imm(MCValue dest_value, i64 imm) {
    assert(dest_value.is_mcreg());

    MCReg dest = (MCReg)dest_value.reg;
    auto rex_prefix = get_rex_prefix_dest(dest);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x6B);
        emit<byte>(modrm_direct(id(dest), id(dest)));
        emit<i8>(imm);
        return;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x69);
        emit<byte>(modrm_direct(id(dest), id(dest)));
        emit<i32>(imm);
        return;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x69);
        emit<byte>(modrm_direct(id(dest), id(dest)));
        emit<i32>(imm);
        return;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0x69);
        emit<byte>(modrm_direct(id(dest), id(dest)));
        emit<i32>(imm);
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::idiv(MCValue dest_value, MCValue src_value) {
    assert(dest_value.is_mcreg() && dest_value.reg == MCReg::rax);

    if(src_value.is_imm()) {
        assert(false);
        idiv_reg_imm(dest_value, src_value.imm);
        return;
    }

    MCReg dest = (MCReg)dest_value.reg;
    MCReg src = (MCReg)src_value.reg;
    auto rex_prefix = get_rex_prefix(dest_value, src_value);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0xF6);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0xF7);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0xF7);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0xF7);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::idiv_reg_imm(MCValue dest_value, i64 imm) {
    assert(dest_value.is_mcreg() && dest_value.reg == MCReg::rax);

    MCReg dest = (MCReg)dest_value.reg;
    auto rex_prefix = get_rex_prefix_dest(dest);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x6B);
        emit<byte>(modrm_direct(5, dest));
        emit<i8>(imm);
        return;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x69);
        emit<byte>(modrm_direct(5, dest));
        emit<i32>(imm);
        return;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x69);
        emit<byte>(modrm_direct(5, dest));
        emit<i32>(imm);
        return;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0x69);
        emit<byte>(modrm_direct(5, dest));
        emit<i32>(imm);
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::cmp(MCValue lhs, MCValue rhs) {
    assert(lhs.is_mcreg());

    if(rhs.is_imm()) {
        cmp_reg_imm(lhs, rhs.imm);
        return;
    }

    MCReg dest = (MCReg)lhs.reg;
    MCReg src = (MCReg)rhs.reg;
    auto rex_prefix = get_rex_prefix(lhs, rhs);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x3a);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x3b);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x3b);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0x3b);
        emit<byte>(modrm_direct(id(src), id(dest)));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::cmp_reg_imm(MCValue lhs, i64 imm) {
    assert(lhs.is_mcreg());

    MCReg dest = (MCReg)lhs.reg;
    auto rex_prefix = get_rex_prefix_dest(dest);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x80);
        emit<byte>(modrm_direct(7, dest));
        emit<i8>(imm);
        return;
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x81);
        emit<byte>(modrm_direct(7, dest));
        emit<i32>(imm);
        return;
    case 32:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x81);
        emit<byte>(modrm_direct(7, dest));
        emit<i32>(imm);
        return;
    case 64:
        emit<byte>(rex_w | rex_prefix);
        emit<byte>(0x81);
        emit<byte>(modrm_direct(7, dest));
        emit<i32>(imm);
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::call(MCValue target) {
    emit<byte>(0xe8);
    emit<u32>(0x0);
    emit_rel32(buf.size() - 0x04, target);
}


void Encoder::jz(MCValue target) {
    emit<byte>(0x0f);
    emit<byte>(0x84);
    emit<u32>(0x0);
    emit_rel32(buf.size() - 0x04, target);
}

void Encoder::jnz(MCValue target) {
    emit<byte>(0x0f);
    emit<byte>(0x85);
    emit<u32>(0x0);
    emit_rel32(buf.size() - 0x04, target);
}

void Encoder::jmp(MCValue target) {
    emit<byte>(0xe9);
    emit<u32>(0x0);
    emit_rel32(buf.size() - 0x04, target);
}

void Encoder::ret() {
    emit<byte>(0xc3);
}

void Encoder::push(MCValue value) {
    MCReg reg = (MCReg)value.reg;
    auto rex_prefix = get_rex_prefix_dest(reg);

    switch (size(reg)) {
    case 8:
        assert(false);
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x50 + id(reg));
    case 32:
        assert(false);
    case 64:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x50 + id(reg));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

// TODO use smallest immediate
void Encoder::push_imm(i64 imm) {
    emit<byte>(0x68);
    emit<i32>(imm);
}

void Encoder::pop(MCValue value) {
    MCReg reg = (MCReg)value.reg;
    auto rex_prefix = get_rex_prefix_dest(reg);

    switch (size(reg)) {
    case 8:
        assert(false);
    case 16:
        emit<byte>(0x66);
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x58 + id(reg));
    case 32:
        assert(false);
    case 64:
        emit_if_nz<byte>(rex_prefix);
        emit<byte>(0x58 + id(reg));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::syscall() {
    emit<byte>(0x0f);
    emit<byte>(0x05);
}

void Encoder::breakpoint() {
    emit<byte>(0xcc);
}

void Encoder::nop(i64 bytes) {
    auto nop_9 = bytes / 9;
    auto nop_rest = bytes % 9;

    while (nop_9-- > 0) { // as nop_9 goes to 0
        emit<byte>(0x66);
        emit<byte>(0x0f);
        emit<byte>(0x1f);
        emit<byte>(0x84);
        emit<byte>(0x00);
        emit<byte>(0x00);
        emit<byte>(0x00);
        emit<byte>(0x00);
        emit<byte>(0x00);
    }

    switch (nop_rest) {
    case 0:
        return;
    case 1:
        emit<byte>(0x90);
        return;
    case 2:
        emit<byte>(0x66);
        emit<byte>(0x90);
        return;
    case 3:
        emit<byte>(0x0f);
        emit<byte>(0x1f);
        emit<byte>(0x00);
        return;
    case 4:
        emit<byte>(0x0f);
        emit<byte>(0x1f);
        emit<byte>(0x40);
        emit<byte>(0x00);
        return;
    case 5:
        emit<byte>(0x0f);
        emit<byte>(0x1f);
        emit<byte>(0x44);
        emit<byte>(0x00);
        emit<byte>(0x00);
        return;
    case 6:
        emit<byte>(0x66);
        emit<byte>(0x0f);
        emit<byte>(0x1f);
        emit<byte>(0x44);
        emit<byte>(0x00);
        emit<byte>(0x00);
        return;
    case 7:
        emit<byte>(0x0f);
        emit<byte>(0x1f);
        emit<byte>(0x80);
        emit<byte>(0x00);
        emit<byte>(0x00);
        emit<byte>(0x00);
        emit<byte>(0x00);
        return;
    case 8:
        emit<byte>(0x0f);
        emit<byte>(0x1f);
        emit<byte>(0x84);
        emit<byte>(0x00);
        emit<byte>(0x00);
        emit<byte>(0x00);
        emit<byte>(0x00);
        emit<byte>(0x00);
        return;
    default:
        assert(false);
    }
}

byte Encoder::get_rex_prefix_dest(MCReg reg) {
    return is_extended(reg) ? rex_b : 0;
}

byte Encoder::get_rex_prefix_src(MCReg reg) {
    return is_extended(reg) ? rex_r : 0;
}

byte Encoder::get_rex_prefix_index(MCReg reg) {
    return is_extended(reg) ? rex_x : 0;
}

byte Encoder::get_rex_prefix(MCValue dest, MCValue src) {
    byte dest_prefix = 0, src_prefix = 0;
    if (dest.kind == (i8)MCValueKind::mcreg)
        dest_prefix = get_rex_prefix_dest((MCReg)dest.reg);
    if (src.kind == (i8)MCValueKind::mcreg)
        src_prefix = get_rex_prefix_src((MCReg)src.reg);

    return dest_prefix | src_prefix;
}

byte Encoder::get_rex_prefix(MCValue dest, MCValue src, MCValue index) {
    if (index.kind == (i8)MCValueKind::mcreg)
        return get_rex_prefix(dest, src) |
               get_rex_prefix_index((MCReg)index.reg);
    return get_rex_prefix(dest, src);
}
