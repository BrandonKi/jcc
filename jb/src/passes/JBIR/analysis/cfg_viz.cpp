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

static void visit_block(std::fstream &out, BasicBlock *b, std::unordered_set<BasicBlock*> &visited) {
    visited.insert(b);
    out << "\t" << name(b) << " [shape=record,label=\"{" << b->id << ":\\l| ";
    for(auto *i: b->insts) {
        std::string stri = str(i);
        out << str(i) << "\\l";
    }
    out << "}\"];\n";
    for(auto *s: b->succ) {
        out << "\t" << name(b) << " -> " << name(s) << ";\n";
        if(!visited.contains(s)) {
            visit_block(out, s, visited);
        }
    }
}

static void visit_dead_blocks(std::fstream &out, std::vector<BasicBlock*> bbs, std::unordered_set<BasicBlock*> &visited) {
    for(auto *b: bbs) {
        if(!visited.contains(b)) {
            visit_block(out, b, visited);
        }
    }
}


void CFGViz::run_pass(Function* function) {
    static int count = 0;
    std::string filename = "temp_files/"+function->id+std::to_string(count++)+".dot";
    std::fstream out;
    out = std::fstream(filename, std::ios::out | std::ios::trunc);
    
    std::unordered_set<BasicBlock*> visited;

    out << "digraph {\n";
    visit_block(out, function->blocks[0], visited);
    visit_dead_blocks(out, function->blocks, visited);
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
