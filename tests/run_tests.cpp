#include "compiler.h"
/* #include "jcc.h" */

#include "cprint.h"

#include <source_location>

using namespace jcc;
using namespace std::literals;

#define NAME std::string(extract_name(std::source_location::current().function_name()))

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
    std::string total = std::to_string(failed_tests.size() + passed_tests.size());

    println();
    println(cprint::fmt("Failed: ", cprint::BRIGHT_RED) + failed);
    for(auto &&n: failed_tests)
        println("\t" + n);
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

    auto result = c.compile_string("int main() { return (1- -1*2+2*4+1)/6; }");

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

int func(int x) {
    int y = x + 20;
    x = y + 10;
    return x;
}

int main() {
    return func(3);
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

bool prefix_inc_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int i = 10;
    int j = ++i;

    return i + j;
}

    )");

    return result == 22;
}

bool prefix_dec_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int i = 10;
    int j = --i;

    return i + j;
}

    )");

    return result == 18;
}

// TODO capture and compare stdout
bool hello_world_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

extern void puts(char *);

int main() {
    char *a = "Hello World!";
    puts(a);
    return 0;
}

    )");

    return result == 0;
}

bool hello_world_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

extern int printf(char *);
extern void puts(char *);

int main() {
    char *a = "Hello World!";
    int result = printf(a);
    puts("");
    return result;
}

    )");

    return result == 12;
}

bool branch_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 60;

    if(a < 50)
        a = 49;

    if(a > 50)
        a = 51;

    return a;
}

    )");

    return result == 51;
}

bool branch_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 40;

    if(a < 50)
        a = 49;

    if(a > 50)
        a = 51;

    return a;
}

    )");

    return result == 49;
}

bool branch_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 60;

    if(a < 50) {
        a = 49;
    } else {
        a = 51;
    }

    return a;
}

    )");

    return result == 51;
}

bool branch_3() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 40;

    if(a < 50) {
        a = 49;
    } else {
        a = 51;
    }

    return 49;
}

    )");

    return result == 49;
}

bool branch_4() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 50;

    if(a < 50) {
        a = 49;
    } else if(a > 50) {
        a = 51;
    } else {
        a = 50;
    }

    return a;
}

    )");

    return result == 50;
}

bool branch_5() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 30;

    if(a < 50) {
        if(a > 25) {
            a = 26;
        } else {
            a = 24;
        }
    } else {
        a = 1000;
    }

    return a;
}

    )");

    return result == 26;
}

bool branch_6() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 20;

    if(a < 50) {
        if(a > 25) {
            a = 26;
        } else {
            a = 24;
        }
    } else {
        a = 1000;
    }

    return a;
}

    )");

    return result == 24;
}

bool for_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 0;

    for(int i = 0; i < 10; ++i) {
        a += i;
    }

    return a;
}

    )");

    return result == 45;
}

bool for_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 0;

    for(int i = 0; i < 0; i -= 1) {
        a += 9;
    }

    return a;
}

    )");

    return result == 0;
}

bool for_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 0;

    for(int i = 0; i < 10; i++) {
        if(i % 2 == 0)
            a += i;
        else
            a -= 1;
    }

    return a;
}

    )");

    return result == 15;
}

bool for_3() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 0;
    int b = 0;

    for(int i = 0; i < 10; ++i) {
        a += i;
        b = 10;
    }

    return a + b;
}

    )");

    return result == 55;    
}

bool while_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 0;

    while(a < 10) {
        a += 1;
    }

    return a;
}

    )");

    return result == 10;
}

bool while_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 10;

    while(a < 10) {
        a += 1;
    }

    return a;
}

    )");

    return result == 10;
}

bool dowhile_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 0;

    do {
        a += 1;
    } while(a < 10);

    return a;
}

    )");

    return result == 10;
}

bool dowhile_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int a = 10;

    do {
        a += 1;
    } while(a < 10);

    return a;
}

    )");

    return result == 11;
}

bool sizeof_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int i;
    return sizeof(int);
}

    )");

    return result == sizeof(int);
}

bool sizeof_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int i;
    return sizeof &i + sizeof(&i);
}

    )");

    return result == sizeof(int *) * 2;
}

bool sizeof_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    short *s;
    int j = sizeof *s;
    return j;
}

    )");

    return result == sizeof(short);
}

bool sizeof_3() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    short a = 0;
    short b = 1;
    return sizeof a + b;
}

    )");

    return result == sizeof(short) + 1;
}

