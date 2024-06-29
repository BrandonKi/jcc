#pragma once

#include "jb.h"

#include "module_builder.h"
#include "baseline_interp.h"
#include "jit_env.h"
#include "arch/x86_64/mcir_gen.h"

namespace jb {

class Context {
public:
    CompileOptions options;
    std::vector<std::string> object_files;

    Context() : options{}, object_files{} {}
    Context(CompileOptions options) : options{options}, object_files{} {}

    ModuleBuilder *new_module_builder(std::string);

    BinaryFile *compile(ModuleBuilder *);

    void write_object_file(BinaryFile *);
    void link_objects();

    JITEnv *new_jit_env(ModuleBuilder *, CompileOptions = {});

    Interp *new_baseline_interp(ModuleBuilder *, CompileOptions = {});
};

} // namespace jb
