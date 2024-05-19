#ifndef JAB_PRETTY_PRINT_H
#define JAB_PRETTY_PRINT_H

#include "jab.h"

#include <iostream>
#include <string>
#include <format>

namespace jab {

inline std::string str(CallConv callconv) {
	switch(callconv) {
        case CallConv::none:
            return "none";
        case CallConv::win64:
            return "win64";
        case CallConv::sysv64:
            return "sysv64";
        default:
            unreachable
            return "";
	}
}

inline std::string str(Type type) {
	switch(type) {
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
            unreachable
            return "";
	}
}

inline std::string str(IRValue irval) {
	switch(irval.kind) {
		case IRValueKind::none:
			return "";
		case IRValueKind::vreg:
			return std::string("%") + std::to_string(irval.vreg.num);
		case IRValueKind::hreg:
			return std::string("$") + std::to_string(irval.hreg.num);
		case IRValueKind::imm:
			return std::to_string(irval.imm);
		default:
			unreachable
            return "";
	}
}

inline std::string str(IROp op) {
	switch(op) {
		case IROp::none:
            return "none";
        case IROp::iconst8:
            return "iconst8";
        case IROp::iconst16:
            return "iconst16";
        case IROp::iconst32:
            return "iconst32";
        case IROp::iconst64:
            return "iconst64";
        case IROp::fconst32:
            return "fconst32";
        case IROp::fconst64:
            return "fconst64";
        case IROp::mov:
            return "mov";
        case IROp::addi:
            return "addi";
        case IROp::subi:
            return "subi";
        case IROp::muli:
            return "muli";
        case IROp::divi:
            return "divi";
        case IROp::modi:
            return "modi";
        case IROp::addf:
            return "addf";
        case IROp::subf:
            return "mulf";
        case IROp::divf:
            return "divf";
        case IROp::modf:
            return "modf";
        case IROp::lt:
            return "lt";
        case IROp::lte:
            return "lte";
        case IROp::gt:
            return "gt";
        case IROp::gte:
            return "gte";
        case IROp::eq:
            return "eq";
        case IROp::br:
            return "br";
        case IROp::brz:
            return "brz";
        case IROp::brnz:
            return "brnz";
        case IROp::call:
            return "call";
        case IROp::ret:
            return "ret";
        default:
            unreachable
            return "";
	}
}

inline std::string str(IRInst irinst) {
	auto op = str(irinst.op);
	switch(irinst.op) {
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
			for(auto& param: irinst.params)
				args += str(param) + " ";
			return std::format("{} {} {}", op, irinst.fn->id, args);
		}
        case IROp::ret:
			return std::format("{} {}", op, str(irinst.src1));
        default:
            unreachable
            return "";
	}
}

inline void pretty_print(Module* module) {
	auto tab_count = 0;
	std::cout << module->name << ":\n";
	++tab_count;
	for(auto* fn: module->functions) {
		std::string btab_string(tab_count, '\t');
		std::string ntab_string(tab_count + 1, '\t');

		std::string fn_string =
			btab_string + "[" + str(fn->callconv) + "]\n" +
			btab_string + fn->id + '(';

		for(auto param: fn->params) {
			fn_string += str(param) + ":" + str(param.type) + ", ";
		}
		if(!fn->params.empty()) {
			fn_string[fn_string.size()-2] = ')';
			fn_string[fn_string.size()-1] = ' ';
		}
		else
			fn_string += ") ";

		fn_string += str(fn->ret) + ":" + str(fn->ret.type);
		std::cout << fn_string << "\n";

		for(auto* block: fn->blocks) {
			std::string pred_string = "[[";
			for(auto* pred: block->preds)
				pred_string += pred->id + ", ";
			if(!block->preds.empty()) {
				pred_string[pred_string.size()-2] = ']';
				pred_string[pred_string.size()-1] = ']';
				std::cout << ntab_string << pred_string << '\n';
			}
			else {
				std::cout << ntab_string << "[[]]\n";
			}
			
			std::string bb_string = block->id;
			for(auto param: block->params) {
				bb_string += str(param) + ":" + str(param.type) + ", ";
			}
			if(!block->params.empty()) {
				bb_string[bb_string.size()-2] = ')';
				bb_string[bb_string.size()-1] = ':';
				std::cout << ntab_string << bb_string << "\n";
			}
			else {
				std::cout << ntab_string << bb_string << ":\n";
			}
			
			for(auto inst: block->insts) {
				std::cout << ntab_string << str(inst) << "\n";
			
			}
		}
	}
    std::cout << '\n';
}

}  // namespace jab

#endif // JAB_PRETTY_PRINT_H