bool sizeof_4() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    short a = 0;
    short b = 1;
    return sizeof a + b;
}

    )");

    return result == sizeof(short) + 1;
}

bool sizeof_5() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    short a = 0;
    short b = 1;
    return sizeof (a + b);
}

    )");

    return result == sizeof(int);
}

bool alignof_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    return _Alignof(int*);
}

    )");

    return result == alignof(int *);
}

bool sizeof_alignof_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int i = sizeof(int);
    int j = sizeof &i;
    int k = _Alignof(int*);
    return i + j + k;
}

    )");

    return result == sizeof(int) + sizeof(int *) + alignof(int *);
}

bool preproc_0() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define W 1 + 2
int main() {
    return W;
}

    )");

    return result == 3;
}

bool preproc_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define X 1
#define Y 2
#define Z 3

int main() {
    return X + Y + Z;
}

    )");

    return result == 6;
}

bool preproc_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
#define X 2
    return X;
}

    )");

    return result == 2;
}

bool preproc_3() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define X 2
#define Y X

int main() {
    return Y;
}

    )");

    return result == 2;
}

bool preproc_4() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define ii i
#define jj j
#define X(x, y) x+y-1

int main() {
    int i = 10;
    int j = 12;
    return X((ii), jj);
}

    )");

    return result == 21;
}

bool preproc_5() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define SQUARE(x) ((x) * (x))

int main() {
    int num = 5;
    return SQUARE(num);
}

    )");

    return result == 25;
}

bool preproc_6() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define COND

int main() {
#ifdef COND
    return 300;
#else
    return 200;
#endif
    return -1;
}

    )");

    return result == 300;
}

bool preproc_7() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define COND

int main() {
#ifndef COND
    return 300;
#else
    return 200;
#endif
    return -1;
}

    )");

    return result == 200;
}

bool preproc_8() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define COND
#undef COND

int main() {
#ifndef COND
    return 300;
#else
    return 200;
#endif
    return -1;
}

    )");

    return result == 300;
}

bool preproc_9() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define COND1

int main() {
#ifndef COND1
#ifdef COND2
    return 1;
#else
    return 2;
#endif
#else
    return 100;
#endif
    return -1;
}

    )");

    return result == 100;
}

bool preproc_10() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define COND1

int main() {
#ifdef COND1
#ifdef COND2
    return 1;
#else
    return 2;
#endif
#else
    return 100;
#endif
    return -1;
}

    )");

    return result == 2;
}

bool preproc_11() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define COND1
#define COND2

int main() {
#ifdef COND1
#ifdef COND2
    return 1;
#else
    return 2;
#endif
#else
    return 100;
#endif
    return -1;
}

    )");

    return result == 1;
}

bool preproc_12() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#include <stdbool.h>

int main() {
    return true;
}

    )");

    return result == 1;
}

bool preproc_13() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define TEST
#if defined(TEST)
#define X 10
#else
#define X 5
#endif

int main() {
    return X;
}

    )");

    return result == 10;
}

bool preproc_14() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define TEST
#if defined TEST && 42 != 42
#define X 10
#else
#define X 5
#endif

int main() {
    return X;
}

    )");

    return result == 5;
}

bool preproc_15() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
#if RANDOM_ID
    return 100;
#else
#if !RANDOM_ID
    return 10;
#else
    return 1;
#endif
#endif
}

    )");

    return result == 10;
}

bool preproc_16() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

extern int printf(char*);

#define STRINGIFY(x) #x

int main() {
    return printf(STRINGIFY(hello));
}

    )");

    return result == 5;
}

bool preproc_17() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#define SCHAR_MIN   (-128)

int main() {
    return SCHAR_MIN;
}

    )");

    return result == -128;
}

bool struct_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    struct IntPair {
        int a;
        int b;
    };
    struct IntPair s;
    s.a = 5;
    return 100;
}

    )");

    return result == 100;
}

bool struct_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    struct IntPair {
        int a;
        int b;
    };
    struct IntPair s;
    s.a = 5;
    return s.a;
}

    )");

    return result == 5;
}

bool struct_3() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    struct IntPair {
        int a;
        int b;
    };
    struct IntPair s;
    s.a = 5;
    s.b = 10;
    return s.b;
}

    )");

    return result == 10;
}

bool struct_4() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    struct IntPair {
        int a;
        int b;
    };
    struct IntPair s;
    s.a = 5;
    s.b = 10;
    return s.a + s.b;
}

    )");

    return result == 15;
}

