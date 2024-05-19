#ifndef JAB_X86_64_ENCODE_H
#define JAB_X86_64_ENCODE_H

#include "jab.h"
#include "arch/x86_64/mdir.h"
#include "arch/x86_64/mdir_gen.h"

namespace jab::x86_64 {

constexpr byte rex(bool w, bool r, bool x, bool b) {
    return 0b1000000 | (w << 3) | (r << 2) | (x << 1) | int(b);
}

constexpr byte rex_w = rex(1, 0, 0, 0);
constexpr byte rex_r = rex(0, 1, 0, 0);
constexpr byte rex_x = rex(0, 0, 1, 0);
constexpr byte rex_b = rex(0, 0, 0, 1);


constexpr byte modrm(byte mod, byte reg, byte rm) {
    return (mod << 6) | (reg << 3) | (rm & 0b111);
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

// FIXME these are all static functions
class Encoder {
public:
	Encoder(MCModule*);
	// just for debug purposes at the moment
	std::vector<byte> raw_bin();

	static void encode_function(std::vector<byte>&, MCFunction*);


private:
	MCModule* module;

	static void encode_inst(std::vector<byte>&, MCInst);

	static void encode_mov(std::vector<byte>&, Register, Register);
	static void encode_mov_reg_imm(std::vector<byte>&, Register, i64);
	static void encode_mov_reg_scale(std::vector<byte>&);
	static void encode_mov_scale_imm(std::vector<byte>&);
	static void encode_mov_mem_imm(std::vector<byte>&);
	static void encode_mov_index_imm(std::vector<byte>&);

	static void encode_cmov(std::vector<byte>&, Register, Register, Condition);

	static void encode_add(std::vector<byte>&, Register, Register);
	static void encode_add_reg_imm(std::vector<byte>&, Register, i64);
	static void encode_add_reg_scale(std::vector<byte>&);
	static void encode_add_scale_imm(std::vector<byte>&);
	static void encode_add_mem_imm(std::vector<byte>&);
	static void encode_add_index_imm(std::vector<byte>&);

	static void encode_call(std::vector<byte>&);
	static void encode_jmp(std::vector<byte>&);
	static void encode_ret(std::vector<byte>&);
		
	static void encode_push(std::vector<byte>&, Register);
	static void encode_push_mem(std::vector<byte>&);
	static void encode_push_imm(std::vector<byte>&, i64);
	static void encode_pop(std::vector<byte>&, Register);
	static void encode_pop_mem(std::vector<byte>&);
		
	static void encode_syscall(std::vector<byte>&);
	static void encode_breakpoint(std::vector<byte>&);
	static void encode_nop(std::vector<byte>&, i64);

	static byte get_rex_prefix_dest(Register);
	static byte get_rex_prefix_src(Register);
	static byte get_rex_prefix_index(Register);
	static byte get_rex_prefix(Register, Register);
	static byte get_rex_prefix(Register, Register, Register);

    template <typename T>
    requires requires(T a) {
        { std::is_integral_v<T> };
        { std::is_pointer_v<T> };
        { std::is_floating_point_v<T> };
    }
    static inline void emit(std::vector<byte>& buf, const T val_) {

        using U =
            std::conditional_t<std::is_same_v<T, bool>,
                uint8_t,
                std::conditional_t<std::is_pointer_v<T>,
                    uintptr_t,
                    T>>;

        U val;
        if constexpr (std::is_pointer_v<T>)
            val = reinterpret_cast<U>(val_);
        else
            val = static_cast<U>(val_);

        using type =
            std::conditional_t<std::is_signed_v<U>,
                std::conditional_t<std::is_floating_point_v<U>,
                    std::conditional_t<std::is_same_v<float, U> && sizeof(float) == 4,
                        uint32_t,
                        std::conditional_t<std::is_same_v<double, U> && sizeof(double) == 8,
                            uint64_t,
                            void>>,
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
	static inline void emit_if_nz(std::vector<byte>& buf, const T val_) {
		if(val_ != 0)
			emit<T>(buf, val_);
	}
};

} // namespace jab::x86_64

#endif // JAB_X86_64_ENCODE_H
