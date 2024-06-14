#pragma once

#include "parser.h"

namespace jcc {

// TODO make these flags
// c++ doesn't make structs like these too user friendly
struct CompileOptions {
    bool link = false;
    bool run_exe = false;
    bool print_ir = false;
    bool print_link_command = false;
};

class Compiler {
    Parser m_parser;
    CompileOptions m_options;

public:
    Compiler();
    Compiler(CompileOptions);

    int compile_file(InputFile);
    int compile_string(std::string text);
    int compile(std::string filepath);

private:
    std::string read_file(const std::string &);
};

} // namespace jcc
