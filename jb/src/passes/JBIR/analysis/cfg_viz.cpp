#include "JBIR/analysis/cfg_viz.h"

#include <map>
#include <utility>
#include <iostream>
#include <fstream>
#include "pretty_print.h"

using namespace jb;

static std::string name(BasicBlock *b) {
    return "BB_" + b->id;
}

static void visit_block(std::ofstream &out, BasicBlock *b) {
    out << "\t" << name(b) << " [shape=record,label=\"{" << b->id << ":\\l| ";
    for(auto *i: b->insts) {
        std::string stri = str(i);
        out << str(i) << "\\l";
    }
    out << "}\"];\n";
    for(auto *s: b->succ) {
        out << "\t" << name(b) << " -> " << name(s) << ";\n";
        visit_block(out, s);
    }
}

void CFGViz::run_pass(Function* function) {
    static int count = 0;
    std::string filename = "temp_files/"+function->id+std::to_string(count++)+".dot";
    std::ofstream out(filename);
    
    out << "digraph {\n";
    visit_block(out, function->blocks[0]);
    out << "}";
    out.close();
    
    std::string cmd = "dot -Tsvg " + filename + " -o " + filename.substr(0, filename.size()-4) + ".svg";
    std::cout << cmd << "\n";
    system(cmd.c_str());
}

void CFGViz::run_pass(Module* module) {
    for(auto *fn: module->functions) {
        CFGViz::run_pass(fn);
    }
}
