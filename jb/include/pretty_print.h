#pragma once

#include "jb.h"

#include <iostream>
#include <string>
#include <format>

namespace jb {

inline std::string str(CallConv callconv) {
    switch (callconv) {
    case CallConv::none:
        return "none";
    case CallConv::win64:
        return "win64";
    case CallConv::sysv64:
        return "sysv64";
    default:
        assert(false);
        return "";
    }
}

inline std::string str(Type type) {
    switch (type) {
    case Type::none:
        return "none";
    case Type::i8:
        return "i8";
    case Type::i16:
        return "i16";
    case Type::i32:
        return "i32";
    case Type::i64:
        return "i64";
    case Type::f32:
        return "f32";
    case Type::f64:
        return "f64";
    default:
        assert(false);
        return "";
    }
}

inline std::string str(IRValue irval) {
    switch (irval.kind) {
    case IRValueKind::none:
        return "";
    case IRValueKind::vreg:
        return std::string("%") + std::to_string(irval.vreg);
    case IRValueKind::imm:
        return std::to_string(irval.imm);
    default:
        assert(false);
        return "";
    }
}

inline std::string str(IROp op) {
    switch (op) {

#define X(x)                                                                                                           \
    case IROp::x:                                                                                                      \
        return #x;
#include "jbir_ops.inc"
#undef X
    default:
        assert(false);
        return "";
    }
}

inline std::string str(IRInst irinst) {
    auto op = str(irinst.op);
    switch (irinst.op) {
    case IROp::none:
        return std::format("{}", op);
    case IROp::iconst8:
        return std::format("{} = {} {}", str(irinst.dest), op, str(irinst.src1));
    case IROp::iconst16:
        return std::format("{} = {} {}", str(irinst.dest), op, str(irinst.src1));
    case IROp::iconst32:
        return std::format("{} = {} {}", str(irinst.dest), op, str(irinst.src1));
    case IROp::iconst64:
        return std::format("{} = {} {}", str(irinst.dest), op, str(irinst.src1));
    case IROp::fconst32:
        return std::format("{} = {} {}", str(irinst.dest), op, str(irinst.src1));
    case IROp::fconst64:
        return std::format("{} = {} {}", str(irinst.dest), op, str(irinst.src1));
    case IROp::mov:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::addi:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::subi:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::muli:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::divi:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::modi:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::addf:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::subf:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::mulf:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::divf:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::modf:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::lt:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::lte:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::gt:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::gte:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::eq:
        return std::format("{} = {} {}, {}", str(irinst.dest), op, str(irinst.src1), str(irinst.src2));
    case IROp::br:
        return std::format("{}", op);
    case IROp::brz:
        return std::format("{}", op);
    case IROp::brnz:
        return std::format("{}", op);
    case IROp::call: {
        std::string args = "";
        for (auto &param : irinst.params)
            args += str(param) + " ";
        return std::format("{} = {} {} {}", str(irinst.fn->ret), op, irinst.fn->id, args);
    }
    case IROp::ret:
        return std::format("{} {}", op, str(irinst.src1));

    case IROp::salloc:
        return std::format("{} = {} {}", str(irinst.dest), op, str(irinst.src1));
    case IROp::store:
        return std::format("{} {}", str(irinst.src1), op, str(irinst.dest));
    case IROp::load:
        return std::format("{} = {}", str(irinst.dest), op, str(irinst.src1));

        // case IROp::phi:
        //     std::string args = "";
        //     for (auto [bb, val] : irinst.values)
        //         args += "(" bb->id + str(val) + ")";
        //     return std::format("{} = [{}]", str(irinst.dest), op, args);
        // }
    default:
        assert(false);
        return "";
    }
}

inline void pretty_print(Module *module) {
    auto tab_count = 0;
    std::cout << module->name << ":\n";
    ++tab_count;
    for (auto *fn : module->functions) {
        std::string btab_string(tab_count, '\t');
        std::string ntab_string(tab_count + 1, '\t');

        std::string fn_string = btab_string + "[" + str(fn->callconv) + "]\n" + btab_string + fn->id + '(';

        for (auto param : fn->params) {
            fn_string += str(param) + ":" + str(param.type) + ", ";
        }
        if (!fn->params.empty()) {
            fn_string[fn_string.size() - 2] = ')';
            fn_string[fn_string.size() - 1] = ' ';
        } else
            fn_string += ") ";

        fn_string += str(fn->ret) + ":" + str(fn->ret.type);
        std::cout << fn_string << "\n";

        for (auto *block : fn->blocks) {
            std::string pred_string = "[[";
            for (auto *pred : block->preds)
                pred_string += pred->id + ", ";
            if (!block->preds.empty()) {
                pred_string[pred_string.size() - 2] = ']';
                pred_string[pred_string.size() - 1] = ']';
                std::cout << ntab_string << pred_string << '\n';
            } else {
                std::cout << ntab_string << "[[]]\n";
            }

            std::string bb_string = block->id;
            for (auto param : block->params) {
                bb_string += str(param) + ":" + str(param.type) + ", ";
            }
            if (!block->params.empty()) {
                bb_string[bb_string.size() - 2] = ')';
                bb_string[bb_string.size() - 1] = ':';
                std::cout << ntab_string << bb_string << "\n";
            } else {
                std::cout << ntab_string << bb_string << ":\n";
            }

            for (auto inst : block->insts) {
                std::cout << ntab_string << str(inst) << "\n";
            }
        }
    }
    std::cout << '\n';
}

} // namespace jb
