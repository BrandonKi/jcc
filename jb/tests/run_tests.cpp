#include "jb.h"
#include "module_builder.h"
#include "jit_env.h"
#include "context.h"

#include "cprint.h"

#include <source_location>

using namespace jb;
using namespace std::literals;

static bool baseline_interp;
static bool jit_compile;
static bool aot_compile;

// #define NAME __FUNCTION__
#define NAME std::string(extract_name(std::source_location::current().function_name()))

#define test(x) run_test(#x, x);

std::vector<std::string> passed_tests;
std::vector<std::string> failed_tests;

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
    println(cprint::fmt("Passed: ", cprint::BRIGHT_GREEN) + passed);
    println("Total: "s + total);
}

constexpr static std::string_view extract_name(const char *signature) {
    std::string_view view(signature);
    return view.substr(13, view.find_first_of('(') - 13);
}

static Context create_context(std::source_location loc = std::source_location::current()) {
    Context ctx;
    ctx.options.output_dir = "temp_files/";
    ctx.options.output_name = extract_name(loc.function_name());
    return ctx;
}

static int run(Context &ctx, ModuleBuilder *builder) {
    if (baseline_interp) {
        auto *interp = ctx.new_baseline_interp(builder);
        return interp->run();
    } else if (jit_compile) {
        auto *jit = ctx.new_jit_env(builder);
        return jit->run_function<i32 (*)()>("main");
    } else if (aot_compile) {
        auto bin = ctx.compile(builder);
        ctx.write_object_file(bin);
        ctx.link_objects();

#ifdef OS_WINDOWS
        ctx.options.output_dir.back() = '\\';
        std::string cmd = ".\\" + ctx.options.output_dir + ctx.options.output_name;
#else
        std::string cmd = "./" + ctx.options.output_dir + ctx.options.output_name;
#endif
        std::cout << cmd << "\n";
        return system(cmd.c_str()); // I'm lazy :(
    } else {
        assert(false);
    }

    return -1;
}

bool exit_success() {
    auto ctx = create_context();

    auto *builder = ctx.new_module_builder(NAME);
    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto ret = builder->iconst8(0);
    builder->ret(ret);

    auto result = run(ctx, builder);
    return result == 0;
}

bool exit_fail() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);
    auto *add = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto ret = builder->iconst8(-1);
    builder->ret(ret);

    auto result = run(ctx, builder);
    return result == -1;
}

bool iadd_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst64(20);
    auto imm2 = builder->iconst64(30);
    auto ret = builder->iadd(imm1, imm2);
    builder->ret(ret);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 50;
}

bool isub_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst64(20);
    auto imm2 = builder->iconst64(30);
    auto ret = builder->isub(imm1, imm2);
    builder->ret(ret);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == -10;
}

bool imul_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst64(20);
    auto imm2 = builder->iconst64(30);
    auto ret = builder->imul(imm1, imm2);
    builder->ret(ret);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 600;
}

bool idiv_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst64(20);
    auto imm2 = builder->iconst64(3);
    auto ret = builder->idiv(imm1, imm2);
    builder->ret(ret);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 6;
}

bool imod_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst64(20);
    auto imm2 = builder->iconst64(3);
    auto ret = builder->imod(imm1, imm2);
    builder->ret(ret);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 2;
}
bool fadd_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->fconst64(2.5);
    auto imm2 = builder->fconst64(2.5);
    auto ret_f = builder->fadd(imm1, imm2);
    auto ret = builder->f2i(ret_f);
    builder->ret(ret);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 5;
}

bool fsub_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->fconst64(2.5);
    auto imm2 = builder->fconst64(0.5);
    auto ret_f = builder->fsub(imm1, imm2);
    auto ret = builder->f2i(ret_f);
    builder->ret(ret);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 2;
}

bool fmul_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->fconst64(3);
    auto imm2 = builder->fconst64(2.5);
    auto ret_f = builder->fmul(imm1, imm2);
    auto ret = builder->f2i(ret_f);
    builder->ret(ret);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 7;
}

bool fdiv_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->fconst64(7.5);
    auto imm2 = builder->fconst64(3);
    auto ret_f = builder->fdiv(imm1, imm2);
    auto ret = builder->f2i(ret_f);
    builder->ret(ret);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 2;
}

