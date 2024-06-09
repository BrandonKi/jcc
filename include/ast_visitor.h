#pragma once

/// This is just an experiment. None of this runs by default.
/// The intention was to give easy ways to
///     adapt the preexisting infrastructure for other usecases.
/// Pretty much just playing around with compiler as a library stuff.

// TODO use accept to continue traversal
// instead of this visit/leave thing

#include "ast.h"

namespace jcc {

// TODO replace a bunch of this manual work with X macros
template <typename Derived>
struct ASTVisitor {
    void traverseFile(FileNode *file) {
        if (!file)
            return;
        derived().visitFile(file);

        for (auto *p : file->prototypes) {
            derived().traversePrototype(p);
        }
        for (auto *f : file->functions) {
            derived().traverseFunction(f);
        }

        derived().leaveFile(file);
    }

    void traversePrototype(PrototypeNode *proto) {
        if (!proto)
            return;
        derived().visitPrototype(proto);

        derived().leavePrototype(proto);
    }

    void traverseFunction(FunctionNode *fn) {
        if (!fn)
            return;
        derived().visitFunction(fn);

        derived().traversePrototype(fn->proto);
        derived().traverseExpr(fn->body);

        derived().leaveFunction(fn);
    }

    void traverseExpr(ExprNode *expr) {
        if (!expr)
            return;
        // TODO should the base expr be visited??
        derived().visitExpr(expr);

        switch (expr->kind) {
        case ExprKind::NumExpr:
            traverseNumExpr(static_cast<NumExprNode *>(expr));
            break;
        case ExprKind::IdExpr:
            traverseIdExpr(static_cast<IdExprNode *>(expr));
            break;
        case ExprKind::UnaryExpr:
            traverseUnaryExpr(static_cast<UnaryExprNode *>(expr));
            break;
        case ExprKind::BinExpr:
            traverseBinExpr(static_cast<BinExprNode *>(expr));
            break;
        case ExprKind::CallExpr:
            traverseCallExpr(static_cast<CallExprNode *>(expr));
            break;
        default:
            ice(false);
        }

        derived().leaveExpr(expr);
    }

    void traverseNumExpr(NumExprNode *num_expr) {
        if (!num_expr)
            return;
        derived().visitNumExpr(num_expr);

        derived().leaveNumExpr(num_expr);
    }

    void traverseIdExpr(IdExprNode *id_expr) {
        if (!id_expr)
            return;
        derived().visitIdExpr(id_expr);

        derived().leaveIdExpr(id_expr);
    }

    void traverseUnaryExpr(UnaryExprNode *unary_expr) {
        if (!unary_expr)
            return;
        derived().visitUnaryExpr(unary_expr);

        derived().traverseExpr(unary_expr->base);

        derived().leaveUnaryExpr(unary_expr);
    }

    void traverseBinExpr(BinExprNode *bin_expr) {
        if (!bin_expr)
            return;
        derived().visitBinExpr(bin_expr);

        derived().traverseExpr(bin_expr->lhs);
        derived().traverseExpr(bin_expr->rhs);

        derived().leaveBinExpr(bin_expr);
    }

    void traverseCallExpr(CallExprNode *call_expr) {
        if (!call_expr)
            return;
        derived().visitCallExpr(call_expr);

        for (auto *e : call_expr->args) {
            derived().traverseExpr(e);
        }

        derived().leaveCallExpr(call_expr);
    }

    void visitFile(FileNode *) {}
    void visitPrototype(PrototypeNode *) {}
    void visitFunction(FunctionNode *) {}
    void visitExpr(ExprNode *) {}
    void visitNumExpr(NumExprNode *) {}
    void visitIdExpr(IdExprNode *) {}
    void visitUnaryExpr(UnaryExprNode *) {}
    void visitBinExpr(BinExprNode *) {}
    void visitCallExpr(CallExprNode *) {}

    void leaveFile(FileNode *) {}
    void leavePrototype(PrototypeNode *) {}
    void leaveFunction(FunctionNode *) {}
    void leaveExpr(ExprNode *) {}
    void leaveNumExpr(NumExprNode *) {}
    void leaveIdExpr(IdExprNode *) {}
    void leaveUnaryExpr(UnaryExprNode *) {}
    void leaveBinExpr(BinExprNode *) {}
    void leaveCallExpr(CallExprNode *) {}

private:
    Derived &derived() {
        return *static_cast<Derived *>(this);
    }
};

struct PrintVisitor : ASTVisitor<PrintVisitor> {
    std::string indent_string = "";
    static constexpr const char *level = "  ";

    void indent() {
        indent_string += level;
    }

    void unindent() {
        indent_string =
            indent_string.substr(0, indent_string.size() - strlen(level));
    }

    void visitFile(FileNode *file) {
        println(indent_string + "File:");
        indent();
    }

    void leaveFile(FileNode *) {
        unindent();
    }

    void visitPrototype(PrototypeNode *proto) {
        println(indent_string + "Prototype:");
    }

    void leavePrototype(PrototypeNode *) {}

    void visitFunction(FunctionNode *fn) {
        println(indent_string + "Function:");
        indent();
    }

    void leaveFunction(FunctionNode *) {
        unindent();
    }

    void visitExpr(ExprNode *expr) {
        println(indent_string + "Expr:");
        indent();
    }

    void leaveExpr(ExprNode *) {
        unindent();
    }

    void visitNumExpr(NumExprNode *num) {
        println(indent_string + "NumExpr:");
    }

    void leaveNumExpr(NumExprNode *) {}

    void visitIdExpr(IdExprNode *) {
        println(indent_string + "IdExpr:");
    }

    void leaveIdExpr(IdExprNode *) {}

    void visitUnaryExpr(UnaryExprNode *) {
        println(indent_string + "UnaryExpr:");
        indent();
    }

    void leaveUnaryExpr(UnaryExprNode *) {
        unindent();
    }

    void visitBinExpr(BinExprNode *) {
        println(indent_string + "BinExpr:");
        indent();
    }

    void leaveBinExpr(BinExprNode *) {
        unindent();
    }

    void visitCallExpr(CallExprNode *) {
        println(indent_string + "CallExpr:");
        indent();
    }

    void leaveCallExpr(CallExprNode *) {
        unindent();
    }
};

} // namespace jcc
