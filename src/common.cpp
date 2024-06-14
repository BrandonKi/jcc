#include "common.h"

void jcc_ice_assert(const char *expr, std::stacktrace s, const char *message) {
    if (message)
        std::cout << "ICE: " << message << ", " << expr << "\n\n";
    else
        std::cout << "ICE: " << expr << "\n\n";
    std::cout << "Stack Trace:\n" << s << '\n';
    std::exit(-1);
}