bool lt() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1_ = builder->iconst32(7);
    auto imm2_ = builder->iconst32(5);
    auto imm1 = builder->id(imm1_);
    auto imm2 = builder->id(imm2_);
    auto ret_1 = builder->lt(imm2, imm1); // 5 < 7
    builder->ret(ret_1);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 1;
}

bool lt_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst32(7);
    auto imm2 = builder->iconst32(5);
    auto ret_1 = builder->lt(imm2, imm1); // 5 < 7
    builder->ret(ret_1);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 1;
}

bool lte() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1_ = builder->iconst32(7);
    auto imm2_ = builder->iconst32(5);
    auto imm1 = builder->id(imm1_);
    auto imm2 = builder->id(imm2_);
    auto ret_1 = builder->lte(imm2, imm1); // 5 <= 7
    auto ret_2 = builder->lte(imm1, imm1); // 7 <= 7
    auto ret_3 = builder->iadd(ret_1, ret_2);
    builder->ret(ret_3);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 2;
}

bool lte_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst32(7);
    auto imm2 = builder->iconst32(5);
    auto ret_1 = builder->lte(imm2, imm1); // 5 <= 7
    auto ret_2 = builder->lte(imm1, imm1); // 7 <= 7
    auto ret_3 = builder->iadd(ret_1, ret_2);
    builder->ret(ret_3);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 2;
}

bool gt() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1_ = builder->iconst32(7);
    auto imm2_ = builder->iconst32(5);
    auto imm1 = builder->id(imm1_);
    auto imm2 = builder->id(imm2_);
    auto ret_1 = builder->gt(imm1, imm2); // 7 > 5
    builder->ret(ret_1);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 1;
}

bool gt_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst32(7);
    auto imm2 = builder->iconst32(5);
    auto ret_1 = builder->gt(imm1, imm2); // 7 > 5
    builder->ret(ret_1);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 1;
}

bool gte() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1_ = builder->iconst32(7);
    auto imm2_ = builder->iconst32(5);
    auto imm1 = builder->id(imm1_);
    auto imm2 = builder->id(imm2_);
    auto ret_1 = builder->gte(imm1, imm2); // 5 >= 7
    auto ret_2 = builder->gte(imm1, imm1); // 7 >= 7
    auto ret_3 = builder->iadd(ret_1, ret_2);
    builder->ret(ret_3);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 2;
}

bool gte_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst32(7);
    auto imm2 = builder->iconst32(5);
    auto ret_1 = builder->gte(imm1, imm2); // 5 >= 7
    auto ret_2 = builder->gte(imm1, imm1); // 7 >= 7
    auto ret_3 = builder->iadd(ret_1, ret_2);
    builder->ret(ret_3);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)()>("add_imm");
    auto result = run(ctx, builder);
    return result == 2;
}

bool eq() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1_ = builder->iconst32(7);
    auto imm1 = builder->id(imm1_);
    auto ret_1 = builder->eq(imm1, imm1);
    builder->ret(ret_1);

    auto result = run(ctx, builder);
    return result == 1;
}

bool eq_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst32(7);
    auto ret_1 = builder->eq(imm1, imm1);
    builder->ret(ret_1);

    auto result = run(ctx, builder);
    return result == 1;
}

bool bsl() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1_ = builder->iconst32(3);
    auto imm1 = builder->id(imm1_);
    auto ret_1 = builder->bsl(imm1, imm1);
    builder->ret(ret_1);

    auto result = run(ctx, builder);
    return result == (3 << 3);
}

bool bsl_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst32(3);
    auto ret_1 = builder->bsl(imm1, imm1);
    builder->ret(ret_1);

    auto result = run(ctx, builder);
    return result == (3 << 3);
}

bool bsr() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1_ = builder->iconst32(27);
    auto imm1 = builder->id(imm1_);
    auto imm2_ = builder->iconst32(3);
    auto imm2 = builder->id(imm2_);
    auto ret_1 = builder->bsr(imm1, imm2);
    builder->ret(ret_1);

    auto result = run(ctx, builder);
    return result == (27 >> 3);
}

