#include "llvm_ir_gen.h"
#include "ast.h"
// #include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Alignment.h"
#include <memory>
#include <vector>

using namespace jcc;

LLVMIRGen::LLVMIRGen() {
    JCC_PROFILE()
    m_context = std::make_unique<llvm::LLVMContext>();
    m_module = std::make_unique<llvm::Module>("jcc", *m_context);

    m_builder = std::make_unique<llvm::IRBuilder<>>(*m_context);
}

// TODO
llvm::Type *LLVMIRGen::to_llvm_type(CType type) {
    switch (type.type) {
    case Char:
        return llvm::Type::getInt8Ty(*m_context);
    case Short:
        return llvm::Type::getInt16Ty(*m_context);
    case Int:
        return llvm::Type::getInt32Ty(*m_context);
    case Long:
        return llvm::Type::getInt32Ty(*m_context);
    case LLong:
        return llvm::Type::getInt64Ty(*m_context);
    case Float:
        return llvm::Type::getFloatTy(*m_context);
    case Double:
        return llvm::Type::getDoubleTy(*m_context);
    case Void:
        return llvm::Type::getVoidTy(*m_context);
    case Pointer:
    case Struct:
    case Union:
    case Array:
    case Function:
        assert(false);
    case Bool:
        return llvm::Type::getInt8Ty(*m_context);
    case Enum:
    default:
        assert(false);
    }
    assert(false);
    return llvm::Type::getVoidTy(*m_context);
}

void LLVMIRGen::genFile(FileNode *file) {
    JCC_PROFILE()
    for (auto *p : file->prototypes) {
        genPrototype(p);
    }

    for (auto *f : file->functions) {
        genFunction(f);
    }
}

llvm::Function *LLVMIRGen::genFunction(FunctionNode *fn) {
    JCC_PROFILE()
    llvm::Function *function = m_module->getFunction(*fn->proto->id);

    if (function)
        assert(false);

    function = genPrototype(fn->proto);

    if (!function)
        assert(false);

    llvm::BasicBlock *bb =
        llvm::BasicBlock::Create(*m_context, "entry", function);
    m_builder->SetInsertPoint(bb);

    m_named_values.clear();
    // for (auto &arg : function->args()) {
    //     m_named_values[std::string(arg.getName())] = &arg;
    // }
    genCompoundStmnt(fn->body);
    // llvm::Value *ret_val = genCompoundStmnt(fn->body);
    // if (!ret_val) {
    //     assert(false);
    // }

    // m_builder->CreateRet(ret_val);
    return function;
}

llvm::Function *LLVMIRGen::genPrototype(PrototypeNode *proto) {
    JCC_PROFILE()
    std::vector<llvm::Type *> arg_types(proto->args.size(),
                                        llvm::Type::getInt32Ty(*m_context));
    llvm::FunctionType *ft = llvm::FunctionType::get(
        to_llvm_type(proto->ret_type), arg_types, false);

    llvm::Function *f = llvm::Function::Create(
        ft, llvm::Function::ExternalLinkage, *proto->id, m_module.get());

    int index = 0;
    for (auto &arg : f->args()) {
        arg.setName(*proto->args[index]);
        ++index;
    }

    return f;
}

void LLVMIRGen::genCompoundStmnt(CompoundStmntNode *stmnts) {
    for (auto *decl : stmnts->decl_list) {
        genDecl(decl);
    }

    for (auto *stmnt : stmnts->stmnt_list) {
        genStmnt(stmnt);
    }
}

void LLVMIRGen::genDecl(DeclNode *decl) {
    auto alloca = m_builder->CreateAlloca(to_llvm_type(decl->type));
    m_builder->CreateAlignedStore(genExpr(decl->init), alloca,
                                  llvm::Align(decl->type.align), false);
    m_named_values[*decl->id] = {decl, alloca};
}

void LLVMIRGen::genStmnt(StmntNode *stmnt) {
    switch (stmnt->kind) {
    case LabelStmnt:
    case CompoundStmnt:
    case ExprStmnt:
        /* case SelectionStmnt: */
        /* case IterStmnt: */
        assert(false);
    case ReturnStmnt:
        m_builder->CreateRet(
            genExpr(static_cast<ReturnStmntNode *>(stmnt)->expr));
        break;
    default:
        assert(false);
    }
}

llvm::Value *LLVMIRGen::genExpr(ExprNode *expr) {
    JCC_PROFILE()
    switch (expr->kind) {
    case ExprKind::NumLitExpr:
        return genNumLitExpr(static_cast<NumLitExprNode *>(expr));
    case ExprKind::IdExpr:
        return genIdExpr(static_cast<IdExprNode *>(expr));
    case ExprKind::UnaryExpr:
        return genUnaryExpr(static_cast<UnaryExprNode *>(expr));
    case ExprKind::BinExpr:
        return genBinExpr(static_cast<BinExprNode *>(expr));
    /* case ExprKind::CallExpr: */
    /*     return genCallExpr(static_cast<CallExprNode *>(expr)); */
    default:
        assert(false);
    }

    assert(false);
    return nullptr;
}

