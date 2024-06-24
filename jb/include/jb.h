#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cassert>
#include <cstddef>
#include <bit>

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

using byte = u8;

using Reg = i32;

namespace jb {

class ModuleBuilder;

enum class Arch : i8 { unknown, x64, aarch64 };

enum class OS : i8 {
    unknown,
    windows,
    linux,
    macos,
    freebsd,
    android,
};

enum class ObjType : i8 { unknown, coff, elf, mach };

enum class OutputType : i8 { unknown, object_file, executable, static_lib, dynamic_lib };

enum class DebugSymbols : i8 { none, codeview, dwarf };

constexpr OS get_host_os() {
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define OS_WINDOWS
    return OS::windows;
#elif defined(__ANDROID__)
#define OS_ANDROID
    return OS::android;
#elif defined(__FreeBSD__)
#define OS_FREEBSD
    return OS::freebsd;
#elif defined(__APPLE__) || defined(__MACH__)
#define OS_MACOS
    return OS::macos;
#elif defined(__linux__)
#define OS_LINUX
    return OS::linux;
#elif defined(unix) || defined(__unix) || defined(__unix__)
#define OS_LINUX
    return OS::linux; // meh
#else
    assert("unsupported host os");
#endif
}

constexpr Arch get_host_arch() {
#if defined(__amd64__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
#define ARCH_X64
    return Arch::x64;
#elif defined(_M_ARM64) || defined(__aarch64__)
#define ARCH_AARCH64
    return Arch::aarch64;
#else
    assert("unsupported host arch");
#endif
}

// get host os/arch here
constexpr OS host_os = get_host_os();
constexpr Arch host_arch = get_host_arch();

inline DebugSymbols get_default_debug_symbols(OS os) {
    switch (os) {
    case OS::windows:
        return DebugSymbols::codeview;
    case OS::linux:
        return DebugSymbols::dwarf;
    case OS::macos:
        return DebugSymbols::dwarf;
    case OS::freebsd: // will not be supported
        return DebugSymbols::dwarf;
    case OS::android: // will not be supported
        return DebugSymbols::dwarf;
    default:
        return DebugSymbols::none;
    }
}

inline ObjType get_default_obj_type(OS os) {
    switch (os) {
    case OS::windows:
        return ObjType::coff;
    case OS::linux:
        return ObjType::elf;
    case OS::macos:
        return ObjType::mach;
    case OS::freebsd:
        return ObjType::elf; // will not be supported
    case OS::android:
        return ObjType::elf; // will not be supported
    default:
        return ObjType::unknown;
    }
}

enum class Type : i8 { none, i8, i16, i32, i64, f32, f64 };

enum class CallConv : i8 { none, win64, sysv64 };

enum class Linkage : i8 {
    none,
    interal,
    external,
};

enum class SymbolType : i8 {
    none,
    function,
    label,
    internal,
    external,
    external_def,
};

struct Symbol {
    std::string name = {};
    SymbolType type = SymbolType::none;

    u64 section_index = 0;
    u64 value = 0;
};

enum class RelocType {
    none,
    addr32,
    addr64,

    rel32,
    rel32_1,
    rel32_2,
    rel32_3,
    rel32_4,
    rel32_5,
};

struct Reloc {
    u64 virtual_address = 0;
    u64 symbol_index = 0;
    RelocType type = RelocType::none;
};

struct Section {
    std::string name = {};

    u32 virtual_size = 0;
    u32 virtual_address = 0;

    std::vector<Reloc> relocs = {};
    std::vector<byte> bin = {};
};

struct BinaryFile {
    std::string name = {};
    std::vector<Section> sections = {};
    std::vector<Symbol> symbols = {};
};

enum class IROp : i8 {
#define X(x) x,
#include "jbir_ops.inc"
#undef X
};

// TODO mem
enum class IRValueKind : i8 { none, vreg, imm };

struct IRValue {
    IRValueKind kind;
    Type type;
    union {
        Reg vreg;
        i64 imm;
    };

    IRValue();
    IRValue(i64);
    IRValue(Type);
    IRValue(Type, i32);
    IRValue(IRValueKind, Type, int);
};

struct BasicBlock;
struct Function;

struct IRInst {
    IROp op;
    IRValue dest;

    union {
        IRValue src1;
        BasicBlock *bb;
        Function *fn;
    };
    IRValue src2;

    // NOTE should only store a pointer and/or variant here
    // no reason to allocate these vectors for every instruction
    std::vector<IRValue> params;
    std::vector<std::pair<BasicBlock *, IRValue>> values;

