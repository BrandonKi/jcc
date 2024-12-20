#include "compiler.h"

#include "jb_ir_gen.h"

#include "llvm_ir_gen.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

#include "platform.h"
#include "link/windows_pe.h"
#include "sema.h"
#include "parser.h"

#include <fstream>

using namespace jcc;

Compiler::Compiler() : m_parser{}, m_options{} {}

Compiler::Compiler(CompileOptions options) : m_parser{}, m_options{options} {}

int Compiler::compile_file(InputFile infile) {
    JCC_PROFILE();

    Platform::init();

    auto lexer = Lexer(InputFile{infile.filepath, infile.text});
    m_parser = Parser(&lexer); // FIXME ownership
    // lexer.lexer__debug_dump();
    FileNode *file = m_parser.parse_file();

    Sema sema;
    sema.run_on(file);

    // if(false) {
    if(true) {
        LLVMIRGen ir_gen;
        ir_gen.gen_file(file);

        if (m_options.print_ir) {
            ir_gen.m_module->print(llvm::outs(), nullptr);
        }

        if (llvm::verifyModule(*ir_gen.m_module, &llvm::errs())) {
            llvm::errs() << "Error: module verification failed\n";
            return 1;
        }

        auto TargetTriple = llvm::sys::getDefaultTargetTriple();

        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        std::string Error;
        auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

        if (!Target) {
            llvm::errs() << Error;
            return 1;
        }

        auto CPU = "generic";
        auto Features = "";

        llvm::TargetOptions opt;
        auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, llvm::Reloc::PIC_);

        ir_gen.m_module->setDataLayout(TargetMachine->createDataLayout());
        ir_gen.m_module->setTargetTriple(TargetTriple);

        auto Filename = "temp_files/test.obj";
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

        if (EC) {
            llvm::errs() << "Could not open file: " << EC.message();
            return 1;
        }

        llvm::legacy::PassManager pass;
        auto FileType1 = llvm::CodeGenFileType::CGFT_ObjectFile;

        if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType1)) {
            llvm::errs() << "TargetMachine can't emit a file of this type";
            return 1;
        }

        pass.run(*ir_gen.m_module);
        dest.close();

    } else {
        JBIRGen ir_gen;
        ir_gen.gen_file(file);

        ir_gen.m_context.options.output_dir = "temp_files/";
        ir_gen.m_context.options.output_name = "test.obj";

        if (m_options.print_ir) {
            ir_gen.m_builder->module->print();
        }
        
        // auto interp = ir_gen.m_context.new_baseline_interp(ir_gen.m_builder);
        // int res = interp->run();
        // std::cout << res << "\n\n";
        // return res;
        auto bin = ir_gen.m_context.compile(ir_gen.m_builder);
        ir_gen.m_context.write_object_file(bin);
        ir_gen.m_context.link_objects();
    }

    if (m_options.link) {
        jb::link_coff_files("temp_files/test", {"C:/Users/Kirin/OneDrive/Desktop/dev/jcc/temp_files/test.obj"},
                            m_options.print_link_command);
    }

    // FIXME lazy
    // should use os-specific version and redirect stdout
    // learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
    if (m_options.run_exe) {
        int res = system("C:/Users/Kirin/OneDrive/Desktop/dev/jcc/temp_files/test.exe");
        return res;
    }

    return 0;
}

int Compiler::compile_string(std::string text) {
    JCC_PROFILE();

    return compile_file(InputFile{"<string>", text});
}

int Compiler::compile(std::string filepath) {
    JCC_PROFILE();

    return this->compile_file(InputFile{filepath, read_file(filepath)});
}
