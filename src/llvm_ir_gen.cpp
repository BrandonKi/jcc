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
llvm::Type *LLVMIRGen::to_llvm_type(CType *type) {
    switch (type->type) {
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
        return to_llvm_type(type->ptr)->getPointerTo();
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

    // FIXME clean up
    m_named_values.clear();
    int i = 0;
    for (auto &arg : function->args()) {
        // m_named_values[std::string(arg.getName())] = {fn->proto->args[i],
        // &arg};
        auto decl = fn->proto->args[i];
        auto arg_val = &arg;
        m_builder->CreateAlignedStore(arg_val, genDecl(decl),
                                      llvm::Align(decl->type->align), false);

        ++i;
    }

    genCompoundStmnt(fn->body);
    return function;
}

llvm::Function *LLVMIRGen::genPrototype(PrototypeNode *proto) {
    JCC_PROFILE()
    std::vector<llvm::Type *> arg_types;
    for (auto arg : proto->args) {
        arg_types.push_back(to_llvm_type(arg->type));
    }

    llvm::FunctionType *ft = llvm::FunctionType::get(
        to_llvm_type(proto->ret_type), arg_types, false);

    llvm::Function *f = llvm::Function::Create(
        ft, llvm::Function::ExternalLinkage, *proto->id, m_module.get());

    int index = 0;
    for (auto &arg : f->args()) {
        arg.setName(*proto->args[index]->id);
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

llvm::Value *LLVMIRGen::genDecl(DeclNode *decl) {
    auto alloca = m_builder->CreateAlloca(to_llvm_type(decl->type));
    if (decl->init)
        m_builder->CreateAlignedStore(genExpr(decl->init), alloca,
                                      llvm::Align(decl->type->align), false);
    m_named_values[*decl->id] = {decl, alloca};
    return alloca;
}

void LLVMIRGen::genStmnt(StmntNode *stmnt) {
    switch (stmnt->kind) {
    case LabelStmnt:
    case CompoundStmnt:
    case ExprStmnt:
        // TODO result not used
        // make a genExpr that doesn't bother loading it?
        // can be reused for comma expr as well
        genExpr(static_cast<ExprStmntNode *>(stmnt)->expr);
        break;
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
    case ExprKind::CallExpr:
        return genCallExpr(static_cast<CallExprNode *>(expr));
    default:
        assert(false);
    }

    assert(false);
    return nullptr;
}

llvm::Value *LLVMIRGen::genCallExpr(CallExprNode *call_expr) {
    JCC_PROFILE()

    // FIXME hack
    if (call_expr->base->kind != ExprKind::IdExpr)
        assert(false);

    llvm::Function *callee =
        m_module->getFunction(*static_cast<IdExprNode *>(call_expr->base)->val);

    if (!callee)
        assert(false);

    if (callee->arg_size() != call_expr->args.size())
        assert(false);

    std::vector<llvm::Value *> args;

    for (auto *a : call_expr->args) {
        args.push_back(genExpr(a));
        if (!args.back())
            assert(false);
    }

    return m_builder->CreateCall(callee, args);
}

llvm::Value *LLVMIRGen::genBinExpr(BinExprNode *bin_expr) {
    JCC_PROFILE()

    llvm::Value *lhs = nullptr;
    llvm::Value *rhs = nullptr;

    switch (bin_expr->op) {
    case BinOp::_log_and:
        // TODO need to short circuit
        break;
    case BinOp::_log_or:
        // TODO need to short circuit
        break;
    case BinOp::_assign:
        return genAssign(bin_expr);
    default:
        break;
        // just continue on
    }

    lhs = genExpr(bin_expr->lhs);
    rhs = genExpr(bin_expr->rhs);
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

// TODO possibly replace this with genLValue(), or something similar
// it's a semi-common pattern, duplicated in genAddressOf
llvm::Value *LLVMIRGen::genAssign(BinExprNode *bin_expr) {
    JCC_PROFILE()

    llvm::Value *rhs = genExpr(bin_expr->rhs);

    switch (bin_expr->lhs->kind) {
    case IdExpr: {
        auto lhs = static_cast<IdExprNode *>(bin_expr->lhs);
        auto &[decl, addr] = m_named_values[*lhs->val];
        m_builder->CreateAlignedStore(
            rhs, addr,
            llvm::Align(CType::getBuiltinType(CTypeKind::LLong)->align));
        return addr;
    }
    case UnaryExpr: {
        auto lhs = static_cast<UnaryExprNode *>(bin_expr->lhs);
    }
    default:
        assert(false);
    }

    return nullptr;
}

// TODO finish
llvm::Value *LLVMIRGen::genUnaryExpr(UnaryExprNode *unary_expr) {
    JCC_PROFILE()

    llvm::Value *expr;

    switch (unary_expr->op) {
    case UnaryOp::_prefix_inc:
    case UnaryOp::_prefix_dec:
    case UnaryOp::_sizeof:
    case UnaryOp::__Alignof:
        assert(false);
    case UnaryOp::_address: {
        return genAddressOf(unary_expr->expr);
    }
    case UnaryOp::_deref: {
        expr = genExpr(unary_expr->expr);
        llvm::Value *val = m_builder->CreateAlignedLoad(
            expr->getType(), expr,
            llvm::Align(CType::getBuiltinType(CTypeKind::LLong)->align));
        return val;
    }
    case UnaryOp::_add:
        expr = genExpr(unary_expr->expr);
        return expr;
    case UnaryOp::_sub:
        expr = genExpr(unary_expr->expr);
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

// FIXME does not handle all valid cases
// TODO possibly replace this with genLValue(), or something similar
// it's a semi-common pattern, duplicated in genAssign
llvm::Value *LLVMIRGen::genAddressOf(ExprNode *base_expr) {
    switch (base_expr->kind) {
    case IdExpr: {
        auto expr = static_cast<IdExprNode *>(base_expr);
        auto &[decl, addr] = m_named_values[*expr->val];
        return addr;
    }
    case UnaryExpr: {
        auto expr = static_cast<UnaryExprNode *>(base_expr);
        return genExpr(expr->expr);
    }
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
                                       llvm::Align(decl->type->align), false);
    return val;
}

llvm::Value *LLVMIRGen::genNumLitExpr(NumLitExprNode *num_expr) {
    JCC_PROFILE()
    return llvm::ConstantInt::get(*m_context,
                                  llvm::APInt(32, num_expr->val, true));
}
