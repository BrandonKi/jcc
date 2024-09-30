#include "jb_ir_gen.h"
#include "ast.h"

#include "jb.h"

#include <memory>
#include <vector>

using namespace jcc;


JBIRGen::JBIRGen() {
    JCC_PROFILE();

    m_context = {};
    m_builder = m_context.new_module_builder("test");

    m_named_values = {};
}

// TODO
jb::Type JBIRGen::to_jb_type(CType *type) {
    JCC_PROFILE();
    if(type->id.has_value()) {
        assert(false);
        // auto cached = jb::StructType::getTypeByName(*m_context, type->id.value());
        // if(cached)
        //     return cached;
    }

    switch (type->type) {
    case Char:
        return jb::Type::i8;
    case Short:
        return jb::Type::i16;
    case Int:
        return jb::Type::i32;
    case Long:
        return jb::Type::i32;
    case LLong:
        return jb::Type::i64;
    case Float:
        return jb::Type::f32;
    case Double:
        return jb::Type::f64;
    case Void:
        // return jb::Type::getVoidTy(*m_context);
        return jb::Type::none; // TODO void
    case Pointer:
        return jb::Type::ptr;
    case Struct:
    case Union: {
        // std::vector<jb::Type*> types;
        // for(auto *field: type->fields) {
        //     types.push_back(to_jb_type(field->type));
        // }
        // if(type->type == CTypeKind::Struct)
        //     return jb::StructType::create(*m_context, types, type->id.value());
        ice(false);
        // return jb::UnionType::create(*m_context, types, type->id.value());
    }
    case Array:
    case Function:
        ice(false);
    case Bool:
        return jb::Type::i8;
    case Enum:
    default:
        ice(false);
    }
    ice(false);
    return jb::Type::none;
}

jb::IRValue JBIRGen::gen_str_lit_expr(StrLitExprNode *str_expr) {
    JCC_PROFILE();

    // jb::IRValue v = m_builder->CreateGlobalString(str_expr->val.value());
    // return v;
    return {};
}

jb::IRValue JBIRGen::gen_num_lit_expr(NumLitExprNode *num_expr) {
    JCC_PROFILE();
    
    return m_builder->iconst64(num_expr->val);
}

jb::IRValue JBIRGen::gen_id_expr(IdExprNode *id_expr) {
    JCC_PROFILE();

    auto [decl, val] = m_named_values[id_expr->val.value()];
    if (val.type == jb::Type::none)
        ice(false);
    val = m_builder->stack_load(val, to_jb_type(decl->type));
    return val;
}

// FIXME does not handle all valid cases
jb::IRValue JBIRGen::gen_lvalue_expr(ExprNode *base_expr) {
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
    // case BinExpr: { // union/struct/array access
    //     auto expr = static_cast<BinExprNode *>(base_expr);
    //     ice(expr->op == BinOp::_field);
    //     auto lvalue = gen_lvalue_expr(expr->lhs);
    //     auto expr_rhs = static_cast<IdExprNode *>(expr->rhs);
    //     int index = -1;
    //     for(auto *field: expr->lhs->type->fields) {
    //         ++index;
    //         if(field->id == expr_rhs->val)
    //             break;
    //     }
    //     ice(index != -1);
    //     return m_builder->CreateStructGEP(to_jb_type(expr->lhs->type), lvalue, index);
    // }
    default:
        ice(false);
    }

    return {};
}

jb::IRValue JBIRGen::gen_address_of(ExprNode *base_expr) {
    JCC_PROFILE();

    return gen_lvalue_expr(base_expr);
}

// TODO finish
jb::IRValue JBIRGen::gen_unary_expr(UnaryExprNode *unary_expr) {
    JCC_PROFILE();

    jb::IRValue expr;

    switch (unary_expr->op) {
    case UnaryOp::_postfix_inc: {
        return m_builder->iadd(gen_expr(unary_expr->base), m_builder->iconst64(1));
    }
    case UnaryOp::_postfix_dec: {
        return m_builder->isub(gen_expr(unary_expr->base), m_builder->iconst64(1));
    }
    case UnaryOp::_prefix_inc: {
        return m_builder->iadd(gen_expr(unary_expr->base), m_builder->iconst64(1));
    }
    case UnaryOp::_prefix_dec: {
        return m_builder->isub(gen_expr(unary_expr->base), m_builder->iconst64(1));
    }
    case UnaryOp::_sizeof: {
        if (unary_expr->base) {
            return m_builder->iconst64(unary_expr->base->type->size);
        }
        return m_builder->iconst64(unary_expr->base_type->size);
    }
    case UnaryOp::__Alignof: {
        return m_builder->iconst64(unary_expr->base_type->align);
    }
    case UnaryOp::_address: {
        return gen_address_of(unary_expr->base);
    }
    case UnaryOp::_deref: {
        expr = gen_expr(unary_expr->base);
        jb::IRValue val = m_builder->load(expr, to_jb_type(unary_expr->type));
        return val;
    }
    case UnaryOp::_add:
        expr = gen_expr(unary_expr->base);
        return expr;
    case UnaryOp::_sub:
        expr = gen_expr(unary_expr->base);
        expr = m_builder->isub(m_builder->iconst64(0), expr);
        return expr;
    case UnaryOp::_bit_not:
    case UnaryOp::_log_not:
        ice(false);
    case UnaryOp::_cast:
        return gen_cast_expr(unary_expr);
    default:
        ice(false);
    }
    return {};
}

