#include "sema.h"

#include <ranges>

using namespace jcc;

Sema::Sema() : type_tab{}, fn_tab{}, var_tab{}, current_fn{nullptr} {}

// TODO intern
// only works for variables, maybe rename: get_var_type()
CType *Sema::get_id_type(std::string *id) {
    JCC_PROFILE();

    for (auto &m : var_tab | std::views::reverse)
        if (m.contains(*id))
            return m[*id]->type;

    ice(false);
    return nullptr;
}

// TODO intern
void Sema::add_var(DeclNode *decl) {
    JCC_PROFILE();

    var_tab.back()[*decl->id] = decl;
}

ExprNode *Sema::insert_cast(ExprNode *expr, CType *type) {
    JCC_PROFILE();

    return UnaryExprNode::create_cast(expr, type);
}

ExprNode *Sema::maybe_insert_cast(ExprNode *expr, CType *type) {
    JCC_PROFILE();

    if (expr->type != type)
        return insert_cast(expr, type);
    return expr;
}

// TODO pointers, arrays, functions
CType *Sema::get_type(CType *lhs, CType *rhs) {
    JCC_PROFILE();

    if (lhs->type == CTypeKind::Double || rhs->type == CTypeKind::Double)
        return CType::getBuiltinType(CTypeKind::Double);
    if (lhs->type == CTypeKind::Float || rhs->type == CTypeKind::Float)
        return CType::getBuiltinType(CTypeKind::Float);

    switch (lhs->type) {
    case CTypeKind::Bool:
    case CTypeKind::Char:
    case CTypeKind::Short:
        lhs = CType::getBuiltinType(CTypeKind::Int);
    default:
        break;
    }
    switch (rhs->type) {
    case CTypeKind::Bool:
    case CTypeKind::Char:
    case CTypeKind::Short:
        rhs = CType::getBuiltinType(CTypeKind::Int);
    default:
        break;
    }

    if (!lhs->is_signed)
        return lhs;
    if (!rhs->is_signed)
        return rhs;

    return lhs;
}

ExprNode *Sema::fold_expr(ExprNode *expr) {
    return nullptr;
}

void Sema::sema_num_lit_expr(NumLitExprNode *num_lit_expr) {
    JCC_PROFILE();

    // TODO deduce type
    num_lit_expr->type = CType::getBuiltinType(CTypeKind::LLong);
}

void Sema::sema_str_lit_expr(StrLitExprNode *str_lit_expr) {
    JCC_PROFILE();

    // TODO const char pointer
    str_lit_expr->type = CType::getBuiltinType(CTypeKind::Pointer);
}

void Sema::sema_id_expr(IdExprNode *id_expr) {
    JCC_PROFILE();

    id_expr->type = get_id_type(id_expr->val);
}

void Sema::sema_unary_expr(UnaryExprNode *unary_expr) {
    JCC_PROFILE();

    if (unary_expr->base)
        sema_expr(unary_expr->base);

    switch (unary_expr->op) {
    case UnaryOp::_postfix_inc:
    case UnaryOp::_postfix_dec:

    case UnaryOp::_prefix_inc:
    case UnaryOp::_prefix_dec:
        // TODO double check this is correct
        unary_expr->type = unary_expr->base->type;
        unary_expr->base =
            maybe_insert_cast(unary_expr->base, unary_expr->type);
        break;

    case UnaryOp::_sizeof:
    case UnaryOp::__Alignof:
        unary_expr->type = CType::getBuiltinType(CTypeKind::Int);
        break;

    case UnaryOp::_address:
        unary_expr->type = CType::pointer_to(unary_expr->base->type);
        break;
    case UnaryOp::_deref:
        unary_expr->type = unary_expr->base->type->base;
        break;

    case UnaryOp::_add:
    case UnaryOp::_sub:
        unary_expr->type = CType::getBuiltinType(CTypeKind::Int);
        unary_expr->base =
            maybe_insert_cast(unary_expr->base, unary_expr->type);
        break;

    case UnaryOp::_bit_not:
        break;
    case UnaryOp::_log_not:
        unary_expr->type = CType::getBuiltinType(CTypeKind::Bool);
        unary_expr->base =
            maybe_insert_cast(unary_expr->base, unary_expr->type);
        break;

    case UnaryOp::_cast:
        sema_cast_expr(unary_expr);
    default:
        break;
    }
}

