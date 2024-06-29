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

bool salloc_i32() {
    Context ctx = create_context();
    auto *builder = ctx.new_module_builder(NAME);

    auto *main = builder->newFn("main", {}, Type::i32, CallConv::win64);
    auto ret_ptr = builder->salloc(Type::i32);
    builder->store(ret_ptr, IRConstantInt(100, 32, true));
    auto ret = builder->load(ret_ptr, Type::i32);
    builder->ret(ret);

    auto result = run(ctx, builder);
    return result == 100;
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
    auto second_ret = builder->iconst8(300);
    builder->ret(second_ret);

    auto result = run(ctx, builder);
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

int main(int argc, char *argv[]) {
    // TODO take from args
    baseline_interp = true;
    // jit_compile = true;
    // aot_compile = true;

    test(exit_success);
    test(exit_fail);
    
    test(iadd_imm);
    test(isub_imm);
    test(imul_imm);
    test(idiv_imm);
    test(imod_imm);
    
    test(fadd_imm);
    test(fsub_imm);
    test(fmul_imm);
    test(fdiv_imm);
    
    test(call);
    test(add);
    test(add4);

    test(salloc_i32);

    test(branches_1);
    test(branches_2);

    test(phi_1);
    test(phi_2);

    print_report();
}
