#include "llvm_ir_gen.h"
#include "ast.h"
// #include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Alignment.h"
#include <memory>
#include <vector>

using namespace jcc;

LLVMIRGen::LLVMIRGen() {
    JCC_PROFILE();

    m_context = std::make_unique<llvm::LLVMContext>();
    m_module = std::make_unique<llvm::Module>("jcc", *m_context);

    m_builder = std::make_unique<llvm::IRBuilder<>>(*m_context);
    m_named_values = {};
}

// TODO
llvm::Type *LLVMIRGen::to_llvm_type(CType *type) {
    JCC_PROFILE();

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
        return to_llvm_type(type->base)->getPointerTo();
    case Struct:
    case Union:
    case Array:
    case Function:
        ice(false);
    case Bool:
        return llvm::Type::getInt1Ty(*m_context);
    case Enum:
    default:
        ice(false);
    }
    ice(false);
    return llvm::Type::getVoidTy(*m_context);
}

llvm::Value *LLVMIRGen::gen_str_lit_expr(StrLitExprNode *str_expr) {
    JCC_PROFILE();

    llvm::Value *v = m_builder->CreateGlobalString(str_expr->val.value());
    return v;
}

llvm::Value *LLVMIRGen::gen_num_lit_expr(NumLitExprNode *num_expr) {
    JCC_PROFILE();

    return llvm::ConstantInt::get(
        *m_context,
        llvm::APInt(to_llvm_type(num_expr->type)->getPrimitiveSizeInBits(),
                    num_expr->val, true));
}

llvm::Value *LLVMIRGen::gen_id_expr(IdExprNode *id_expr) {
    JCC_PROFILE();

    auto [decl, val] = m_named_values[id_expr->val.value()];
    if (!val)
        ice(false);
    val = m_builder->CreateAlignedLoad(to_llvm_type(decl->type), val,
                                       llvm::Align(decl->type->align), false);
    return val;
}

// FIXME does not handle all valid cases
// TODO possibly replace this with genLValue(), or something similar
// it's a semi-common pattern, duplicated in genAssign
llvm::Value *LLVMIRGen::gen_address_of(ExprNode *base_expr) {
    JCC_PROFILE();

    switch (base_expr->kind) {
    case IdExpr: {
        auto expr = static_cast<IdExprNode *>(base_expr);
        auto &[decl, addr] = m_named_values[expr->val];
        return addr;
    }
    case UnaryExpr: {
        auto expr = static_cast<UnaryExprNode *>(base_expr);
        return gen_expr(expr->base);
    }
    default:
        ice(false);
    }

    return nullptr;
}

// TODO finish
llvm::Value *LLVMIRGen::gen_unary_expr(UnaryExprNode *unary_expr) {
    JCC_PROFILE();

    llvm::Value *expr;

    switch (unary_expr->op) {
    case UnaryOp::_postfix_inc: {
        return m_builder->CreateAdd(
            gen_expr(unary_expr->base),
            llvm::ConstantInt::get(
                *m_context,
                llvm::APInt(unary_expr->base->type->size, 1, true)));
    }
    case UnaryOp::_postfix_dec: {
        return m_builder->CreateSub(
            gen_expr(unary_expr->base),
            llvm::ConstantInt::get(
                *m_context,
                llvm::APInt(unary_expr->base->type->size, 1, true)));
    }
    case UnaryOp::_prefix_inc: {
        return m_builder->CreateAdd(
            gen_expr(unary_expr->base),
            llvm::ConstantInt::get(
                *m_context,
                llvm::APInt(unary_expr->base->type->size, 1, true)));
    }
    case UnaryOp::_prefix_dec: {
        return m_builder->CreateSub(
            gen_expr(unary_expr->base),
            llvm::ConstantInt::get(
                *m_context,
                llvm::APInt(unary_expr->base->type->size, 1, true)));
    }
    case UnaryOp::_sizeof: {
        if (unary_expr->base) {
            return llvm::ConstantInt::get(
                *m_context,
                llvm::APInt(32, unary_expr->base->type->size, true));
        }
        return llvm::ConstantInt::get(
            *m_context, llvm::APInt(32, unary_expr->base_type->size, true));
    }
    case UnaryOp::__Alignof: {
        return llvm::ConstantInt::get(
            *m_context, llvm::APInt(32, unary_expr->base_type->align, true));
    }
    case UnaryOp::_address: {
        return gen_address_of(unary_expr->base);
    }
    case UnaryOp::_deref: {
        expr = gen_expr(unary_expr->base);
        llvm::Value *val =
            m_builder->CreateAlignedLoad(to_llvm_type(unary_expr->type), expr,
                                         llvm::Align(unary_expr->type->align));
        return val;
    }
    case UnaryOp::_add:
        expr = gen_expr(unary_expr->base);
        return expr;
    case UnaryOp::_sub:
        expr = gen_expr(unary_expr->base);
        expr = m_builder->CreateNSWSub(
            llvm::ConstantInt::get(
                *m_context, llvm::APInt(to_llvm_type(unary_expr->base->type)
                                            ->getPrimitiveSizeInBits(),
                                        0, true)),
            expr);
        return expr;
    case UnaryOp::_bit_not:
    case UnaryOp::_log_not:
        ice(false);
    case UnaryOp::_cast:
        return gen_cast_expr(unary_expr);
    default:
        ice(false);
    }
    return nullptr;
}