bool bsr_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst32(27);
    auto imm2 = builder->iconst32(3);
    auto ret_1 = builder->bsr(imm1, imm2);
    builder->ret(ret_1);

    auto result = run(ctx, builder);
    return result == (27 >> 3);
}

bool band() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1_ = builder->iconst32(24);
    auto imm1 = builder->id(imm1_);
    auto imm2_ = builder->iconst32(7);
    auto imm2 = builder->id(imm2_);
    auto ret_1 = builder->band(imm1, imm2);
    builder->ret(ret_1);

    auto result = run(ctx, builder);
    return result == (24 & 7);
}

bool band_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst32(24);
    auto imm2 = builder->iconst32(7);
    auto ret_1 = builder->band(imm1, imm2);
    builder->ret(ret_1);

    auto result = run(ctx, builder);
    return result == (24 & 7);
}

bool bor() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1_ = builder->iconst32(24);
    auto imm1 = builder->id(imm1_);
    auto imm2_ = builder->iconst32(7);
    auto imm2 = builder->id(imm2_);
    auto ret_1 = builder->bor(imm1, imm2);
    builder->ret(ret_1);

    auto result = run(ctx, builder);
    return result == (24 | 7);
}

bool bor_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst32(24);
    auto imm2 = builder->iconst32(7);
    auto ret_1 = builder->bor(imm1, imm2);
    builder->ret(ret_1);

    auto result = run(ctx, builder);
    return result == (24 | 7);
}

bool bxor() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1_ = builder->iconst32(24);
    auto imm1 = builder->id(imm1_);
    auto imm2_ = builder->iconst32(7);
    auto imm2 = builder->id(imm2_);
    auto ret_1 = builder->bxor(imm1, imm2);
    builder->ret(ret_1);

    auto result = run(ctx, builder);
    return result == (24 ^ 7);
}

bool bxor_imm() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *f = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst32(24);
    auto imm2 = builder->iconst32(7);
    auto ret_1 = builder->bxor(imm1, imm2);
    builder->ret(ret_1);

    auto result = run(ctx, builder);
    return result == (24 ^ 7);
}


bool call() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *zero = builder->newFn("zero", {}, Type::i64, CallConv::win64);
    auto zero_ret = builder->iconst8(100);
    builder->ret(zero_ret);

    auto *one = builder->newFn("one", {}, Type::i64, CallConv::win64);
    auto one_ret = builder->iconst8(1);
    builder->ret(one_ret);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto main_ret = builder->call(one, {});
    builder->ret(main_ret);

    auto result = run(ctx, builder);
    return result == 1;
}

bool add() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);
    auto *add = builder->newFn("add", {Type::i64, Type::i64}, Type::i64, CallConv::win64);
    auto ret = builder->iadd(add->param(0), add->param(1));
    builder->ret(ret);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst64(5);
    auto imm2 = builder->iconst64(6);
    auto main_ret = builder->call(add, {imm1, imm2});
    builder->ret(main_ret);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)(i64, i64)>("add", 5, 6);
    auto result = run(ctx, builder);
    std::cout << result << '\n';
    return result == 11;
}

bool add4() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);
    auto *add = builder->newFn("add4", {Type::i64, Type::i64, Type::i64, Type::i64}, Type::i64, CallConv::win64);
    auto res1 = builder->iadd(add->param(0), add->param(1));
    auto res2 = builder->iadd(add->param(2), add->param(3));
    auto res3 = builder->iadd(res1, res2);
    builder->ret(res3);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto imm1 = builder->iconst64(12);
    auto imm2 = builder->iconst64(8);
    auto imm3 = builder->iconst64(20);
    auto imm4 = builder->iconst64(15);
    auto main_ret = builder->call(add, {imm1, imm2, imm3, imm4});
    builder->ret(main_ret);

    // auto *jit = ctx.new_jit_env(builder);
    // auto result = jit->run_function<i64 (*)(i64, i64, i64, i64)>("add4", 12, 8, 20, 15);
    auto result = run(ctx, builder);
    return result == 55;
}

bool slot_i32() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto ret_ptr = builder->slot(Type::i32);
    builder->stack_store(ret_ptr, IRConstantInt(100, 32));
    auto ret = builder->stack_load(ret_ptr, Type::i32);
    builder->ret(ret);

    auto result = run(ctx, builder);
    return result == 100;
}

