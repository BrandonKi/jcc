#pragma once

#include "ast.h"

#include "jb.h"
#include "context.h"


namespace jcc {

class JBIRGen {
public:
    jb::Context m_context;
    jb::ModuleBuilder *m_builder;
    std::unordered_map<std::string, std::pair<DeclNode *, jb::IRValue>> m_named_values;
    std::unordered_map<std::string, jb::Function *> m_functions;

    JBIRGen();

    jb::Type to_jb_type(CType *);

    jb::IRValue gen_str_lit_expr(StrLitExprNode *);
    jb::IRValue gen_num_lit_expr(NumLitExprNode *);
    jb::IRValue gen_id_expr(IdExprNode *);
    jb::IRValue gen_lvalue_expr(ExprNode *);
    jb::IRValue gen_address_of(ExprNode *);
    jb::IRValue gen_unary_expr(UnaryExprNode *);
    jb::IRValue gen_assign(BinExprNode *);
    jb::IRValue gen_bin_expr(BinExprNode *);
    jb::IRValue gen_cast_expr(UnaryExprNode *);
    jb::IRValue gen_call_expr(CallExprNode *);
    jb::IRValue gen_expr(ExprNode *);

    jb::IRValue gen_decl(DeclNode *);

    void gen_if_stmnt(IfStmntNode *);
    void gen_while_stmnt(WhileStmntNode *);
    void gen_do_stmnt(DoStmntNode *);
    void gen_for_stmnt(ForStmntNode *);
    void gen_compound_stmnt(CompoundStmntNode *);
    void gen_stmnt(StmntNode *);

    jb::Function *gen_prototype(PrototypeNode *);
    jb::Function *gen_function(FunctionNode *);

    void gen_file(FileNode *);
};

} // namespace jcc
