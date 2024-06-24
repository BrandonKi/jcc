#include "arch/x86_64/encode.h"

#include "arch/x86_64/mcir.h"
#include "arch/x86_64/pretty_print.h"

using namespace jb;
using namespace x86_64;

Encoder::Encoder(MCModule *module) : module{module} {}

// TODO remove, this is just for debugging
std::vector<byte> Encoder::raw_bin() {
    std::vector<byte> buf;
    for (auto *fn : module->functions)
        encode_function(buf, fn);

    pretty_print(buf);
    return buf;
}

void Encoder::encode_function(std::vector<byte> &buf, MCFunction *fn) {
    labels[fn->id] = buf.size();

    for (auto bb : fn->blocks) {
        labels[bb->id] = buf.size();
        for (auto inst : bb->insts) {
            encode_inst(buf, inst);
        }
    }
}

void Encoder::encode_inst(std::vector<byte> &buf, MCInst inst) {
    if (inst.op == (i8)OpCode::label) {
        labels[inst.DEST.label] = buf.size();
        return;
    }

    MCType mc_type = to_mc_type(inst.DEST.type);

    OpCodeDesc desc = OpCodeTable[inst.op];
    byte opcode = desc.op;

    if (uses_data_type_info(desc.enc_kind)) {
        opcode += (mc_type > MCType::byte);

        byte rex_prefix = get_rex_prefix(inst.DEST, inst.SRC1);
        switch (mc_type) {
        case MCType::byte:
            break;
        case MCType::word:
            emit<byte>(buf, 0x66);
            break;
        case MCType::dword:
            break;
        case MCType::qword:
            rex_prefix |= rex_w;
            break;
        default:
            if (desc.num_operands > 0)
                assert(false);
        }
        emit_if_nz<byte>(buf, rex_prefix);
    }

    if (desc.enc_kind == EncodingKind::PREFIX_0x0f)
        emit<byte>(buf, 0x0f);

    if (desc.enc_kind == EncodingKind::REG_RM)
        opcode += id((MCReg)inst.DEST.reg);

    emit<byte>(buf, opcode);

    if (desc.num_operands == 0)
        return;

    if (need_modrm_byte(desc.enc_kind)) {
        byte mod = 0b00, reg = 0b000, rm = 0b000;

        if (desc.enc_kind == EncodingKind::MOD_R_RM) {
            mod = 0b11;
            reg = id((MCReg)inst.SRC1.reg);
        } else if (is_rx(desc.enc_kind)) {
            mod = 0b11;
            reg = (i8)desc.enc_kind;
        }
        rm = id((MCReg)inst.DEST.reg);

        emit<byte>(buf, modrm(mod, reg, rm));
    }

    if (desc.num_operands == 1) {
        if (inst.DEST.kind == (i8)MCValueKind::imm)
            emit_imm(buf, inst.DEST, mc_type);
        // TODO check if enc::offset
        else if (inst.DEST.kind == (i8)MCValueKind::lbl) {
            emit<u32>(buf, 0);
            emit_rel32(buf, buf.size() - 0x04, inst.DEST);
        }
    }

    if (desc.num_operands == 2 && inst.SRC1.kind == (i8)MCValueKind::imm)
        emit_imm(buf, inst.SRC1, mc_type);

    // switch (desc.num_operands) {}
}

void Encoder::emit_imm(std::vector<byte> &buf, MCValue imm, MCType mc_type) {
    switch (mc_type) {
    case MCType::byte: {
        emit<byte>(buf, imm.imm);
        break;
    }
    case MCType::word: {
        emit<i16>(buf, imm.imm);
        break;
    }
    case MCType::dword: {
        emit<i32>(buf, imm.imm);
        break;
    }
    case MCType::qword: {
        // emit<i64>(buf, imm.imm);
        // FIXME, most ops can only use 32 bit immediates
        emit<i32>(buf, imm.imm);
        break;
    }
    default:
        assert(false);
    }
}

void Encoder::emit_rel32(std::vector<byte> &buf, i64 pos, MCValue lbl) {
    assert(to_mc_type(lbl.type) == MCType::dword);

    if (labels.contains(lbl.label)) { // FIXME double lookup
        patch<4>(buf, pos, -pos - 0x04 + labels[lbl.label]);
    } else {
        relocs.push_back(Reloc{RelocKind::rel32, pos, lbl.label});
    }
}