bool slot_i64_1() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto ret_ptr = builder->slot(Type::i64);
    builder->stack_store(ret_ptr, IRConstantInt(0xf0000000001, 64));
    auto ret = builder->stack_load(ret_ptr, Type::i64);
    // TODO trunc instruction, basically noop here
    builder->ret(ret);

    auto result = run(ctx, builder);
    return result == 1;
}

bool slot_i64_2() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto lhs_ptr = builder->slot(Type::i64);
    auto rhs_ptr = builder->slot(Type::i64);
    builder->stack_store(lhs_ptr, IRConstantInt(40, 64));
    builder->stack_store(rhs_ptr, IRConstantInt(2, 64));
    auto lhs = builder->stack_load(lhs_ptr, Type::i64);
    auto rhs = builder->stack_load(rhs_ptr, Type::i64);
    auto ret = builder->iadd(lhs, rhs);
    // TODO trunc instruction, basically noop here
    builder->ret(ret);

    auto result = run(ctx, builder);
    return result == 42;
}

bool branches_1() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto *first = builder->newBB("first");
    auto *second = builder->newBB("second");

    builder->setInsertPoint(entry);
    builder->br(first);

    builder->setInsertPoint(first);
    auto first_ret = builder->iconst8(100);
    builder->ret(first_ret);

    builder->setInsertPoint(second);
    auto second_ret = builder->iconst32(300);
    builder->ret(second_ret);

    auto result = run(ctx, builder);
    std::cout << result;
    return result == 100;
}

bool branches_2() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto *first = builder->newBB("first");
    auto *second = builder->newBB("second");

    builder->setInsertPoint(entry);
    builder->br(second);

    builder->setInsertPoint(first);
    auto first_ret = builder->iconst8(100);
    builder->ret(first_ret);

    builder->setInsertPoint(second);
    auto second_ret = builder->iconst32(300);
    builder->ret(second_ret);

    auto result = run(ctx, builder);
    return result == 300;
}

bool branches_3() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto *first = builder->newBB("first");
    auto *second = builder->newBB("second");

    builder->setInsertPoint(entry);
    auto s = builder->slot(Type::i32);
    builder->stack_store(s, builder->iconst8(1));
    auto cond = builder->stack_load(s, Type::i32);
    builder->brz(cond, first, second);

    builder->setInsertPoint(first);
    auto first_ret = builder->iconst8(100);
    builder->ret(first_ret);

    builder->setInsertPoint(second);
    auto second_ret = builder->iconst32(300);
    builder->ret(second_ret);

    auto result = run(ctx, builder);
    return result == 300;
}

bool branches_4() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto *first = builder->newBB("first");
    auto *second = builder->newBB("second");

    builder->setInsertPoint(entry);
    auto s = builder->slot(Type::i32);
    builder->stack_store(s, builder->iconst8(1));
    auto cond = builder->stack_load(s, Type::i32);
    builder->brnz(cond, first, second);

    builder->setInsertPoint(first);
    auto first_ret = builder->iconst8(100);
    builder->ret(first_ret);

    builder->setInsertPoint(second);
    auto second_ret = builder->iconst32(300);
    builder->ret(second_ret);

    auto result = run(ctx, builder);
    return result == 100;
}

bool branches_5() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto *first = builder->newBB("first");
    auto *second = builder->newBB("second");
    auto *last = builder->newBB("last");

    builder->setInsertPoint(entry);
    auto s = builder->slot(Type::i32);
    builder->brnz(builder->iconst32(1), first, second);

    builder->setInsertPoint(first);
    builder->stack_store(s, builder->iconst8(100));
    builder->br(last);

    builder->setInsertPoint(second);
    builder->stack_store(s, builder->iconst8(42));
    builder->br(last);

    builder->setInsertPoint(last);
    auto ret = builder->stack_load(s, Type::i32);
    builder->ret(ret);

    auto result = run(ctx, builder);
    return result == 100;
}