/* llvm::Value *LLVMIRGen::genCallExpr(CallExprNode *call_expr) { */
/*     JCC_PROFILE() */
/*     llvm::Function *callee = m_module->getFunction(*call_expr->id); */
/**/
/*     if (!callee) */
/*         assert(false); */
/**/
/*     if (callee->arg_size() != call_expr->args.size()) */
/*         assert(false); */
/**/
/*     std::vector<llvm::Value *> args; */
/**/
/*     for (auto *a : call_expr->args) { */
/*         args.push_back(genExpr(a)); */
/*         if (!args.back()) */
/*             assert(false); */
/*     } */
/**/
/*     return m_builder->CreateCall(callee, args, "calltmp"); */
/* } */

llvm::Value *LLVMIRGen::genBinExpr(BinExprNode *bin_expr) {
    JCC_PROFILE()
    llvm::Value *lhs = genExpr(bin_expr->lhs);
    llvm::Value *rhs = genExpr(bin_expr->rhs);
    if (!lhs || !rhs)
        assert(false);

    switch (bin_expr->op) {
    case BinOp::_mul:
        return m_builder->CreateMul(lhs, rhs);
    case BinOp::_div:
        return m_builder->CreateSDiv(lhs, rhs);
    case BinOp::_mod:
        return m_builder->CreateSRem(lhs, rhs);
    case BinOp::_add:
        return m_builder->CreateAdd(lhs, rhs);
    case BinOp::_sub:
        return m_builder->CreateSub(lhs, rhs);
    case BinOp::_bitshift_left:
        return m_builder->CreateShl(lhs, rhs);
    case BinOp::_bitshift_right:
        return m_builder->CreateAShr(lhs, rhs);
    case BinOp::_less_than:
        lhs = m_builder->CreateICmpSLT(lhs, rhs);
        lhs = m_builder->CreateZExt(lhs, llvm::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_greater_than:
        lhs = m_builder->CreateICmpSGT(lhs, rhs);
        lhs = m_builder->CreateZExt(lhs, llvm::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_less_than_equal:
        lhs = m_builder->CreateICmpSLE(lhs, rhs);
        lhs = m_builder->CreateZExt(lhs, llvm::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_greater_than_equal:
        lhs = m_builder->CreateICmpSGE(lhs, rhs);
        lhs = m_builder->CreateZExt(lhs, llvm::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_equal:
        lhs = m_builder->CreateICmpEQ(lhs, rhs);
        lhs = m_builder->CreateZExt(lhs, llvm::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_not_equal:
        lhs = m_builder->CreateICmpNE(lhs, rhs);
        lhs = m_builder->CreateZExt(lhs, llvm::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_bit_and:
        return m_builder->CreateAnd(lhs, rhs);
    case BinOp::_xor:
        return m_builder->CreateXor(lhs, rhs);
    case BinOp::_bit_or:
        return m_builder->CreateOr(lhs, rhs);
    case BinOp::_log_and:

    case BinOp::_log_or:

    case BinOp::_assign:
    default:
        assert(false);
    }

    assert(false);
    return nullptr;
}

// TODO
llvm::Value *LLVMIRGen::genUnaryExpr(UnaryExprNode *unary_expr) {
    JCC_PROFILE()

    llvm::Value *expr = genExpr(unary_expr->expr);
    if (!expr)
        assert(false);

    switch (unary_expr->op) {
    case UnaryOp::_prefix_inc:
    case UnaryOp::_prefix_dec:
    case UnaryOp::_sizeof:
    case UnaryOp::__Alignof:
    case UnaryOp::_address:
    case UnaryOp::_deref:
        assert(false);
    case UnaryOp::_add:
        return expr;
    case UnaryOp::_sub:
        expr = m_builder->CreateNSWSub(
            llvm::ConstantInt::get(*m_context, llvm::APInt(32, 0, true)), expr);
        return expr;
    case UnaryOp::_bit_not:
    case UnaryOp::_log_not:
    case UnaryOp::_cast:
    default:
        assert(false);
    }
    return nullptr;
}

llvm::Value *LLVMIRGen::genIdExpr(IdExprNode *id_expr) {
    JCC_PROFILE()
    auto [decl, val] = m_named_values[*id_expr->val];
    if (!val)
        assert(false);
    val = m_builder->CreateAlignedLoad(to_llvm_type(decl->type), val,
                                       llvm::Align(decl->type.align), false);
    return val;
}

llvm::Value *LLVMIRGen::genNumLitExpr(NumLitExprNode *num_expr) {
    JCC_PROFILE()
    return llvm::ConstantInt::get(*m_context,
                                  llvm::APInt(32, num_expr->val, true));
}