// TODO possibly replace this with genLValue(), or something similar
// it's a semi-common pattern, duplicated in genAddressOf
llvm::Value *LLVMIRGen::gen_assign(BinExprNode *bin_expr) {
    JCC_PROFILE();

    llvm::Value *rhs = gen_expr(bin_expr->rhs);

    switch (bin_expr->lhs->kind) {
    case IdExpr: {
        auto lhs = static_cast<IdExprNode *>(bin_expr->lhs);
        auto &[decl, addr] = m_named_values[lhs->val];
        m_builder->CreateAlignedStore(rhs, addr,
                                      llvm::Align(bin_expr->rhs->type->align));
        return rhs;
    }
    case UnaryExpr: {
        auto lhs = static_cast<UnaryExprNode *>(bin_expr->lhs);
    }
    default:
        ice(false);
    }

    return nullptr;
}

llvm::Value *LLVMIRGen::gen_bin_expr(BinExprNode *bin_expr) {
    JCC_PROFILE();

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
        return gen_assign(bin_expr);
    default:
        break;
        // just continue on
    }

    lhs = gen_expr(bin_expr->lhs);
    rhs = gen_expr(bin_expr->rhs);
    if (!lhs || !rhs)
        ice(false);

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
        // TODO sema, so that this as well as many other things can work
        // correctly, tldr casts need to be inserted
    case BinOp::_less_than:
        lhs = m_builder->CreateICmpSLT(lhs, rhs);
        // lhs = m_builder->CreateZExt(lhs, llvm::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_greater_than:
        lhs = m_builder->CreateICmpSGT(lhs, rhs);
        // lhs = m_builder->CreateZExt(lhs, llvm::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_less_than_equal:
        lhs = m_builder->CreateICmpSLE(lhs, rhs);
        // lhs = m_builder->CreateZExt(lhs, llvm::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_greater_than_equal:
        lhs = m_builder->CreateICmpSGE(lhs, rhs);
        // lhs = m_builder->CreateZExt(lhs, llvm::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_equal:
        lhs = m_builder->CreateICmpEQ(lhs, rhs);
        // lhs = m_builder->CreateZExt(lhs, llvm::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_not_equal:
        lhs = m_builder->CreateICmpNE(lhs, rhs);
        // lhs = m_builder->CreateZExt(lhs, llvm::Type::getInt32Ty(*m_context));
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
        ice(false);
    }

    ice(false);
    return nullptr;
}

llvm::Value *LLVMIRGen::gen_cast_expr(UnaryExprNode *cast_expr) {
    JCC_PROFILE();

    llvm::Value *val = gen_expr(cast_expr->base);

    CType *src = cast_expr->base->type;
    CType *dest = cast_expr->type;

    // FIXME temporary, need to add size info to each type to make this easier
    if (src->is_int_type() && dest->is_int_type()) {
        val = m_builder->CreateSExtOrTrunc(val, to_llvm_type(dest));
    } else if (src->type == CTypeKind::Bool && dest->is_int_type()) {
        val = m_builder->CreateZExtOrTrunc(val, to_llvm_type(dest));
    }

    return val;
}