bool branches_6() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto *first = builder->newBB("first");
    auto *second = builder->newBB("second");
    auto *last = builder->newBB("last");

    builder->setInsertPoint(entry);
    auto s = builder->slot(Type::i32);
    builder->stack_store(s, builder->iconst8(100));
    auto cond = builder->id(builder->iconst32(1));
    builder->brnz(cond, first, second);

    builder->setInsertPoint(first);
    builder->br(last);

    builder->setInsertPoint(second);
    builder->stack_store(s, builder->iconst8(42));
    builder->br(last);

    builder->setInsertPoint(last);
    auto ret = builder->stack_load(s, Type::i32);
    builder->ret(ret);

    auto result = run(ctx, builder);
    return result == 100;
}

bool branches_7() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto *last = builder->newBB("last");

    builder->setInsertPoint(entry);
    auto s = builder->slot(Type::i32);
    builder->stack_store(s, builder->iconst8(100));
    builder->br(last);

    builder->setInsertPoint(last);
    auto ret = builder->stack_load(s, Type::i32);
    builder->ret(ret);

    auto result = run(ctx, builder);
    return result == 100;
}

bool branches_8() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto *first = builder->newBB("first");
    auto *second = builder->newBB("second");
    auto *last = builder->newBB("last");

    builder->setInsertPoint(entry);
    auto s = builder->slot(Type::i32);
    builder->stack_store(s, builder->iconst8(100));
    builder->brz(builder->iconst8(0), first, second);

    builder->setInsertPoint(first);
    builder->br(last);
    builder->setInsertPoint(second);
    builder->br(last);

    builder->setInsertPoint(last);
    auto ret = builder->stack_load(s, Type::i32);
    builder->ret(ret);

    auto result = run(ctx, builder);
    return result == 100;
}

bool phi_1() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto *first = builder->newBB("first");
    auto *second = builder->newBB("second");
    auto *leave = builder->newBB("leave");

    builder->setInsertPoint(entry);
    builder->br(first);

    builder->setInsertPoint(first);
    auto first_ret = builder->iconst8(100);
    builder->br(leave);

    builder->setInsertPoint(second);
    auto second_ret = builder->iconst32(300);
    builder->br(leave);

    builder->setInsertPoint(leave);
    auto leave_ret = builder->phi({{first, first_ret}, {second, second_ret}});
    builder->ret(leave_ret);

    auto result = run(ctx, builder);
    return result == 100;
}

bool phi_2() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto *first = builder->newBB("first");
    auto *second = builder->newBB("second");
    auto *leave = builder->newBB("leave");

    builder->setInsertPoint(entry);
    builder->br(second);

    builder->setInsertPoint(first);
    auto first_ret = builder->iconst8(100);
    builder->br(leave);

    builder->setInsertPoint(second);
    auto second_ret = builder->iconst32(300);
    builder->br(leave);

    builder->setInsertPoint(leave);
    auto leave_ret = builder->phi({{first, first_ret}, {second, second_ret}});
    builder->ret(leave_ret);

    auto result = run(ctx, builder);
    return result == 300;
}

bool phi_3() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto *first = builder->newBB("first");
    auto *second = builder->newBB("second");
    auto *leave = builder->newBB("leave");

    builder->setInsertPoint(entry);
    builder->br(second);

    builder->setInsertPoint(first);
    auto first_ret_const = builder->iconst32(100);
    auto first_ret = builder->id(first_ret_const);
    builder->br(leave);

    builder->setInsertPoint(second);
    auto second_ret_const = builder->iconst32(300);
    auto second_ret = builder->id(second_ret_const);
    builder->br(leave);

    builder->setInsertPoint(leave);
    auto leave_ret = builder->phi({{first, first_ret}, {second, second_ret}});
    builder->ret(leave_ret);

    auto result = run(ctx, builder);
    return result == 300;
}

bool phi_4() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto *then = builder->newBB("then");
    auto *cont = builder->newBB("cont");

    builder->setInsertPoint(entry);
    auto x = builder->id(builder->iconst32(17));
    auto y = builder->id(builder->iconst32(6));
    auto z = builder->id(builder->iconst32(0));
    auto cond = builder->lt(y, z);
    builder->brz(cond, then, cont);

    builder->setInsertPoint(then);
    auto then_z = builder->iadd(x, y);
    builder->br(cont);

    builder->setInsertPoint(cont);
    auto new_z = builder->phi({{entry, z}, {then, then_z}});
    auto ret = builder->iadd(new_z, x);
    builder->ret(ret);

    auto result = run(ctx, builder);
    return result == 40;
}

