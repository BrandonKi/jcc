#include "context.h"

#include "link/windows_pe.h"
#include "link/coff.h"

#include "arch/x86_64/mdf.h"
#include "register_manager.h"
#include "register_allocator.h"

#include "baseline_interp.h"

#include "pretty_print.h"
//TODO temporary
#include "arch/x86_64/pretty_print.h"

// TODO use an allocator for all of this in the future

using namespace jb;

// TODO keep track of all ModuleBuilders created and add them
// to an internal list so we don't have to pass them around
ModuleBuilder *Context::new_module_builder(std::string name) {
    return new ModuleBuilder(std::move(name));
}

// TODO move this into a Compiler class/file
// there will be too much logic here in the future
BinaryFile *Context::compile(ModuleBuilder *builder) {
    //	auto bin_file = new BinaryFile {builder->module->name};
    BinaryFile *bin_file;
    //	std::vector<byte> bin;
    if (options.target_arch == Arch::x64) {
        pretty_print(builder->module);

        x86_64::MCIRGen mcir_gen(options, builder->module);
        mcir_gen.compile();
        x86_64::pretty_print(mcir_gen.machine_module);

        auto mng = x86_64::register_manager();
        RegisterAllocator reg_alloc(mng);
        reg_alloc.alloc(mcir_gen.machine_module);
        x86_64::pretty_print(mcir_gen.machine_module);

        bin_file = mcir_gen.emit_bin();
    }

    // TODO move all of this into mcir_gen probably
    // mcir_gen.emit_bin() should return a BinaryFile*
    // and do all of this
    // especially because it will know more about the target arch
    // hmm, although I guess it also depends on OS a little bit
    //	bin_file->sections.push_back(Section {
    //		.name = ".text",
    //		.virtual_size = 0,
    //		.virtual_address = 0,
    //		.relocs = {},
    //		.bin = bin,
    //	});

    // TODO should be adding all the function names here
    // and their corresponding sections
    //	bin_file->symbols.push_back(Symbol{".text", SymbolType::internal});
    //	bin_file->symbols.push_back(Symbol{"@feat.00", SymbolType::internal});
    //	bin_file->symbols.push_back(Symbol{"main", SymbolType::function});

    return bin_file;
}

void Context::write_object_file(BinaryFile *bin_file) {

    if (options.obj_type == ObjType::coff) {
        auto obj_file = to_coff(bin_file);
        auto path = options.output_dir + bin_file->name + ".obj";

        object_files.push_back(path);
        obj_file.serialize(path);
    } else {
        assert(false);
    }
}

void Context::link_objects() {
    link_coff_files(options.output_dir + options.output_name, object_files);
}

JITEnv *Context::new_jit_env(ModuleBuilder *builder, CompileOptions options) {

    std::vector<byte> bin;
    if (options.target_arch == Arch::x64) {
        pretty_print(builder->module);

        x86_64::MCIRGen mcir_gen(options, builder->module);
        mcir_gen.compile();

        auto mng = x86_64::register_manager();
        RegisterAllocator reg_alloc(mng);
        reg_alloc.alloc(mcir_gen.machine_module);
        // reg_alloc.alloc(mcir_gen.machine_module->functions[0]);

        bin = mcir_gen.emit_raw_bin();
    }

    return new JITEnv(bin);
}

Interp *Context::new_baseline_interp(ModuleBuilder *builder, CompileOptions options) {
    pretty_print(builder->module);

    Interp *interp = new Interp(options, builder->module);
    return interp;
}