// void Encoder::encode_inst(std::vector<byte> &buf, MCInst inst) {
//     using enum OpCode;
//
//     switch ((OpCode)inst.op) {
//     case mov:
//         encode_mov(buf, inst.DEST, inst.SRC1);
//         return;
//     case mov_imm:
//         encode_mov_imm(buf, inst.DEST, inst.SRC1.imm);
//         return;
//     case cmov:
//         assert(false);
//         // encode_cmov(buf, inst.DEST, inst.SRC1, inst.cond);
//         return;
//
//     case add:
//         encode_add(buf, inst.DEST, inst.SRC1);
//         return;
//         // case add_reg_imm:
//         //     encode_add_reg_imm(buf, inst.DEST, inst.SRC1.imm);
//         //     return;
//
//     case call:
//         encode_call(buf);
//         return;
//     case jmp:
//         encode_jmp(buf);
//         return;
//     case ret:
//         encode_ret(buf);
//         return;
//
//     case push:
//         encode_push(buf, inst.SRC1);
//         return;
//         return;
//     // case push_imm:
//     //     encode_push_imm(buf, inst.SRC1.imm);
//     //     return;
//     case pop:
//         encode_pop(buf, inst.DEST);
//         return;
//
//     case syscall:
//         encode_syscall(buf);
//         return;
//     case breakpoint:
//         encode_breakpoint(buf);
//         return;
//     case nop:
//         encode_nop(buf, 1);
//         return;
//     default:
//         assert(false);
//     }
// }

void Encoder::encode_mov(std::vector<byte> &buf, MCValue dest_value,
                         MCValue src_value) {
    MCReg dest = (MCReg)dest_value.reg;
    MCReg src = (MCReg)src_value.reg;
    assert(size(dest) >= size(src));
    auto rex_prefix = get_rex_prefix(dest_value, src_value);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x88);
        emit<byte>(buf, modrm_direct(id(src), id(dest)));
        return;
    case 16:
        emit<byte>(buf, 0x66);
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x89);
        emit<byte>(buf, modrm_direct(id(src), id(dest)));
        return;
    case 32:
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x89);
        emit<byte>(buf, modrm_direct(id(src), id(dest)));
        return;
    case 64:
        emit<byte>(buf, rex_w | rex_prefix);
        emit<byte>(buf, 0x89);
        emit<byte>(buf, modrm_direct(id(src), id(dest)));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

// TODO pick the smallest immediate encoding
void Encoder::encode_mov_imm(std::vector<byte> &buf, MCValue dest_value,
                             i64 imm) {
    MCReg dest = (MCReg)dest_value.reg;
    auto rex_prefix = get_rex_prefix_dest(dest);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0xb0 | id(dest));
        emit<i8>(buf, imm);
        return;
    case 16:
        emit<byte>(buf, 0x66);
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0xb8 | id(dest));
        emit<i16>(buf, imm);
        return;
    case 32:
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0xb8 | id(dest));
        emit<i32>(buf, imm);
        return;
    case 64:
        emit<byte>(buf, rex_w | rex_prefix);
        emit<byte>(buf, 0xb8 | id(dest));
        emit<i64>(buf, imm);
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
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

void Encoder::encode_cmov(std::vector<byte> &buf, MCValue dest_value,
                          MCValue src_value, Condition cond) {
    MCReg dest = (MCReg)dest_value.reg;
    MCReg src = (MCReg)src_value.reg;
    auto rex_prefix = get_rex_prefix(dest_value, src_value);
    auto op = get_cmov_opcode(cond);

    switch (size(dest)) {
    case 8:
        assert(false);
    case 16:
    case 32:
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x0f);
        emit<byte>(buf, op);
        emit<byte>(buf, modrm_direct(id(dest), id(src)));
        return;
    case 64:
        emit<byte>(buf, rex_w | rex_prefix);
        emit<byte>(buf, 0x0f);
        emit<byte>(buf, op);
        emit<byte>(buf, modrm_direct(id(dest), id(src)));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::encode_add(std::vector<byte> &buf, MCValue dest_value,
                         MCValue src_value) {
    MCReg dest = (MCReg)dest_value.reg;
    MCReg src = (MCReg)src_value.reg;
    auto rex_prefix = get_rex_prefix(dest_value, src_value);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, modrm_direct(id(src), id(dest)));
        return;
    case 16:
        emit<byte>(buf, 0x66);
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x01);
        emit<byte>(buf, modrm_direct(id(src), id(dest)));
        return;
    case 32:
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x01);
        emit<byte>(buf, modrm_direct(id(src), id(dest)));
        return;
    case 64:
        emit<byte>(buf, rex_w | rex_prefix);
        emit<byte>(buf, 0x01);
        emit<byte>(buf, modrm_direct(id(src), id(dest)));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::encode_add_reg_imm(std::vector<byte> &buf, MCValue dest_value,
                                 i64 imm) {
    MCReg dest = (MCReg)dest_value.reg;
    auto rex_prefix = get_rex_prefix_dest(dest);

    switch (size(dest)) {
    case 8:
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x80);
        emit<byte>(buf, modrm_direct(0, dest));
        emit<i8>(buf, imm);
        return;
    case 16:
        emit<byte>(buf, 0x66);
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x81);
        emit<byte>(buf, modrm_direct(0, dest));
        emit<i8>(buf, imm);
        return;
    case 32:
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x81);
        emit<byte>(buf, modrm_direct(0, dest));
        emit<i8>(buf, imm);
        return;
    case 64:
        emit<byte>(buf, rex_w | rex_prefix);
        emit<byte>(buf, 0x81);
        emit<byte>(buf, modrm_direct(0, dest));
        emit<i8>(buf, imm);
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::encode_call(std::vector<byte> &buf) {
    emit<byte>(buf, 0xe8);
    emit<u32>(buf, 0x0);
}