bool phi_5() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *b0 = builder->newBB("b0");
    auto *b1 = builder->newBB("b1");
    auto *b2 = builder->newBB("b2");
    auto *b3 = builder->newBB("b3");
    auto *b4 = builder->newBB("b4");

    builder->setInsertPoint(b0);
    auto a = builder->id(builder->iconst32(34));
    auto b = builder->id(builder->iconst32(8));
    builder->br(b1);

    builder->setInsertPoint(b1);
    auto x = builder->phi({{b0, a}});
    auto y = builder->phi({{b0, b}});
    auto phi1 = builder->insts().end()[-2];
    phi1->values.push_back({b3, y});
    auto phi2 = builder->insts().end()[-1];
    phi2->values.push_back({b3, x});

    auto sum1 = builder->iadd(x, y);
    auto cond1 = builder->id(builder->iconst32(0));
    builder->brz(cond1, b2, b3);

    builder->setInsertPoint(b2);
    auto new_x = builder->phi({{b1, y}, {b3, builder->iconst32(100)}});
    auto new_y = builder->phi({{b1, x}, {b3, builder->iconst32(200)}});
    auto sum2 = builder->iadd(new_x, new_y);
    auto cond2 = builder->id(builder->iconst32(0));
    builder->brz(cond2, b3, b1);

    builder->setInsertPoint(b3);
    auto ret = builder->phi({{b1, sum1}, {b2, sum2}});
    auto cond3 = builder->id(builder->iconst32(0));
    builder->brz(cond3, b4, b2);

    builder->setInsertPoint(b4);
    builder->ret(ret);

    auto result = run(ctx, builder);
    std::cout << result << "\n";
    return result == 42;
}

bool phi_6() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *b1 = builder->newBB("b1");
    auto *b2 = builder->newBB("b2");
    auto *b3 = builder->newBB("b3");

    builder->setInsertPoint(b1);
    auto x1 = builder->id(builder->iconst32(0));
    builder->br(b2);

    builder->setInsertPoint(b2);
    auto x2 = builder->phi({{b1, x1}});
    auto x3 = builder->iadd(x2, builder->iconst8(1));
    auto phi2 = builder->insts().end()[-2];
    phi2->values.push_back({b2, x3});
    auto max = builder->id(builder->iconst32(5));
    auto cond = builder->lt(x2, max);
    // builder->brz(cond, b3, b2);
    builder->brnz(cond, b2, b3);

    builder->setInsertPoint(b3);
    builder->ret(x2);

    auto result = run(ctx, builder);
    std::cout << result << "\n";
    return result == 5;
}

bool dce_1() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *b1 = builder->newBB("b1");
    auto *b2 = builder->newBB("b2");
    auto *b3 = builder->newBB("b3");

    builder->setInsertPoint(b1);
    auto x1 = builder->id(builder->iconst32(42));
    builder->br(b3);

    builder->setInsertPoint(b2);
    builder->br(b3);

    builder->setInsertPoint(b3);
    auto x2 = builder->phi({{b1, x1}, {b2, builder->iconst8(-10)}});
    builder->ret(x2);

    auto result = run(ctx, builder);
    std::cout << result << "\n";
    return result == 42;
}

bool dce_2() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *b1 = builder->newBB("b1");
    auto *b2 = builder->newBB("b2");
    auto *b3 = builder->newBB("b3");

    builder->setInsertPoint(b1);
    auto x1 = builder->id(builder->iconst32(42));
    auto cond = builder->id(builder->iconst8(1));
    builder->brz(cond, b2, b3);

    builder->setInsertPoint(b2);
    auto x2 = builder->id(builder->iconst8(-10));
    builder->br(b3);

    builder->setInsertPoint(b3);
    auto ret = builder->phi({{b1, x1}, {b2, x2}});
    builder->ret(ret);

    auto result = run(ctx, builder);
    std::cout << result << "\n";
    return result == 42;
}

bool cprop_1() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *b1 = builder->newBB("b1");

    builder->setInsertPoint(b1);
    auto x1 = builder->id(builder->iconst32(6));
    auto x2 = builder->iadd(builder->iconst8(1), builder->iconst8(3));
    auto ret = builder->iadd(x1, x2);
    builder->ret(ret);

    auto result = run(ctx, builder);
    std::cout << result << "\n";
    return result == 10;
}

