#pragma once

#include "jb.h"
#include "arch/x86_64/mcir.h"
#include "arch/x86_64/mcir_gen.h"

#include <unordered_map>

namespace jb::x86_64 {

constexpr byte rex(bool w, bool r, bool x, bool b) {
    return 0b1000000 | (w << 3) | (r << 2) | (x << 1) | int(b);
}

constexpr byte rex_w = rex(1, 0, 0, 0);
constexpr byte rex_r = rex(0, 1, 0, 0);
constexpr byte rex_x = rex(0, 0, 1, 0);
constexpr byte rex_b = rex(0, 0, 0, 1);

constexpr byte modrm(byte mod, byte reg, byte rm) {
    // sanity checks
    mod &= 0b11;
    reg &= 0b111;
    rm &= 0b111;
    return (mod << 6) | (reg << 3) | (rm);
}

constexpr byte modrm_direct(byte reg, byte rm) {
    return modrm(0b11, reg, rm);
}

constexpr byte modrm_disp0(byte reg, byte rm) {
    return modrm(0b00, reg, rm);
}

constexpr byte modrm_disp8(byte reg, byte rm) {
    return modrm(0b01, reg, rm);
}

constexpr byte modrm_SIB_disp0(byte reg) {
    return modrm(0b00, reg, 0b100);
}

constexpr byte modrm_SIB_disp8(byte reg) {
    return modrm(0b01, reg, 0b100);
}

constexpr byte modrm_SIB_disp32(byte reg) {
    return modrm(0b10, reg, 0b100);
}

constexpr byte modrm_RIP_disp0(byte reg) {
    return modrm(0b00, reg, 0b101);
}

constexpr byte sib(byte scale, byte index, byte base) {
    return (scale << 6) | (index << 3) | base;
}

constexpr byte sib_base(byte base) {
    return sib(0b00, 0b100, base);
}

constexpr byte sib_scale_index(byte scale, byte index) {
    return sib(scale, index, 0b101);
}

constexpr byte sib_disp32() {
    return sib(0b00, 0b100, 0b101);
}

constexpr bool uses_data_type_info(EncodingKind e) {
    return e != EncodingKind::BASIC && e != EncodingKind::OFFSET;
}

constexpr bool is_rx(EncodingKind e) {
    return e >= EncodingKind::RX_0 && e <= EncodingKind::RX_7;
}

constexpr bool need_modrm_byte(EncodingKind e) {
    return is_rx(e) || e == EncodingKind::MOD_R_RM;
}

enum class RelocKind {
    none,
    rel32,
    absolute,
};

struct Reloc {
    RelocKind kind;
    i64 pos;
    std::string label;
};

class Encoder {
public:
    Encoder(MCModule *);
    // just for debug purposes at the moment
    std::vector<byte> raw_bin();

    void encode_function(std::vector<byte> &, MCFunction *);

private:
    MCModule *module;
    // std::unordered_map<i32, std::string> labels; // TODO
    std::unordered_map<std::string, i32> labels;
    std::vector<Reloc> relocs;

    void encode_inst(std::vector<byte> &, MCInst);

    void encode_mov(std::vector<byte> &, MCValue, MCValue);
    void encode_mov_imm(std::vector<byte> &, MCValue, i64);

    void encode_cmov(std::vector<byte> &, MCValue, MCValue, Condition);

    void encode_add(std::vector<byte> &, MCValue, MCValue);
    void encode_add_reg_imm(std::vector<byte> &, MCValue, i64);

    void encode_call(std::vector<byte> &);
    void encode_jmp(std::vector<byte> &);
    void encode_ret(std::vector<byte> &);

    void encode_push(std::vector<byte> &, MCValue);
    void encode_push_imm(std::vector<byte> &, i64);
    void encode_pop(std::vector<byte> &, MCValue);

    void encode_syscall(std::vector<byte> &);
    void encode_breakpoint(std::vector<byte> &);
    void encode_nop(std::vector<byte> &, i64);

    byte get_rex_prefix_dest(MCReg);
    byte get_rex_prefix_src(MCReg);
    byte get_rex_prefix_index(MCReg);
    byte get_rex_prefix(MCValue, MCValue);
    byte get_rex_prefix(MCValue, MCValue, MCValue);

    void emit_imm(std::vector<byte> &, MCValue, MCType);
    void emit_rel32(std::vector<byte> &, i64, MCValue);

    template <typename T>
        requires requires(T a) {
            { std::is_integral_v<T> };
            { std::is_pointer_v<T> };
            { std::is_floating_point_v<T> };
        }
    static inline void emit(std::vector<byte> &buf, const T val_) {

        using U = std::conditional_t<
            std::is_same_v<T, bool>, uint8_t,
            std::conditional_t<std::is_pointer_v<T>, uintptr_t, T>>;

        U val;
        if constexpr (std::is_pointer_v<T>)
            val = reinterpret_cast<U>(val_);
        else
            val = static_cast<U>(val_);

        using type = std::conditional_t<
            std::is_signed_v<U>,
            std::conditional_t<
                std::is_floating_point_v<U>,
                std::conditional_t<
                    std::is_same_v<float, U> && sizeof(float) == 4, uint32_t,
                    std::conditional_t<std::is_same_v<double, U> &&
                                           sizeof(double) == 8,
                                       uint64_t, void>>,
                U>,
            U>;

        auto raw_val = std::bit_cast<type>(val);
        for (size_t i = 0; i < sizeof(type); ++i)
            buf.push_back(byte((raw_val >> (i * 8)) & 0xff));
    }

    template <typename T>
        requires requires(T a) {
            { std::is_integral_v<T> };
            { std::is_pointer_v<T> };
            { std::is_floating_point_v<T> };
        }
    static inline void emit_if_nz(std::vector<byte> &buf, const T val_) {
        if (val_ != 0)
            emit<T>(buf, val_);
    }

    template <int count_, typename S>
        requires requires(S s) {
            { std::is_integral_v<S> };
        }
    static inline void patch(std::vector<byte> &buf, size_t dest_, S src_) {
        for (int i = dest_; i < dest_ + count_; ++i) {
            buf[i] = byte(src_ >> ((i-dest_) * 8)) & 0xff;
        }
    }
};

} // namespace jb::x86_64
