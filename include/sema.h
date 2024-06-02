#pragma once

#include "ast.h"

namespace jcc {

class Sema {
    std::vector<std::unordered_map<std::string, CType *>> type_tab;
    std::unordered_map<std::string, FunctionNode *> fn_tab;
    std::vector<std::unordered_map<std::string, DeclNode *>> var_tab;

    FunctionNode *current_fn;

public:
    Sema();

    CType *get_id_type(std::string *);
    void add_var(DeclNode *);

    ExprNode *insert_cast(ExprNode *, CType *);
    ExprNode *maybe_insert_cast(ExprNode *, CType *);

    ExprNode *apply_promotion(ExprNode *);
    CType *get_type(CType *, CType *);

    ExprNode *fold_expr(ExprNode *);

    void sema_num_lit_expr(NumLitExprNode *);
    void sema_str_lit_expr(StrLitExprNode *);
    void sema_id_expr(IdExprNode *);
    void sema_call_expr(CallExprNode *);
    void sema_cast_expr(CastExprNode *);
    void sema_unary_expr(UnaryExprNode *);
    void sema_bin_expr(BinExprNode *);
    void sema_cond_expr(CondExprNode *);
    void sema_expr(ExprNode *);

    void sema_decl(DeclNode *);

    void sema_case_stmnt(CaseStmntNode *);
    void sema_default_stmnt(DefaultStmntNode *);
    void sema_if_stmnt(IfStmntNode *);
    void sema_switch_stmnt(SwitchStmntNode *);
    void sema_while_stmnt(WhileStmntNode *);
    void sema_do_stmnt(DoStmntNode *);
    void sema_for_stmnt(ForStmntNode *);
    void sema_goto_stmnt(GotoStmntNode *);
    void sema_return_stmnt(ReturnStmntNode *);
    void sema_label_stmnt(LabelStmntNode *);
    void sema_expr_stmnt(ExprStmntNode *);
    void sema_compound_stmnt(CompoundStmntNode *);
    void sema_stmnt(StmntNode *);

    void sema_prototype(PrototypeNode *);
    void sema_function(FunctionNode *);
    void sema_file(FileNode *);
    bool run_on(FileNode *);
};

} // namespace jcc
