#pragma once

#include "lexer.h"
#include "ast.h"

#include <unordered_map>

namespace jcc {

class Parser {
    Lexer m_lex;

    // TODO follow spec
    // std::unordered_map<std::string, > function_scope;
    // file_scope
    // block_scope
    // prototype_scope

    // TODO intern
    std::unordered_map<std::string, CType> m_tags;

    CType parse_type();
    bool is_start_of_type(Token);

    ExprNode *parse_id_expr();
    ExprNode *parse_num_lit_expr();
    ExprNode *parse_str_lit_expr();

    ExprNode *parse_primary_expr();

    std::vector<ExprNode *> parse_function_args();
    ExprNode *parse_function_call();

    ExprNode *parse_postfix_expr();
    ExprNode *parse_unary_expr();
    ExprNode *parse_cast_expr();
    ExprNode *parse_multiplicative_expr();
    ExprNode *parse_additive_expr();
    ExprNode *parse_shift_expr();
    ExprNode *parse_relational_expr();
    ExprNode *parse_equality_expr();
    ExprNode *parse_bit_and_expr();
    ExprNode *parse_xor_expr();
    ExprNode *parse_bit_or_expr();
    ExprNode *parse_and_expr();
    ExprNode *parse_or_expr();
    ExprNode *parse_conditional_expr();
    ExprNode *parse_assign_expr();
    ExprNode *parse_comma_expr();
    ExprNode *parse_expr();

    DeclNode *parse_decl(TokenKind terminator = TokenKind::_semicolon);

    CaseStmntNode *parse_case_stmnt();
    DefaultStmntNode *parse_default_stmnt();
    IfStmntNode *parse_if_stmnt();
    SwitchStmntNode *parse_switch_stmnt();
    WhileStmntNode *parse_while_stmnt();
    DoStmntNode *parse_do_stmnt();
    ForStmntNode *parse_for_stmnt();
    GotoStmntNode *parse_goto_stmnt();
    ContinueStmntNode *parse_continue_stmnt();
    BreakStmntNode *parse_break_stmnt();
    ReturnStmntNode *parse_return_stmnt();
    LabelStmntNode *parse_label_stmnt();
    ExprStmntNode *parse_expr_stmnt();
    CompoundStmntNode *parse_compound_stmnt();
    StmntNode *parse_stmnt();

    PrototypeNode *parse_extern();
    PrototypeNode *parse_prototype();
    FunctionNode *parse_function();

    FileNode *parse_top_level();

public:
    Parser();
    Parser(Lexer lex);

    FileNode *parse_file();
};

} // namespace jcc