jb::IRValue JBIRGen::gen_assign(BinExprNode *bin_expr) {
    JCC_PROFILE();

    jb::IRValue lhs = gen_lvalue_expr(bin_expr->lhs);
    jb::IRValue rhs = gen_expr(bin_expr->rhs);

    m_builder->stack_store(lhs, rhs);
    return rhs;
}

jb::IRValue JBIRGen::gen_bin_expr(BinExprNode *bin_expr) {
    JCC_PROFILE();
    jb::IRValue lhs;
    jb::IRValue rhs;

    switch (bin_expr->op) {
    case BinOp::_log_and:
        // TODO need to short circuit
        break;
    case BinOp::_log_or:
        // TODO need to short circuit
        break;
    case BinOp::_field: {
        auto addr = gen_lvalue_expr(bin_expr);
        // addr = m_builder->CreateAlignedLoad(to_jb_type(bin_expr->type), addr,
        //                                 jb::Align(bin_expr->type->align));
        return addr;
    }
    case BinOp::_assign:
        return gen_assign(bin_expr);
    default:
        break;
        // just continue on
    }

    lhs = gen_expr(bin_expr->lhs);
    rhs = gen_expr(bin_expr->rhs);
    // if (!lhs || !rhs)
    //     ice(false);

    switch (bin_expr->op) {
    case BinOp::_mul:
        return m_builder->imul(lhs, rhs);
    case BinOp::_div:
        return m_builder->idiv(lhs, rhs);
    case BinOp::_mod:
        return m_builder->imod(lhs, rhs);
    case BinOp::_add:
        return m_builder->iadd(lhs, rhs);
    case BinOp::_sub:
        return m_builder->isub(lhs, rhs);
    case BinOp::_bitshift_left:
        return m_builder->bsl(lhs, rhs);
    case BinOp::_bitshift_right:
        return m_builder->bsr(lhs, rhs);
    //     // TODO sema, so that this as well as many other things can work
    //     // correctly, tldr casts need to be inserted
    case BinOp::_less_than:
        lhs = m_builder->lt(lhs, rhs);
        // lhs = m_builder->CreateZExt(lhs, jb::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_greater_than:
        lhs = m_builder->gt(lhs, rhs);
        // lhs = m_builder->CreateZExt(lhs, jb::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_less_than_equal:
        lhs = m_builder->lte(lhs, rhs);
        // lhs = m_builder->CreateZExt(lhs, jb::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_greater_than_equal:
        lhs = m_builder->gte(lhs, rhs);
        // lhs = m_builder->CreateZExt(lhs, jb::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_equal:
        lhs = m_builder->eq(lhs, rhs);
        // lhs = m_builder->CreateZExt(lhs, jb::Type::getInt32Ty(*m_context));
        return lhs;
    case BinOp::_not_equal:
        lhs = m_builder->neq(lhs, rhs);
        return lhs;
    case BinOp::_bit_and:
        return m_builder->band(lhs, rhs);
    case BinOp::_xor:
        return m_builder->bxor(lhs, rhs);
    case BinOp::_bit_or:
        return m_builder->bor(lhs, rhs);
    // case BinOp::_log_and:
    // case BinOp::_log_or:
    // case BinOp::_field:
    // case BinOp::_assign:
    default:
        ice(false);
    }

    ice(false);
    return {};
}

jb::IRValue JBIRGen::gen_cast_expr(UnaryExprNode *cast_expr) {
    JCC_PROFILE();

    jb::IRValue val = gen_expr(cast_expr->base);

    // TODO cast
    return val;
}

jb::IRValue JBIRGen::gen_call_expr(CallExprNode *call_expr) {
    JCC_PROFILE();
    
    // FIXME hack
    if (call_expr->base->kind != ExprKind::IdExpr)
        ice(false);
    auto str = static_cast<IdExprNode *>(call_expr->base)->val.value();
    jb::Function *callee = m_functions[static_cast<IdExprNode *>(call_expr->base)->val.value()];

    // llvm::Function *callee =
    //     m_module->getFunction(static_cast<IdExprNode *>(call_expr->base)->val.value());

    if (!callee)
        ice(false);

    if (callee->params.size() != call_expr->args.size())
        ice(false);

    std::vector<jb::IRValue> args;

    for (auto *a : call_expr->args) {
        args.push_back(gen_expr(a));
        if (args.back().type == jb::Type::none)
            ice(false);
    }

    return m_builder->call(callee, args);
}