llvm::Value *LLVMIRGen::gen_call_expr(CallExprNode *call_expr) {
    JCC_PROFILE();

    // FIXME hack
    if (call_expr->base->kind != ExprKind::IdExpr)
        ice(false);

    llvm::Function *callee =
        m_module->getFunction(static_cast<IdExprNode *>(call_expr->base)->val.value());

    if (!callee)
        ice(false);

    if (callee->arg_size() != call_expr->args.size())
        ice(false);

    std::vector<llvm::Value *> args;

    for (auto *a : call_expr->args) {
        args.push_back(gen_expr(a));
        if (!args.back())
            ice(false);
    }

    return m_builder->CreateCall(callee, args);
}

llvm::Value *LLVMIRGen::gen_expr(ExprNode *expr) {
    JCC_PROFILE();

    switch (expr->kind) {
    case ExprKind::NumLitExpr:
        return gen_num_lit_expr(static_cast<NumLitExprNode *>(expr));
    case ExprKind::StrLitExpr:
        return gen_str_lit_expr(static_cast<StrLitExprNode *>(expr));
    case ExprKind::IdExpr:
        return gen_id_expr(static_cast<IdExprNode *>(expr));
    case ExprKind::UnaryExpr:
        return gen_unary_expr(static_cast<UnaryExprNode *>(expr));
    case ExprKind::BinExpr:
        return gen_bin_expr(static_cast<BinExprNode *>(expr));
    case ExprKind::CallExpr:
        return gen_call_expr(static_cast<CallExprNode *>(expr));
    default:
        ice(false);
    }

    ice(false);
    return nullptr;
}

llvm::Value *LLVMIRGen::gen_decl(DeclNode *decl) {
    JCC_PROFILE();

    auto saved_ip = m_builder->saveIP();
    m_builder->SetInsertPointPastAllocas(m_builder->GetInsertBlock()->getParent());
    
    auto alloca = m_builder->CreateAlloca(to_llvm_type(decl->type));

    m_builder->restoreIP(saved_ip);
    if (decl->init)
        m_builder->CreateAlignedStore(gen_expr(decl->init), alloca,
                                      llvm::Align(decl->type->align), false);
    m_named_values[decl->id] = {decl, alloca};
    return alloca;
}

// FIXME rewrite, possibly use similar logic to switch
void LLVMIRGen::gen_if_stmnt(IfStmntNode *if_stmnt) {
    JCC_PROFILE();

    llvm::Function *function = m_builder->GetInsertBlock()->getParent();

    llvm::Value *cond_val = gen_expr(if_stmnt->cond);

    llvm::BasicBlock *true_block =
        llvm::BasicBlock::Create(*m_context, "true", function);

    llvm::BasicBlock *cont_block = llvm::BasicBlock::Create(*m_context, "cont");

    llvm::BasicBlock *false_block = cont_block;
    if (if_stmnt->false_branch)
        false_block = llvm::BasicBlock::Create(*m_context, "false");

    m_builder->CreateCondBr(cond_val, true_block, false_block);

    m_builder->SetInsertPoint(true_block);
    gen_stmnt(if_stmnt->true_branch);
    if (!true_block->getTerminator()) {
        m_builder->CreateBr(cont_block);
    }

    if (if_stmnt->false_branch) {
        function->insert(function->end(), false_block);

        if (!m_builder->GetInsertBlock()->getTerminator())
            m_builder->CreateBr(cont_block);

        m_builder->SetInsertPoint(false_block);
        gen_stmnt(if_stmnt->false_branch);
        if (!false_block->getTerminator()) {
            m_builder->CreateBr(cont_block);
        }
    }

    function->insert(function->end(), cont_block);

    if (!m_builder->GetInsertBlock()->getTerminator())
        m_builder->CreateBr(cont_block);

    m_builder->SetInsertPoint(cont_block);
}

