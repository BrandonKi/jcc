/* #include <iostream> */

#include "compiler.h"

int main() {
    JCC_PROFILE();

    init_cprint();

    jcc::Compiler c(jcc::CompileOptions{
        .link = true,
        .run_exe = true,
        .print_ir = true,
        .print_link_command = true,
    });

    int res = c.compile("test.c");

    println(res);

    return 0;
}