jb::IRValue JBIRGen::gen_expr(ExprNode *expr) {
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
    return {};
}

jb::IRValue JBIRGen::gen_decl(DeclNode *decl) {
    JCC_PROFILE();

    if(decl->id.empty() && (decl->type->type == CTypeKind::Struct || decl->type->type == CTypeKind::Union))
        return jb::IRValue();
        // return nullptr;

    // auto saved_ip = m_builder->saveIP();
    // m_builder->SetInsertPointPastAllocas(m_builder->GetInsertBlock()->getParent());
    
    // auto alloc = m_builder->CreateAlloca(to_jb_type(decl->type));
    auto alloc = m_builder->slot(to_jb_type(decl->type));

    // m_builder->restoreIP(saved_ip);
    if (decl->init)
        m_builder->stack_store(alloc, gen_expr(decl->init));
        // m_builder->CreateAlignedStore(gen_expr(decl->init), alloc,
        //                               jb::Align(decl->type->align), false);
    

    m_named_values[decl->id] = {decl, alloc};
    return alloc;
}

// FIXME rewrite, possibly use similar logic to switch
void JBIRGen::gen_if_stmnt(IfStmntNode *if_stmnt) {
    JCC_PROFILE();

    jb::Function *function = m_builder->function;

    jb::IRValue cond_val = gen_expr(if_stmnt->cond);

    jb::BasicBlock *orig_block = m_builder->insert_point;

    jb::BasicBlock *true_block = m_builder->newBB("true");

    jb::BasicBlock *cont_block = m_builder->newBB("cont");

    jb::BasicBlock *false_block = cont_block;
    if (if_stmnt->false_branch)
        false_block = m_builder->newBB("false");

    m_builder->setInsertPoint(orig_block);
    m_builder->brnz(cond_val, true_block, false_block);

    m_builder->setInsertPoint(true_block);
    gen_stmnt(if_stmnt->true_branch);
    if (!true_block->terminator()) {
        m_builder->br(cont_block);
    }

    if (if_stmnt->false_branch) {
        // function->insert(function->end(), false_block);

        if (!m_builder->insert_point->terminator())
            m_builder->br(cont_block);

        m_builder->setInsertPoint(false_block);
        gen_stmnt(if_stmnt->false_branch);
        if (!false_block->terminator()) {
            m_builder->br(cont_block);
        }
    }

    // function->insert(function->end(), cont_block);

    if (!m_builder->insert_point->terminator())
        m_builder->br(cont_block);

    m_builder->setInsertPoint(cont_block);
}

void JBIRGen::gen_while_stmnt(WhileStmntNode *while_stmnt) {
    JCC_PROFILE();

    jb::Function *function = m_builder->function;

    jb::BasicBlock *orig_block = m_builder->insert_point;
    jb::BasicBlock *cond_block = m_builder->newBB("");
    jb::BasicBlock *body_block = m_builder->newBB("");
    jb::BasicBlock *cont_block = m_builder->newBB("");

    m_builder->setInsertPoint(orig_block);
    m_builder->br(cond_block);

    m_builder->setInsertPoint(cond_block);
    jb::IRValue cond = gen_expr(while_stmnt->cond);
    m_builder->brnz(cond, body_block, cont_block);

    m_builder->setInsertPoint(body_block);
    gen_stmnt(while_stmnt->body);
    m_builder->br(cond_block);

    // function->insert(function->end(), cont_block);

    if (!m_builder->insert_point->terminator())
        m_builder->br(cont_block);

    m_builder->setInsertPoint(cont_block);
}

void JBIRGen::gen_do_stmnt(DoStmntNode *do_stmnt) {
    JCC_PROFILE();
    
    jb::Function *function = m_builder->function;

    jb::BasicBlock *orig_block = m_builder->insert_point;
    jb::BasicBlock *cond_block = m_builder->newBB("");
    jb::BasicBlock *body_block = m_builder->newBB("");
    jb::BasicBlock *cont_block = m_builder->newBB("");

    m_builder->setInsertPoint(orig_block);
    m_builder->br(body_block);

    m_builder->setInsertPoint(body_block);
    gen_stmnt(do_stmnt->body);
    m_builder->br(cond_block);

    m_builder->setInsertPoint(cond_block);
    jb::IRValue cond = gen_expr(do_stmnt->cond);
    m_builder->brnz(cond, body_block, cont_block);

    // function->insert(function->end(), cont_block);

    if (!m_builder->insert_point->terminator())
        m_builder->br(cont_block);

    m_builder->setInsertPoint(cont_block);
}