bool struct_5() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    struct IntPair {
        long long a;
        int b;
    };
    struct IntPair s;
    s.a = 5;
    s.b = 10;
    return s.a + s.b;
}

    )");

    return result == 15;
}

bool struct_6() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    struct Int {
        int i;
    };
    struct Int integer;
    struct Int *ptr = &integer;
    (*ptr).i = 42;
    return (*ptr).i;
}

    )");

    return result == 42;
}

bool struct_7() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    struct Int {
        int i;
    };
    struct Int integer;
    struct Int *ptr = &integer;
    ptr->i = 42;
    return ptr->i;
}

    )");

    return result == 42;
}

bool array_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int main() {
    int arr[10];
    arr[0] = 1;
    arr[1] = 4;
    arr[2] = 5;
    arr[9] = 0;
    return arr[0] + arr[1]+ arr[2] + arr[9];
}

    )");

    return result == 10;
}

bool general_1() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

#include <stdbool.h>

// extern int printf(char*);

bool isPerfect(int num) {
    int sum = 0;
    for (int i = 1; i < num; i++) {
        if (num % i == 0) {
            sum += i;
        }
    }
    return sum == num;
}

int main() {
    return isPerfect(28);
}

    )");

    return result == 1;
}

bool general_2() {
    jcc::Compiler c(co);

    auto result = c.compile_string(R"(

int popcount_recursive(unsigned int n) {
    if (n == 0) return 0;
    return (n & 1) + popcount_recursive(n >> 1);
}

int main() {
    unsigned int num = 29; // Binary: 11101
    return popcount_recursive(num);
}

    )");

    return result == 4;
}

int main(int argc, char *argv[]) {
    // test(exit_success);
    // test(exit_fail);
    // test(add_literals);
    // test(sub_literals);
    // test(mul_literals);
    // test(div_literals);
    // test(lt_literals_1);
    // test(lt_literals_2);
    // test(gt_literals_1);
    // test(gt_literals_2);
    // test(lte_literals_1);
    // test(lte_literals_2);
    // test(lte_literals_3);
    // test(gte_literals_1);
    // test(gte_literals_2);
    // test(gte_literals_3);
    // test(equal_literals_1);
    // test(equal_literals_2);
    // test(not_equal_literals_1);
    // test(not_equal_literals_2);
    // test(ooo_literals);
    // test(mod_literals);
    // test(shift_left_literals);
    // test(shift_right_literals);
    // test(int_variable);
    // test(int_variables_arithmetic_0);
    // test(int_variables_arithmetic_1);
    // test(int_variables_arithmetic_2);
    // test(function_call_0);
    // test(function_call_1);
    // test(function_call_2);
    // test(pointers_0);
    // test(pointers_1);
    // test(assign_0);
    // test(assign_1);
    // test(assign_2);
    // test(assign_3);
    // test(assign_4);
    // test(assign_5);
    // test(assign_6);
    // test(assign_7);
    // test(assign_8);
    // test(assign_9);
    // test(assign_10);
    // test(prefix_inc_0);
    // test(prefix_dec_0);
    // test(hello_world_0);
    // test(hello_world_1);
    // test(branch_0);
    // test(branch_1);
    // test(branch_2);
    // test(branch_3);
    // test(branch_4);
    // test(branch_5);
    // test(branch_6); // good test
    // test(for_0);
    // test(for_1);
    // test(for_2);
    // test(for_3);
    // test(while_0);
    // test(dowhile_0);
    // test(sizeof_0);
    // test(sizeof_1);
    // test(sizeof_2);
    // test(sizeof_3);
    // test(sizeof_4);
    // test(sizeof_5);
    // test(alignof_0);
    // test(sizeof_alignof_0);
    // test(preproc_0);
    // test(preproc_1);
    // test(preproc_2);
    // test(preproc_3);
    // test(preproc_4);
    // test(preproc_5);
    // test(preproc_6);
    // test(preproc_7);
    // test(preproc_8);
    // test(preproc_9);
    // test(preproc_10);
    // test(preproc_11);
    // test(preproc_12);
    // test(preproc_13);
    // test(preproc_14);
    // test(preproc_15);
    // test(preproc_16);
    // test(preproc_17);
    // test(struct_1);
    // test(struct_2);
    // test(struct_3);
    // test(struct_4);
    // test(struct_5);
    // test(struct_6);
    // test(struct_7);
    test(array_1);
    // test(general_1);
    // test(general_2);

    print_report();
}