bool gvn_1() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *b1 = builder->newBB("b1");
    auto *b2 = builder->newBB("b2");
    auto *b3 = builder->newBB("b3");

    builder->setInsertPoint(b1);
    auto x1 = builder->id(builder->iconst32(6));
    auto x2 = builder->id(builder->iconst8(4));
    auto ret1 = builder->iadd(x1, x2);
    auto ret2 = builder->iadd(x1, x2);
    builder->br(b2);

    builder->setInsertPoint(b2);
    auto x3 = builder->id(builder->iconst8(4));
    auto ret3 = builder->iadd(x1, x3);
    auto ret4 = builder->iadd(x1, x2);
    auto t = builder->isub(x1, x2);
    builder->br(b3);
    
    builder->setInsertPoint(b3);
    auto phitemp = builder->phi({{b2, ret4}});
    auto ret5 = builder->iadd(ret1, ret2);
    auto ret6 = builder->iadd(ret3, ret4);
    auto ret7 = builder->iadd(ret5, ret6);
    builder->ret(ret7);

    auto result = run(ctx, builder);
    std::cout << result << "\n";
    return result == 40;
}

bool peephole_1() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *fn = builder->newFn("fn", {Type::i32}, Type::i32, CallConv::win64, false);
    auto *b1 = builder->newBB("b1");
    auto *b2 = builder->newBB("b2");
    auto *b3 = builder->newBB("b3");

    builder->setInsertPoint(b1);
    auto x1 = fn->param(0);
    auto zero = builder->id(builder->iconst8(0));
    auto one = builder->id(builder->iconst8(1));
    auto two = builder->id(builder->iconst8(2));
    auto r1 = builder->iadd(x1, zero);
    auto r2 = builder->isub(r1, zero);
    auto r3 = builder->imul(r2, zero);
    auto r4 = builder->idiv(x1, one);
    builder->brz(r4, b2, b3);

    builder->setInsertPoint(b2);
    auto r5 = builder->bsl(x1, zero);
    auto r6 = builder->bsr(x1, zero);
    auto r7 = builder->band(x1, x1);
    auto r8 = builder->bor(x1, x1);
    builder->br(b3);
    
    builder->setInsertPoint(b3);
    auto phi = builder->phi({{b1, x1}, {b2, one}});
    auto mul = builder->imul(phi, two);
    builder->ret(mul);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry");
    auto res = builder->call(fn, {builder->iconst32(42)});
    builder->ret(res);

    auto result = run(ctx, builder);
    std::cout << result << "\n";
    return result == 84;
}

bool licm_1() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);
    auto *fn = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *entry = builder->newBB("entry", LoopInfo::entry, 1);
    auto *cond = builder->newBB("cond", LoopInfo::cond, 1);
    auto *body = builder->newBB("body", LoopInfo::body, 1);
    auto *inc = builder->newBB("inc", LoopInfo::inc, 1);
    auto *cont = builder->newBB("cont", LoopInfo::exit, 1);
    builder->setInsertPoint(entry);
    auto init = builder->id(builder->iconst8(0));
    auto inv1 = builder->id(builder->iconst8(15));
    auto inv2 = builder->id(builder->iconst8(5));
    builder->br(cond);
    builder->setInsertPoint(cond);
    auto ind = builder->phi({{entry, init}});
    auto phi_1 = builder->insts().end()[-1];
    auto sum = builder->phi({{entry, builder->iconst8(0)}});
    auto phi_2 = builder->insts().end()[-1];
    auto sent = builder->lt(ind, builder->iconst8(10));
    builder->brnz(sent, body, cont);
    
    builder->setInsertPoint(body);
    auto new_sum = builder->iadd(sum, ind);
    auto inv = builder->iadd(inv1, inv2);
    builder->br(inc);
    builder->setInsertPoint(inc);
    auto new_ind = builder->iadd(ind, builder->iconst8(1));
    builder->br(cond);
    phi_1->values.push_back({inc, new_ind});
    phi_2->values.push_back({inc, new_sum});
    builder->setInsertPoint(cont);
    auto res = builder->iadd(new_sum, inv);
    builder->ret(res);
    auto result = run(ctx, builder);
    std::cout << result << "\n";
    return result == 65;
}