void LLVMIRGen::gen_while_stmnt(WhileStmntNode *while_stmnt) {
    JCC_PROFILE();

    llvm::Function *function = m_builder->GetInsertBlock()->getParent();

    llvm::BasicBlock *cond_block =
        llvm::BasicBlock::Create(*m_context, "", function);

    llvm::BasicBlock *body_block =
        llvm::BasicBlock::Create(*m_context, "", function);

    llvm::BasicBlock *cont_block = llvm::BasicBlock::Create(*m_context, "");

    m_builder->CreateBr(cond_block);

    m_builder->SetInsertPoint(cond_block);
    llvm::Value *cond = gen_expr(while_stmnt->cond);
    m_builder->CreateCondBr(cond, body_block, cont_block);

    m_builder->SetInsertPoint(body_block);
    gen_stmnt(while_stmnt->body);
    m_builder->CreateBr(cond_block);

    function->insert(function->end(), cont_block);

    if (!m_builder->GetInsertBlock()->getTerminator())
        m_builder->CreateBr(cont_block);

    m_builder->SetInsertPoint(cont_block);
}

void LLVMIRGen::gen_do_stmnt(DoStmntNode *do_stmnt) {
    JCC_PROFILE();

    llvm::Function *function = m_builder->GetInsertBlock()->getParent();

    llvm::BasicBlock *body_block =
        llvm::BasicBlock::Create(*m_context, "", function);

    llvm::BasicBlock *cond_block =
        llvm::BasicBlock::Create(*m_context, "", function);

    llvm::BasicBlock *cont_block = llvm::BasicBlock::Create(*m_context, "");

    m_builder->CreateBr(body_block);

    m_builder->SetInsertPoint(body_block);
    gen_stmnt(do_stmnt->body);
    m_builder->CreateBr(cond_block);

    m_builder->SetInsertPoint(cond_block);
    llvm::Value *cond = gen_expr(do_stmnt->cond);
    m_builder->CreateCondBr(cond, body_block, cont_block);

    function->insert(function->end(), cont_block);

    if (!m_builder->GetInsertBlock()->getTerminator())
        m_builder->CreateBr(cont_block);

    m_builder->SetInsertPoint(cont_block);
}

void LLVMIRGen::gen_for_stmnt(ForStmntNode *for_stmnt) {
    JCC_PROFILE();

    llvm::Function *function = m_builder->GetInsertBlock()->getParent();

    llvm::BasicBlock *cond_block =
        llvm::BasicBlock::Create(*m_context, "cond", function);
    llvm::BasicBlock *body_block =
        llvm::BasicBlock::Create(*m_context, "body", function);
    llvm::BasicBlock *inc_block = llvm::BasicBlock::Create(*m_context, "inc");
    llvm::BasicBlock *cont_block = llvm::BasicBlock::Create(*m_context, "cont");

    gen_decl(for_stmnt->init);
    m_builder->CreateBr(cond_block);

    m_builder->SetInsertPoint(cond_block);
    llvm::Value *cond = gen_expr(for_stmnt->cond);
    m_builder->CreateCondBr(cond, body_block, cont_block);

    m_builder->SetInsertPoint(body_block);
    gen_stmnt(for_stmnt->body);
    if (!m_builder->GetInsertBlock()->getTerminator())
        m_builder->CreateBr(inc_block);

    function->insert(function->end(), inc_block);

    m_builder->SetInsertPoint(inc_block);
    gen_expr(for_stmnt->inc);
    m_builder->CreateBr(cond_block);

    function->insert(function->end(), cont_block);

    if (!m_builder->GetInsertBlock()->getTerminator())
        m_builder->CreateBr(cont_block);

    m_builder->SetInsertPoint(cont_block);
}

void LLVMIRGen::gen_compound_stmnt(CompoundStmntNode *stmnts) {
    JCC_PROFILE();

    for (auto *decl : stmnts->decl_list) {
        gen_decl(decl);
    }

    for (auto *stmnt : stmnts->stmnt_list) {
        gen_stmnt(stmnt);
    }
}

