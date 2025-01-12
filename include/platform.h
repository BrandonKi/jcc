#pragma once

#include <array>

namespace jcc {

class CType;

class Platform {
public:
    static int init();
    static std::array<std::array<CType, 2>, 16> builtinTypes;

private:
    Platform();
};

} // namespace jcc