bool inline_1() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *add = builder->newFn("add", {Type::i32, Type::i32}, Type::i32, CallConv::win64, true);
    add->always_inline = true;
    auto res = builder->iadd(add->param(0), add->param(1));
    builder->ret(res);
    
    auto *fn = builder->newFn("main", {}, Type::i32, CallConv::win64, true);
    auto main_res = builder->call(add, {builder->iconst32(3), builder->iconst32(39)});
    builder->ret(main_res);

    auto result = run(ctx, builder);
    std::cout << result << "\n";
    return result == 42;
}

bool inline_2() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *pred = builder->newFn("pred", {Type::i32}, Type::i32, CallConv::win64, false);
    pred->always_inline = true;
    auto *b1 = builder->newBB("b1");
    auto *b2 = builder->newBB("b2");
    auto *b3 = builder->newBB("b3");
    auto *b4 = builder->newBB("b4");
    builder->setInsertPoint(b1);
    auto cond = builder->eq(pred->param(0), builder->iconst8(0));
    builder->brnz(cond, b2, b3);
    builder->setInsertPoint(b2);
    auto t1 = builder->id(builder->iconst8(10));
    builder->br(b4);
    builder->setInsertPoint(b3);
    auto t2 = builder->isub(pred->param(0), builder->iconst8(1));
    builder->br(b4);
    builder->setInsertPoint(b4);
    auto pred_res = builder->phi({{b2, t1}, {b3, t2}});
    builder->ret(pred_res);

    auto *func = builder->newFn("func", {Type::i32}, Type::i32, CallConv::win64, true);
    func->always_inline = true;
    auto p1 = builder->isub(func->param(0), builder->iconst8(2));
    auto c1 = builder->call(pred, {p1});
    auto p2 = builder->iadd(func->param(0), builder->iconst8(1));
    auto c2 = builder->call(pred, {p2});
    auto c3 = builder->call(pred, {builder->iconst32(0)});
    auto func_res1 = builder->iadd(c1, c2);
    auto func_res2 = builder->iadd(func_res1, c3);
    builder->ret(func_res2);
    
    auto *fn = builder->newFn("main", {}, Type::i32, CallConv::win64, false);
    auto *fn_entry = builder->newBB("main_entry");
    auto main_res = builder->call(func, {builder->iconst32(3)});
    builder->ret(main_res);

    auto result = run(ctx, builder);
    std::cout << result << "\n";
    return result == 13;
}

int main(int argc, char *argv[]) {
    // TODO take from args
    // baseline_interp = true;
    // jit_compile = true;
    aot_compile = true;

    // test(exit_success);
    // test(exit_fail);
    
    // test(iadd_imm);
    // test(isub_imm);
    // test(imul_imm);
    // test(idiv_imm);
    // test(imod_imm);
    
    // test(fadd_imm);
    // test(fsub_imm);
    // test(fmul_imm);
    // test(fdiv_imm);

    // test(lt);
    // test(lt_imm);
    // test(lte);
    // test(lte_imm);
    // test(gt);
    // test(gt_imm);
    // test(gte);
    // test(gte_imm);
    // test(eq);
    // test(eq_imm);
    
    // /* test(bsl); */
    test(bsl_imm);
    // / * test(bsr); */
    test(bsr_imm);
    test(band);
    test(band_imm);
    test(bor);
    test(bor_imm);
    test(bxor);
    test(bxor_imm);

    // test(call);
    // test(add);
    // test(add4);

    // test(slot_i32);
    // test(slot_i64_1);
    // test(slot_i64_2);

    // test(branches_1);
    // test(branches_2);
    // test(branches_3);
    // test(branches_4);
    // test(branches_5);
    // test(branches_6);
    // test(branches_7);
    // test(branches_8);

    // test(phi_1);
    // test(phi_2);
    // test(phi_3);
    // test(phi_4);
    // test(phi_5);
    // test(phi_6);

    // test(dce_1);
    // test(dce_2);

    // test(cprop_1);
    // test(gvn_1);
    // test(peephole_1);
    // test(licm_1);
    // test(inline_1);
    // test(inline_2);


    print_report();
}
