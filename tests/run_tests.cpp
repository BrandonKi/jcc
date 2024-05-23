#include "compiler.h"
/* #include "jcc.h" */

#include "cprint.h"

#include <source_location>

using namespace jcc;
using namespace std::literals;

#define NAME                                                                   \
    std::string(extract_name(std::source_location::current().function_name()))

#define test(x) run_test(#x, x);

std::vector<std::string> passed_tests;
std::vector<std::string> failed_tests;

jcc::CompileOptions co = {
    .link = true,
    .run_exe = true,
    .print_ir = false,
    .print_link_command = false,
};

static void run_test(std::string name, auto test_fn) {
    using cprint::println;

    static auto pass_str = cprint::fmt("passed: ", cprint::BRIGHT_GREEN);
    static auto fail_str = cprint::fmt("failed: ", cprint::BRIGHT_RED);

    println("Running test: "s + name);

    if (test_fn()) {
        println(pass_str + name);
        passed_tests.push_back(name);
    } else {
        println(fail_str + name);
        failed_tests.push_back(name);
    }
}

static void print_report() {
    using cprint::println;

    std::string failed = std::to_string(failed_tests.size());
    std::string passed = std::to_string(passed_tests.size());
    std::string total =
        std::to_string(failed_tests.size() + passed_tests.size());

    println();
    println(cprint::fmt("Failed: ", cprint::BRIGHT_RED) + failed);
    println(cprint::fmt("Passed: ", cprint::BRIGHT_GREEN) + passed);
    println("Total: "s + total);
}

constexpr static std::string_view extract_name(const char *signature) {
    std::string_view view(signature);
    return view.substr(13, view.find_first_of('(') - 13);
}

bool exit_success() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 100; }");

    return result == 100;
}

bool exit_fail() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return -1; }");

    return result == -1;
}

bool add_literals() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 40+2; }");

    return result == 42;
}

bool sub_literals() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 44-2; }");

    return result == 42;
}

bool mul_literals() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 40*2; }");

    return result == 80;
}

bool div_literals() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 40/2; }");

    return result == 20;
}

bool lt_literals_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 1 < 8; }");

    return result == 1;
}

bool lt_literals_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 8 < 1; }");

    return result == 0;
}

bool gt_literals_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 1023 > 9; }");

    return result == 1;
}

bool gt_literals_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 9 > 1023; }");

    return result == 0;
}

bool lte_literals_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 56 <= 56; }");

    return result == 1;
}

bool lte_literals_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 56 <= 55; }");

    return result == 0;
}

bool lte_literals_3() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 56 <= 57; }");

    return result == 1;
}

bool gte_literals_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 77 >= 77; }");

    return result == 1;
}

bool gte_literals_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 77 >= 78; }");

    return result == 0;
}

bool gte_literals_3() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 77 >= 76; }");

    return result == 1;
}

bool equal_literals_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 3 == 3; }");

    return result == 1;
}

bool equal_literals_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 3 == 4; }");

    return result == 0;
}

bool not_equal_literals_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 3 != 4; }");

    return result == 1;
}

bool not_equal_literals_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 3 != 3; }");

    return result == 0;
}

bool ooo_literals() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return (1--1*2+2*4+1)/6; }");

    return result == 2;
}

bool mod_literals() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 5 % 2; }");

    return result == 1;
}

bool shift_left_literals() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 4 << 1; }");

    return result == 8;
}

bool shift_right_literals() {
    jcc::Compiler c(co);

    auto result = c.compile_string("int main() { return 4 >> 1; }");

    return result == 2;
}

bool int_variable() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(
        int main() {
            int i = 10;
            return i;
        }
    )");

    return result == 10;
}

bool int_variables_arithmetic_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(
        int main() {
            int i = 10;
            int x = 20;
            return i - x;
        }
    )");

    return result == -10;
}

bool int_variables_arithmetic_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int x = 10;
    int y = x * 2;
    int z = y - x + 5;
    return (y * z);
}
    )");

    return result == 300;
}

bool int_variables_arithmetic_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int x = 10;
    int y = x * 2;
    int z = y - x + 5;
    return -(+y * +z) / -(+x);
}
    )");

    return result == 30;
}

bool function_call_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int three() {
    return 3;
}

int main() {
    return three();
}
    )");

    return result == 3;
}

bool function_call_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int identity(int a) {
    return a;
}

int main() {
    return identity(42);
}
    )");

    return result == 42;
}

bool function_call_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int fn(int x, int y) {
    int z = y - x + 5;
    return -(y * z) / -(+x);
}

int main() {
    return fn(10, 20);
}
    )");

    return result == 30;
}

bool pointers_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int i = 88;
    int *p = &i;
    return *p;
}
    )");

    return result == 88;
}

bool pointers_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int identity(int x) {
    int *y = &x;
    int z = *y;
    return *&z;
}

int main() {
    return identity(1001);
}
    )");

    return result == 1001;
}

bool assign_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int identity(int x) {
    int y = x + 20;
    x = y + 10;
    return x;
}

int main() {
    return identity(3);
}
    )");

    return result == 33;
}

bool assign_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int x = 90;
    x *= 2;
    return x;
}
    )");

    return result == 180;
}

bool assign_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int x = 90;
    x /= 2;
    return x;
}
    )");

    return result == 45;
}

bool assign_3() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int x = 90;
    x %= 4;
    return x;
}
    )");

    return result == 2;
}

bool assign_4() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int x = 90;
    x += 11;
    return x;
}
    )");

    return result == 101;
}

bool assign_5() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int x = 90;
    x -= 50;
    return x;
}
    )");

    return result == 40;
}

bool assign_6() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int x = 2;
    x <<= 3;
    return x;
}
    )");

    return result == 16;
}

bool assign_7() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int x = 16;
    x >>= 3;
    return x;
}
    )");

    return result == 2;
}

bool assign_8() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int x = 18;
    x &= 11;
    return x;
}
    )");

    return result == 2;
}

bool assign_9() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int x = 18;
    x ^= 11;
    return x;
}
    )");

    return result == 25;
}

bool assign_10() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int x = 18;
    x |= 11;
    return x;
}
    )");

    return result == 27;
}

int main(int argc, char *argv[]) {
    test(exit_success);
    test(exit_fail);
    test(add_literals);
    test(sub_literals);
    test(mul_literals);
    test(div_literals);
    test(lt_literals_1);
    test(lt_literals_2);
    test(gt_literals_1);
    test(gt_literals_2);
    test(lte_literals_1);
    test(lte_literals_2);
    test(lte_literals_3);
    test(gte_literals_1);
    test(gte_literals_2);
    test(gte_literals_3);
    test(equal_literals_1);
    test(equal_literals_2);
    test(not_equal_literals_1);
    test(not_equal_literals_2);
    test(ooo_literals);
    test(mod_literals);
    test(shift_left_literals);
    test(shift_right_literals);
    test(int_variable);
    test(int_variables_arithmetic_0);
    test(int_variables_arithmetic_1);
    test(int_variables_arithmetic_2);
    test(function_call_0);
    test(function_call_1);
    test(function_call_2);
    test(pointers_0);
    test(pointers_1);
    test(assign_0);
    test(assign_1);
    test(assign_2);
    test(assign_3);
    test(assign_4);
    test(assign_5);
    test(assign_6);
    test(assign_7);
    test(assign_8);
    test(assign_9);
    test(assign_10);

    print_report();
}