void Sema::sema_call_expr(CallExprNode *call_expr) {
    JCC_PROFILE();

    ice(call_expr->base->kind == ExprKind::IdExpr);
    // sema_expr(call_expr->base);
    FunctionNode *callee =
        fn_tab[*static_cast<IdExprNode *>(call_expr->base)->val];

    for (int i = 0; i < call_expr->args.size(); ++i) {
        sema_expr(call_expr->args[i]);
        call_expr->args[i] =
            maybe_insert_cast(call_expr->args[i], callee->proto->args[i]->type);
    }

    call_expr->type = callee->proto->ret_type;
}

void Sema::sema_bin_expr(BinExprNode *bin_expr) {
    JCC_PROFILE();

    sema_expr(bin_expr->lhs);
    sema_expr(bin_expr->rhs);

    bool skip_lhs = false;
    CType *final_expr_type = nullptr;

    switch (bin_expr->op) {
    case BinOp::_less_than:
    case BinOp::_greater_than:
    case BinOp::_less_than_equal:
    case BinOp::_greater_than_equal:
    case BinOp::_equal:
    case BinOp::_not_equal:
        final_expr_type = CType::getBuiltinType(CTypeKind::Bool);
        break;
    case BinOp::_log_and:
    case BinOp::_log_or:
        ice(false);
    case BinOp::_assign: // TODO ensure lhs is lvalue
        skip_lhs = true;
        bin_expr->type = bin_expr->lhs->type;
        break;
    case BinOp::_comma:
        skip_lhs = true;
        break;
    default:
        break;
    }

    if (!skip_lhs) {
        bin_expr->type = get_type(bin_expr->lhs->type, bin_expr->rhs->type);
        bin_expr->lhs = maybe_insert_cast(bin_expr->lhs, bin_expr->type);
    }
    bin_expr->rhs = maybe_insert_cast(bin_expr->rhs, bin_expr->type);

    if (final_expr_type)
        bin_expr->type = final_expr_type;
}

void Sema::sema_cond_expr(CondExprNode *cond_expr) {
    JCC_PROFILE();

    ice(false);
}

void Sema::sema_cast_expr(UnaryExprNode *cast_expr) {
    JCC_PROFILE();

    sema_expr(cast_expr->base);
}

void Sema::sema_expr(ExprNode *expr) {
    JCC_PROFILE();

    fold_expr(expr);

    switch (expr->kind) {
    case NumLitExpr:
        sema_num_lit_expr(static_cast<NumLitExprNode *>(expr));
        break;
    case StrLitExpr:
        sema_str_lit_expr(static_cast<StrLitExprNode *>(expr));
        break;
    case IdExpr:
        sema_id_expr(static_cast<IdExprNode *>(expr));
        break;
    case CallExpr:
        sema_call_expr(static_cast<CallExprNode *>(expr));
        break;
    case PrimaryExpr:
        ice(false);
        break;
    case PostfixExpr:
        ice(false);
        break;
    case UnaryExpr:
        sema_unary_expr(static_cast<UnaryExprNode *>(expr));
        break;
    case BinExpr:
        sema_bin_expr(static_cast<BinExprNode *>(expr));
        break;
    case CondExpr:
        sema_cond_expr(static_cast<CondExprNode *>(expr));
        break;
    default:
        ice(false);
    }
}

void Sema::sema_decl(DeclNode *decl) {
    JCC_PROFILE();

    if (decl->id)
        add_var(decl);

    if (decl->init) {
        sema_expr(decl->init);
        decl->init = maybe_insert_cast(decl->init, decl->type);
    }
}

void Sema::sema_case_stmnt(CaseStmntNode *case_stmnt) {
    JCC_PROFILE();

    ice(false);
}

void Sema::sema_default_stmnt(DefaultStmntNode *default_stmnt) {
    JCC_PROFILE();

    ice(false);
}

void Sema::sema_if_stmnt(IfStmntNode *if_stmnt) {
    JCC_PROFILE();

    sema_expr(if_stmnt->cond);
    if_stmnt->cond = maybe_insert_cast(if_stmnt->cond,
                                       CType::getBuiltinType(CTypeKind::Bool));

    sema_stmnt(if_stmnt->true_branch);
    if (if_stmnt->false_branch)
        sema_stmnt(if_stmnt->false_branch);
}

void Sema::sema_switch_stmnt(SwitchStmntNode *switch_stmnt) {
    JCC_PROFILE();

    ice(false);
}

void Sema::sema_while_stmnt(WhileStmntNode *while_stmnt) {
    JCC_PROFILE();

    sema_expr(while_stmnt->cond);
    while_stmnt->cond = maybe_insert_cast(
        while_stmnt->cond, CType::getBuiltinType(CTypeKind::Bool));

    sema_stmnt(while_stmnt->body);
}

