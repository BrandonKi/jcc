#include "platform.h"

#include "common.h"

#include "ast.h"

using namespace jcc;

std::array<std::array<CType, 2>, 11> Platform::builtinTypes;

int Platform::init() {
    JCC_PROFILE();

    OS os = get_host_os();
    Arch arch = get_host_arch();

    switch (arch) {
    case Arch::x64: {
        if (os == OS::windows) {
            Platform::builtinTypes = {{
                {CType(), CType()}, // None

                {CType(Char, 1, 1, false), CType(Char, 1, 1, true)}, // Char
                {CType(Short, 2, 2, false), CType(Short, 2, 2, true)}, // Short
                {CType(Int, 4, 4, false), CType(Int, 4, 4, true)}, // Int
                {CType(Long, 4, 4, false), CType(Long, 4, 4, true)}, // Long
                {CType(LLong, 8, 8, false), CType(LLong, 8, 8, true)}, // LLong

                {CType(Float, 4, 4, true), CType(Float, 4, 4, true)}, // Float
                {CType(Double, 8, 8, true),
                 CType(Double, 8, 8, true)}, // Double

                {CType(Void, 1, 1, false), CType(Void, 1, 1, false)}, // Void

                {CType(Pointer, 8, 8, false),
                 CType(Pointer, 8, 8, false)}, // Pointer

                {CType(Bool, 1, 1, false), CType(Bool, 1, 1, false)}, // Bool
            }};
        } else if (os == OS::linux) {

        } else {
            ice(false);
        }
        break;
    }
    case Arch::aarch64: {
        if (os == OS::windows) {

        } else if (os == OS::linux) {

        } else {
            ice(false);
        }
        break;
    }
    case Arch::unknown:
    default:
        ice(false);
    }

    return 0;
}