void LLVMIRGen::gen_stmnt(StmntNode *stmnt) {
    JCC_PROFILE();

    llvm::BasicBlock *insert_block = m_builder->GetInsertBlock();
    if (!insert_block) {
        llvm::BasicBlock *bb =
            llvm::BasicBlock::Create(*m_context, "", insert_block->getParent());
        m_builder->SetInsertPoint(bb);
    } else if (insert_block->getTerminator()) {
        return; // someone wrote code after a return or something
    }

    switch (stmnt->kind) {
    case LabelStmnt:
    case CaseStmnt:
    case DefaultStmnt:
        ice(false);
    case IfStmnt:
        gen_if_stmnt(static_cast<IfStmntNode *>(stmnt));
        break;
    case SwitchStmnt:
        ice(false);
    case WhileStmnt:
        gen_while_stmnt(static_cast<WhileStmntNode *>(stmnt));
        break;
    case DoStmnt:
        gen_do_stmnt(static_cast<DoStmntNode *>(stmnt));
        break;
    case ForStmnt:
        gen_for_stmnt(static_cast<ForStmntNode *>(stmnt));
        break;
    case GotoStmnt:
    case ContinueStmnt:
    case BreakStmnt:
        ice(false);
    case ReturnStmnt:
        m_builder->CreateRet(
            gen_expr(static_cast<ReturnStmntNode *>(stmnt)->expr));
        break;
    case CompoundStmnt:
        gen_compound_stmnt(static_cast<CompoundStmntNode *>(stmnt));
        break;
    case ExprStmnt:
        // TODO result not used
        // make a genExpr that doesn't bother loading it?
        // can be reused for comma expr as well
        gen_expr(static_cast<ExprStmntNode *>(stmnt)->expr);
        break;
    default:
        ice(false);
    }
}

llvm::Function *LLVMIRGen::gen_prototype(PrototypeNode *proto) {
    JCC_PROFILE();

    std::vector<llvm::Type *> arg_types;
    for (auto arg : proto->args) {
        arg_types.push_back(to_llvm_type(arg->type));
    }

    llvm::FunctionType *ft = llvm::FunctionType::get(
        to_llvm_type(proto->ret_type), arg_types, false);

    llvm::Function *f = llvm::Function::Create(
        ft, llvm::Function::ExternalLinkage, proto->id.value(), m_module.get());

    if (!proto->attr._extern) {
        int index = 0;
        for (auto &arg : f->args()) {
            arg.setName(proto->args[index]->id.value());
            ++index;
        }
    }

    return f;
}

llvm::Function *LLVMIRGen::gen_function(FunctionNode *fn) {
    JCC_PROFILE();

    llvm::Function *function = m_module->getFunction(fn->proto->id.value());

    if (function)
        ice(false);

    function = gen_prototype(fn->proto);

    if (!function)
        ice(false);

    llvm::BasicBlock *bb =
        llvm::BasicBlock::Create(*m_context, "entry", function);
    m_builder->SetInsertPoint(bb);

    // FIXME clean up
    m_named_values.clear();
    int i = 0;
    for (auto &arg : function->args()) {
        auto decl = fn->proto->args[i];
        auto arg_val = &arg;
        m_builder->CreateAlignedStore(arg_val, gen_decl(decl),
                                      llvm::Align(decl->type->align), false);

        ++i;
    }

    gen_compound_stmnt(fn->body);

    if (!function->back().getTerminator()) {
        auto &last = function->back();
        m_builder->SetInsertPoint(&last);

        if(function->getName() == "main") {
            m_builder->CreateRet(
                llvm::ConstantInt::get(*m_context, llvm::APInt(function->getReturnType()->getPrimitiveSizeInBits(), 0, true)));
        }
        else {
            m_builder->CreateCall(llvm::Intrinsic::getDeclaration(m_module.get(), llvm::Intrinsic::trap), {});
            m_builder->CreateUnreachable();
        }
    }

    return function;
}

void LLVMIRGen::gen_file(FileNode *file) {
    JCC_PROFILE();

    for (auto *f : file->functions) {
        if (f->body == nullptr)
            gen_prototype(f->proto);
        else
            gen_function(f);
    }
}