void Sema::sema_do_stmnt(DoStmntNode *do_stmnt) {
    JCC_PROFILE();

    sema_stmnt(do_stmnt->body);

    sema_expr(do_stmnt->cond);
    do_stmnt->cond = maybe_insert_cast(do_stmnt->cond,
                                       CType::getBuiltinType(CTypeKind::Bool));
}

void Sema::sema_for_stmnt(ForStmntNode *for_stmnt) {
    JCC_PROFILE();

    sema_decl(for_stmnt->init);

    sema_expr(for_stmnt->cond);
    for_stmnt->cond = maybe_insert_cast(for_stmnt->cond,
                                        CType::getBuiltinType(CTypeKind::Bool));

    sema_expr(for_stmnt->inc);

    sema_stmnt(for_stmnt->body);
}

void Sema::sema_goto_stmnt(GotoStmntNode *goto_stmnt) {
    JCC_PROFILE();

    ice(false);
}

void Sema::sema_return_stmnt(ReturnStmntNode *ret_stmnt) {
    JCC_PROFILE();

    sema_expr(ret_stmnt->expr);

    ret_stmnt->expr =
        maybe_insert_cast(ret_stmnt->expr, current_fn->proto->ret_type);
}

void Sema::sema_label_stmnt(LabelStmntNode *label_stmnt) {
    JCC_PROFILE();

    ice(false);
}

void Sema::sema_expr_stmnt(ExprStmntNode *expr_stmnt) {
    JCC_PROFILE();

    sema_expr(expr_stmnt->expr);
}

void Sema::sema_compound_stmnt(CompoundStmntNode *compound_stmnt) {
    JCC_PROFILE();

    var_tab.emplace_back();

    for (auto *decl : compound_stmnt->decl_list) {
        sema_decl(decl);
    }

    for (auto *stmnt : compound_stmnt->stmnt_list) {
        sema_stmnt(stmnt);
    }

    var_tab.pop_back();
}

void Sema::sema_stmnt(StmntNode *stmnt) {
    JCC_PROFILE();

    switch (stmnt->kind) {
    case LabelStmnt:
        sema_label_stmnt(static_cast<LabelStmntNode *>(stmnt));
        break;
    case CaseStmnt:
        sema_case_stmnt(static_cast<CaseStmntNode *>(stmnt));
        break;
    case DefaultStmnt:
        sema_default_stmnt(static_cast<DefaultStmntNode *>(stmnt));
        break;
    case IfStmnt:
        sema_if_stmnt(static_cast<IfStmntNode *>(stmnt));
        break;
    case SwitchStmnt:
        sema_switch_stmnt(static_cast<SwitchStmntNode *>(stmnt));
        break;
    case WhileStmnt:
        sema_while_stmnt(static_cast<WhileStmntNode *>(stmnt));
        break;
    case DoStmnt:
        sema_do_stmnt(static_cast<DoStmntNode *>(stmnt));
        break;
    case ForStmnt:
        sema_for_stmnt(static_cast<ForStmntNode *>(stmnt));
        break;
    case GotoStmnt:
        sema_goto_stmnt(static_cast<GotoStmntNode *>(stmnt));
        break;
    case ContinueStmnt:
        break;
    case BreakStmnt:
        break;
    case ReturnStmnt:
        sema_return_stmnt(static_cast<ReturnStmntNode *>(stmnt));
        break;
    case CompoundStmnt:
        sema_compound_stmnt(static_cast<CompoundStmntNode *>(stmnt));
        break;
    case ExprStmnt:
        sema_expr_stmnt(static_cast<ExprStmntNode *>(stmnt));
        break;
    default:
        ice(false);
    }
}

void Sema::sema_prototype(PrototypeNode *proto) {
    JCC_PROFILE();

    // TODO add self to symbol table?
    fn_tab[*proto->id] = current_fn;

    for (auto *arg : proto->args) {
        sema_decl(arg);
    }
}

void Sema::sema_function(FunctionNode *fn) {
    JCC_PROFILE();

    current_fn = fn;

    var_tab.emplace_back();

    sema_prototype(fn->proto);
    if (fn->body)
        sema_stmnt(fn->body);

    var_tab.pop_back();
}

void Sema::sema_file(FileNode *file) {
    JCC_PROFILE();

    for (auto *fn : file->functions) {
        sema_function(fn);
    }
}

bool Sema::run_on(FileNode *file) {
    JCC_PROFILE();

    sema_file(file);
    return true;
}