void JBIRGen::gen_for_stmnt(ForStmntNode *for_stmnt) {
    JCC_PROFILE();

    jb::Function *function = m_builder->function;

    jb::BasicBlock *orig_block = m_builder->insert_point;
    jb::BasicBlock *cond_block = m_builder->newBB("cond");
    jb::BasicBlock *body_block = m_builder->newBB("body");
    jb::BasicBlock *inc_block = m_builder->newBB("inc");
    jb::BasicBlock *cont_block = m_builder->newBB("cont");

    m_builder->setInsertPoint(orig_block);
    gen_decl(for_stmnt->init);
    m_builder->br(cond_block);

    m_builder->setInsertPoint(cond_block);
    jb::IRValue cond = gen_expr(for_stmnt->cond);
    m_builder->brnz(cond, body_block, cont_block);

    m_builder->setInsertPoint(body_block);
    gen_stmnt(for_stmnt->body);
    if (!m_builder->insert_point->terminator())
        m_builder->br(inc_block);

    // function->insert(function->end(), inc_block);

    m_builder->setInsertPoint(inc_block);
    gen_expr(for_stmnt->inc);
    m_builder->br(cond_block);

    // function->insert(function->end(), cont_block);

    if (!m_builder->insert_point->terminator())
        m_builder->br(cont_block);

    m_builder->setInsertPoint(cont_block);
}

void JBIRGen::gen_compound_stmnt(CompoundStmntNode *stmnts) {
    JCC_PROFILE();

    for (auto *decl : stmnts->decl_list) {
        gen_decl(decl);
    }

    for (auto *stmnt : stmnts->stmnt_list) {
        gen_stmnt(stmnt);
    }
}

void JBIRGen::gen_stmnt(StmntNode *stmnt) {
    JCC_PROFILE();

    jb::BasicBlock *insert_block = m_builder->insert_point;
    if (!insert_block) {
        static int cnt = 0;
        jb::BasicBlock *bb = m_builder->newBB(std::to_string(cnt++));
        // jb::BasicBlock *bb =
        //     jb::BasicBlock::Create(*m_context, "", insert_block->getParent());
        m_builder->setInsertPoint(bb);
    }
    // else if (insert_block->getTerminator()) {
    //     return; // someone wrote code after a return or something
    // }

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
        m_builder->ret(gen_expr(static_cast<ReturnStmntNode *>(stmnt)->expr));
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

jb::Function *JBIRGen::gen_prototype(PrototypeNode *proto) {
    JCC_PROFILE();

    std::vector<jb::Type> arg_types;
    for (auto arg : proto->args) {
        arg_types.push_back(to_jb_type(arg->type));
    }

    jb::Function *f = m_builder->newFn(proto->id.value(), arg_types, jb::Type::i32, jb::CallConv::win64, false);

    return f;
}

jb::Function *JBIRGen::gen_function(FunctionNode *fn) {
    JCC_PROFILE();

    jb::Function *function = gen_prototype(fn->proto);
    m_functions[function->id] = function;

    if (!function)
        ice(false);

    jb::BasicBlock *bb = m_builder->newBB("entry");
    m_builder->setInsertPoint(bb);

    // FIXME clean up
    m_named_values.clear();
    int i = 0;
    for (auto &arg : function->params) {
        auto decl = fn->proto->args[i];
        // auto arg_val = &arg;
        m_builder->stack_store(gen_decl(decl), arg);
        // m_builder->CreateAlignedStore(arg_val, gen_decl(decl),
        //                               jb::Align(decl->type->align), false);

        ++i;
    }

    gen_compound_stmnt(fn->body);

    if (!function->blocks.back()->terminator()) {
        auto &last = function->blocks.back();
        m_builder->setInsertPoint(last);

        if(function->id == "main") {
            m_builder->ret(jb::IRConstantInt(0, 32));
            // m_builder->CreateRet(
            //     jb::ConstantInt::get(*m_context, jb::APInt(function->getReturnType()->getPrimitiveSizeInBits(), 0, true)));
        }
        else {
            ice(false);
            // m_builder->CreateCall(jb::Intrinsic::getDeclaration(m_module.get(), jb::Intrinsic::trap), {});
            // m_builder->CreateUnreachable();
        }
    }

    return function;
}

void JBIRGen::gen_file(FileNode *file) {
    JCC_PROFILE();

    for (auto *f : file->functions) {
        if (f->body == nullptr)
            gen_prototype(f->proto);
        else
            gen_function(f);
    }
}