void Encoder::encode_jmp(std::vector<byte> &buf) {}

void Encoder::encode_ret(std::vector<byte> &buf) {
    emit<byte>(buf, 0xc3);
}

void Encoder::encode_push(std::vector<byte> &buf, MCValue value) {
    MCReg reg = (MCReg)value.reg;
    auto rex_prefix = get_rex_prefix_dest(reg);

    switch (size(reg)) {
    case 8:
        assert(false);
    case 16:
        emit<byte>(buf, 0x66);
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x50 + id(reg));
    case 32:
        assert(false);
    case 64:
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x50 + id(reg));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

// TODO use smallest immediate
void Encoder::encode_push_imm(std::vector<byte> &buf, i64 imm) {
    emit<byte>(buf, 0x68);
    emit<i32>(buf, imm);
}

void Encoder::encode_pop(std::vector<byte> &buf, MCValue value) {
    MCReg reg = (MCReg)value.reg;
    auto rex_prefix = get_rex_prefix_dest(reg);

    switch (size(reg)) {
    case 8:
        assert(false);
    case 16:
        emit<byte>(buf, 0x66);
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x58 + id(reg));
    case 32:
        assert(false);
    case 64:
        emit_if_nz<byte>(buf, rex_prefix);
        emit<byte>(buf, 0x58 + id(reg));
        return;
    case 128:
    case 256:
    default:
        assert(false);
    }
}

void Encoder::encode_syscall(std::vector<byte> &buf) {
    emit<byte>(buf, 0x0f);
    emit<byte>(buf, 0x05);
}

void Encoder::encode_breakpoint(std::vector<byte> &buf) {
    emit<byte>(buf, 0xcc);
}

void Encoder::encode_nop(std::vector<byte> &buf, i64 bytes) {
    auto nop_9 = bytes / 9;
    auto nop_rest = bytes % 9;

    while (nop_9-- > 0) { // as nop_9 goes to 0
        emit<byte>(buf, 0x66);
        emit<byte>(buf, 0x0f);
        emit<byte>(buf, 0x1f);
        emit<byte>(buf, 0x84);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
    }

    switch (nop_rest) {
    case 0:
        return;
    case 1:
        emit<byte>(buf, 0x90);
        return;
    case 2:
        emit<byte>(buf, 0x66);
        emit<byte>(buf, 0x90);
        return;
    case 3:
        emit<byte>(buf, 0x0f);
        emit<byte>(buf, 0x1f);
        emit<byte>(buf, 0x00);
        return;
    case 4:
        emit<byte>(buf, 0x0f);
        emit<byte>(buf, 0x1f);
        emit<byte>(buf, 0x40);
        emit<byte>(buf, 0x00);
        return;
    case 5:
        emit<byte>(buf, 0x0f);
        emit<byte>(buf, 0x1f);
        emit<byte>(buf, 0x44);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
        return;
    case 6:
        emit<byte>(buf, 0x66);
        emit<byte>(buf, 0x0f);
        emit<byte>(buf, 0x1f);
        emit<byte>(buf, 0x44);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
        return;
    case 7:
        emit<byte>(buf, 0x0f);
        emit<byte>(buf, 0x1f);
        emit<byte>(buf, 0x80);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
        return;
    case 8:
        emit<byte>(buf, 0x0f);
        emit<byte>(buf, 0x1f);
        emit<byte>(buf, 0x84);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
        emit<byte>(buf, 0x00);
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