    IRInst();
    IRInst(IROp, IRValue);
    IRInst(IROp, IRValue, IRValue, IRValue);
    IRInst(IROp, i32, IRValue, IRValue);
    IRInst(IROp, BasicBlock *);
    IRInst(IROp, IRValue, BasicBlock *);
    IRInst(IROp, i32, Function *, std::vector<IRValue>);
    IRInst(IROp, IRValue, Function *, std::vector<IRValue>);
    IRInst(IROp, IRValue, std::vector<std::pair<BasicBlock *, IRValue>>);
    // TODO move these into a different file
    bool has_dest() {
        return dest.kind != IRValueKind::none;
    }

    bool has_src1() {
        return src1.kind != IRValueKind::none;
    }

    bool has_src2() {
        return src2.kind != IRValueKind::none;
    }

    bool dest_is_vreg() {
        return dest.kind == IRValueKind::vreg;
    }

    bool src1_is_vreg() {
        return src1.kind == IRValueKind::vreg;
    }

    bool src2_is_vreg() {
        return src2.kind == IRValueKind::vreg;
    }
};

struct BasicBlock {
    std::string id;
    std::vector<BasicBlock *> preds;
    std::vector<IRValue> params;
    std::vector<IRInst> insts;

    BasicBlock(std::string);
};

// TODO add a linkage field
struct Function {
    std::string id;
    std::vector<IRValue> params;
    IRValue ret;
    CallConv callconv;
    std::vector<BasicBlock *> blocks;

    Function(std::string, std::vector<Type>, Type, CallConv);

    IRValue param(int);
};

struct Module {
    std::string name;
    std::vector<Function *> functions;

    Module(std::string);
};

enum class OptLevel : i8 { O0, O1, O2, Os };

struct CompileOptions {
    OptLevel opt = OptLevel::O0;
    DebugSymbols debug = get_default_debug_symbols(host_os);
    Arch target_arch = host_arch;
    OS target_os = host_os;
    ObjType obj_type = get_default_obj_type(host_os);
    OutputType output_type = OutputType::executable;

    std::string output_dir = "./";
    std::string output_name = "a"; // same default as gcc *shrug*
};

inline bool is_imm(IROp op) {
    return (i64)op >= (i64)IROp::iconst8 && (i64)op <= (i64)IROp::fconst64;
}

inline bool is_mov(IROp op) {
    return op == IROp::mov;
}

inline bool is_bin(IROp op) {
    return (i64)op >= (i64)IROp::addi && (i64)op <= (i64)IROp::eq;
}

inline bool is_branch(IROp op) {
    return (i64)op >= (i64)IROp::br && (i64)op <= (i64)IROp::brnz;
}

inline bool is_call(IROp op) {
    return op == IROp::call;
}

inline bool is_ret(IROp op) {
    return op == IROp::ret;
}

inline bool has_dest(IROp op) {
    return !(is_ret(op) || is_branch(op));
}

template <typename T>
    requires requires(T a) {
        { std::is_integral_v<T> };
        { std::is_pointer_v<T> };
        { std::is_floating_point_v<T> };
    }
inline void append(std::vector<byte> &buf, const T val_) {
    using U =
        std::conditional_t<std::is_same_v<T, bool>, uint8_t, std::conditional_t<std::is_pointer_v<T>, uintptr_t, T>>;

    U val;
    if constexpr (std::is_pointer_v<T>)
        val = reinterpret_cast<U>(val_);
    else
        val = static_cast<U>(val_);

    using type = std::conditional_t<
        std::is_signed_v<U>,
        std::conditional_t<
            std::is_floating_point_v<U>,
            std::conditional_t<std::is_same_v<float, U> && sizeof(float) == 4, uint32_t,
                               std::conditional_t<std::is_same_v<double, U> && sizeof(double) == 8, uint64_t, void>>,
            U>,
        U>;

    auto raw_val = std::bit_cast<type>(val);
    for (size_t i = 0; i < sizeof(type); ++i)
        buf.push_back(byte((raw_val >> (i * 8)) & 0xff));
}

template <typename T>
inline void vec_append(std::vector<T> &dest, std::vector<T> &src) {
    dest.reserve(dest.size() + src.size());
    dest.insert(dest.end(), src.begin(), src.end());
}

template <typename T>
inline void string_append(std::vector<T> &dest, std::string src) {
    auto vec = std::vector<T>(src.begin(), src.end());
    vec_append(dest, vec);
}

} // namespace jb
