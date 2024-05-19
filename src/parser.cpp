#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "platform.h"

#include <cassert>
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace jcc;

CType CType::getBuiltinType(CTypeKind type, bool is_signed) {
    return Platform::builtinTypes[(int)type][(int)is_signed];
}

Parser::Parser() : m_lex{}, m_tags{} {}

Parser::Parser(Lexer lex) : m_lex{lex}, m_tags{} {}

// TODO incomplete
CType Parser::parse_type() {
    CType type = {};

    // FIXME, allows signed/unsigned aggregate types...
    Token t1 = m_lex.curr();
    if (t1.kind == TokenKind::k_signed || t1.kind == TokenKind::k_unsigned) {
        t1 = m_lex.next();
        type.is_signed = (t1.kind == TokenKind::k_signed);
    }
    switch (t1.kind) {
    case TokenKind::k_void:
        type = CType::getBuiltinType(CTypeKind::Void, type.is_signed);
        break;
    case TokenKind::k__Bool:
        type = CType::getBuiltinType(CTypeKind::Bool, type.is_signed);
        break;
    case TokenKind::k_char:
        type = CType::getBuiltinType(CTypeKind::Char, type.is_signed);
        break;
    case TokenKind::k_short:
        type = CType::getBuiltinType(CTypeKind::Short, type.is_signed);
        break;
    case TokenKind::k_int:
        type = CType::getBuiltinType(CTypeKind::Int, type.is_signed);
        break;
    case TokenKind::k_long:
        type = CType::getBuiltinType(CTypeKind::Long, type.is_signed);
        break;
    case TokenKind::k_float:
        type = CType::getBuiltinType(CTypeKind::Float, type.is_signed);
        break;
    case TokenKind::k_double:
        type = CType::getBuiltinType(CTypeKind::Double, type.is_signed);
        break;
    case TokenKind::k_struct:

    default:
        return CType{CTypeKind::None};
    }

    m_lex.next();
    return type;
}

// TODO use clang trick
bool Parser::is_start_of_type(Token tkn) {
    switch (tkn.kind) {
    case _id:
        return m_tags.contains(*tkn.id.val);
    case k_char:
    case k_double:
    case k_float:
    case k_int:
    case k_short:
    case k_long:
    case k_signed:
    case k_unsigned:
    case k_void:
    case k_enum:
    case k_struct:
    case k_union:
    case k_auto:
    case k_const:
    case k_extern:
    case k_inline:
    case k_register:
    case k_restrict:
    case k_static:
    case k_volatile:
    case k_typedef:
    case k__Bool:
        return true;
    default:
        return false;
    }
}

// TODO why is this separate
PrototypeNode *Parser::parse_extern() {
    m_lex.eat(TokenKind::k_extern);
    return parse_prototype();
}

PrototypeNode *Parser::parse_prototype() {
    PrototypeNode *node = new PrototypeNode();

    node->ret_type = parse_type();

    if (node->ret_type.type == CTypeKind::None)
        return nullptr;

    node->id = m_lex.curr().id.val;

    m_lex.eat_next('(');

    while (m_lex.curr().kind != TokenKind::_close_paren) {
        // TODO doesn't do anything with types yet
        node->args.push_back(m_lex.curr().id.val);
        m_lex.next();
    }

    m_lex.eat((TokenKind)')');
    return node;
}

ExprNode *Parser::parse_id_expr() {
    std::string *val = m_lex.curr().id.val;
    m_lex.next();
    return new IdExprNode(val);
}

ExprNode *Parser::parse_num_lit_expr() {
    int val = m_lex.curr().number.val;
    m_lex.next();
    return new NumLitExprNode(val);
}

ExprNode *Parser::parse_str_lit_expr() {
    std::string *val = m_lex.curr().str.val;
    m_lex.next();
    return new StrLitExprNode(val);
}

// primary-expression:
//      identifier
//      constant
//      string-literal
//      ( expression )
//      generic-selection
ExprNode *Parser::parse_primary_expr() {
    ExprNode *expr = nullptr;

    switch (m_lex.curr().kind) {
    case TokenKind::_id:
        expr = parse_id_expr();
        break;
    case TokenKind::_num_lit:
        expr = parse_num_lit_expr();
        break;
    case TokenKind::_str_lit:
        expr = parse_str_lit_expr();
        break;
    case TokenKind::_open_paren:
        m_lex.next();
        expr = parse_expr();
        m_lex.eat(')');
        break;
    default:
        assert(false);
    }
    return expr;
}

std::vector<ExprNode *> Parser::parse_function_args() {
    std::vector<ExprNode *> result = {};

    if (m_lex.curr().kind == TokenKind::_close_paren)
        return result;

    result.push_back(parse_assign_expr());

    while (m_lex.curr().kind == TokenKind::_comma) {
        m_lex.next();
        result.push_back(parse_assign_expr());
    }

    return result;
}

ExprNode *Parser::parse_function_call() {
    return nullptr;
}

// postfix-expression:
//      primary-expression
//      postfix-expression [ expression ]
//      postfix-expression ( argument-expression-list opt )
//      postfix-expression . identifier
//      postfix-expression -> identifier
//      postfix-expression ++
//      postfix-expression --
//      ( type-name ) { initializer-list }
//      ( type-name ) { initializer-list , }
// argument-expression-list:
//      assignment-expression
//      argument-expression-list , assignment-expression
ExprNode *Parser::parse_postfix_expr() {
    ExprNode *base = parse_primary_expr();
    ExprNode *extra = nullptr;

    // FIXME is there a condition instead of true?
    TokenKind op = m_lex.curr().kind;
    while (true) {
        switch (op) {
        case TokenKind::_open_bracket:
            assert(false);
        case TokenKind::_open_paren:
            extra = new CallExprNode(base, parse_function_args());
        case TokenKind::_dot:
            assert(false);
        case TokenKind::_arrow:
            assert(false);
        case TokenKind::_inc:
            return new UnaryExprNode(UnaryOp::_prefix_inc, base);
        case TokenKind::_dec:
            return new UnaryExprNode(UnaryOp::_prefix_dec, base);
        default:
            return base;
        }

        op = m_lex.next().kind;
    }

    assert(false);
}

// unary-expression:
//      postfix-expression
//      ++ unary-expression
//      -- unary-expression
//      unary-operator cast-expression
//      sizeof unary-expression
//      sizeof ( type-name )
//      _Alignof ( type-name )
// unary-operator: one of
//      & * + - ~ !
ExprNode *Parser::parse_unary_expr() {
    ExprNode *expr = nullptr;

    TokenKind op = m_lex.curr().kind;

    // TODO deduplicate code
    switch (op) {
    case TokenKind::_inc:
        m_lex.next();
        expr = parse_cast_expr();
        return new UnaryExprNode(UnaryOp::_prefix_inc, expr);
    case TokenKind::_dec:
        m_lex.next();
        expr = parse_cast_expr();
        return new UnaryExprNode(UnaryOp::_prefix_dec, expr);
    case TokenKind::_and:
        m_lex.next();
        expr = parse_cast_expr();
        return new UnaryExprNode(UnaryOp::_address, expr);
    case TokenKind::_star:
        m_lex.next();
        expr = parse_cast_expr();
        return new UnaryExprNode(UnaryOp::_deref, expr);
    case TokenKind::_add:
        m_lex.next();
        expr = parse_cast_expr();
        return new UnaryExprNode(UnaryOp::_add, expr);
    case TokenKind::_sub:
        m_lex.next();
        expr = parse_cast_expr();
        return new UnaryExprNode(UnaryOp::_sub, expr);
    case TokenKind::_tilde:
        m_lex.next();
        expr = parse_cast_expr();
        return new UnaryExprNode(UnaryOp::_bit_not, expr);
    case TokenKind::_bang:
        m_lex.next();
        expr = parse_cast_expr();
        return new UnaryExprNode(UnaryOp::_log_not, expr);
    default:
        return parse_postfix_expr();
    }

    assert(false);
    return nullptr;
}

// cast-expression:
//      unary-expression
//      ( type-name ) cast-expression
ExprNode *Parser::parse_cast_expr() {
    if (m_lex.curr().kind == TokenKind::_open_paren &&
        is_start_of_type(m_lex.peek())) {
        m_lex.next();
        CType ty = parse_type();
        m_lex.eat(TokenKind::_close_paren);
        ExprNode *cast_expr = parse_cast_expr();
        return new UnaryExprNode(UnaryOp::_cast, ty, cast_expr);
    }

    return parse_unary_expr();
}

// multiplicative-expression:
//      cast-expression
//      multiplicative-expression * cast-expression
//      multiplicative-expression / cast-expression
//      multiplicative-expression % cast-expression
ExprNode *Parser::parse_multiplicative_expr() {
    ExprNode *lhs = parse_cast_expr();

    TokenKind op = m_lex.curr().kind;
    while (op == TokenKind::_star || op == TokenKind::_slash ||
           op == TokenKind::_percent) {
        m_lex.next();
        ExprNode *rhs = parse_cast_expr();

        // TODO TokenKind -> BinOP conversion function
        if (op == TokenKind::_star)
            lhs = new BinExprNode(BinOp::_mul, lhs, rhs);
        else if (op == TokenKind::_slash)
            lhs = new BinExprNode(BinOp::_div, lhs, rhs);
        else if (op == TokenKind::_percent)
            lhs = new BinExprNode(BinOp::_mod, lhs, rhs);

        op = m_lex.curr().kind;
    }
    return lhs;
}

// additive-expression:
//      multiplicative-expression
//      additive-expression + multiplicative-expression
//      additive-expression - multiplicative-expression
ExprNode *Parser::parse_additive_expr() {
    ExprNode *lhs = parse_multiplicative_expr();

    TokenKind op = m_lex.curr().kind;
    while (op == TokenKind::_add || op == TokenKind::_sub) {
        m_lex.next();
        ExprNode *rhs = parse_multiplicative_expr();

        // TODO TokenKind -> BinOP conversion function
        if (op == TokenKind::_add)
            lhs = new BinExprNode(BinOp::_add, lhs, rhs);
        else if (op == TokenKind::_sub)
            lhs = new BinExprNode(BinOp::_sub, lhs, rhs);

        op = m_lex.curr().kind;
    }
    return lhs;
}

// shift-expression:
//      additive-expression
//      shift-expression << additive-expression
//      shift-expression >> additive-expression
ExprNode *Parser::parse_shift_expr() {
    ExprNode *lhs = parse_additive_expr();

    TokenKind op = m_lex.curr().kind;
    while (op == TokenKind::_shift_left || op == TokenKind::_shift_right) {
        m_lex.next();
        ExprNode *rhs = parse_additive_expr();

        // TODO TokenKind -> BinOP conversion function
        if (op == TokenKind::_shift_left)
            lhs = new BinExprNode(BinOp::_bitshift_left, lhs, rhs);
        else if (op == TokenKind::_shift_right)
            lhs = new BinExprNode(BinOp::_bitshift_right, lhs, rhs);

        op = m_lex.curr().kind;
    }
    return lhs;
}

// relational-expression:
//      shift-expression
//      relational-expression < shift-expression
//      relational-expression > shift-expression
//      relational-expression <= shift-expression
//      relational-expression >= shift-expression
ExprNode *Parser::parse_relational_expr() {
    ExprNode *lhs = parse_shift_expr();

    TokenKind op = m_lex.curr().kind;
    while (op == TokenKind::_less_than || op == TokenKind::_greater_than ||
           op == TokenKind::_less_than_equal ||
           op == TokenKind::_greater_than_equal) {
        m_lex.next();
        ExprNode *rhs = parse_shift_expr();

        // TODO TokenKind -> BinOP conversion function
        if (op == TokenKind::_less_than)
            lhs = new BinExprNode(BinOp::_less_than, lhs, rhs);
        else if (op == TokenKind::_greater_than)
            lhs = new BinExprNode(BinOp::_greater_than, lhs, rhs);
        else if (op == TokenKind::_less_than_equal)
            lhs = new BinExprNode(BinOp::_less_than_equal, lhs, rhs);
        else if (op == TokenKind::_greater_than_equal)
            lhs = new BinExprNode(BinOp::_greater_than_equal, lhs, rhs);

        op = m_lex.curr().kind;
    }
    return lhs;
}

// equality-expression:
//      relational-expression
//      equality-expression == relational-expression
//      equality-expression != relational-expression
ExprNode *Parser::parse_equality_expr() {
    ExprNode *lhs = parse_relational_expr();

    TokenKind op = m_lex.curr().kind;
    while (op == TokenKind::_equal_equal || op == TokenKind::_not_equal) {
        m_lex.next();
        ExprNode *rhs = parse_relational_expr();

        // TODO TokenKind -> BinOP conversion function
        if (op == TokenKind::_equal_equal)
            lhs = new BinExprNode(BinOp::_equal, lhs, rhs);
        else if (op == TokenKind::_not_equal)
            lhs = new BinExprNode(BinOp::_not_equal, lhs, rhs);

        op = m_lex.curr().kind;
    }
    return lhs;
}

// AND-expression:
//      equality-expression
//      AND-expression & equality-expression
ExprNode *Parser::parse_bit_and_expr() {
    ExprNode *lhs = parse_equality_expr();

    while (m_lex.curr().kind == TokenKind::_and) {
        m_lex.next();
        ExprNode *rhs = parse_equality_expr();

        lhs = new BinExprNode(BinOp::_bit_and, lhs, rhs);
    }
    return lhs;
}

// exclusive-OR-expression:
//      AND-expression
//      exclusive-OR-expression ^ AND-expression
ExprNode *Parser::parse_xor_expr() {
    ExprNode *lhs = parse_bit_and_expr();

    while (m_lex.curr().kind == TokenKind::_xor) {
        m_lex.next();
        ExprNode *rhs = parse_bit_and_expr();

        lhs = new BinExprNode(BinOp::_xor, lhs, rhs);
    }
    return lhs;
}

// inclusive-OR-expression:
//      exclusive-OR-expression
//      inclusive-OR-expression | exclusive-OR-expression
ExprNode *Parser::parse_bit_or_expr() {
    ExprNode *lhs = parse_xor_expr();

    while (m_lex.curr().kind == TokenKind::_or) {
        m_lex.next();
        ExprNode *rhs = parse_xor_expr();

        lhs = new BinExprNode(BinOp::_bit_or, lhs, rhs);
    }
    return lhs;
}

// logical-AND-expression:
//      inclusive-OR-expression
//      logical-AND-expression && inclusive-OR-expression
ExprNode *Parser::parse_and_expr() {
    ExprNode *lhs = parse_bit_or_expr();

    while (m_lex.curr().kind == TokenKind::_log_and) {
        m_lex.next();
        ExprNode *rhs = parse_bit_or_expr();

        lhs = new BinExprNode(BinOp::_log_and, lhs, rhs);
    }
    return lhs;
}

// logical-OR-expression:
//      logical-AND-expression
//      logical-OR-expression || logical-AND-expression
ExprNode *Parser::parse_or_expr() {
    ExprNode *lhs = parse_and_expr();

    while (m_lex.curr().kind == TokenKind::_log_or) {
        m_lex.next();
        ExprNode *rhs = parse_and_expr();

        lhs = new BinExprNode(BinOp::_log_or, lhs, rhs);
    }
    return lhs;
}

// conditional-expression:
//      logical-OR-expression
//      logical-OR-expression ? expression : conditional-expression
ExprNode *Parser::parse_conditional_expr() {
    ExprNode *expr = parse_or_expr();

    while (m_lex.peek().kind == TokenKind::_question_mark) {
        m_lex.next();
        ExprNode *true_branch = parse_expr();
        m_lex.eat(':');
        ExprNode *false_branch = parse_conditional_expr();
        expr = new CondExprNode(expr, true_branch, false_branch);
    }

    return expr;
}

// assignment-expression:
//      conditional-expression
//      unary-expression assignment-operator assignment-expression
// assignment-operator: one of
//      = *= /= %= += -= <<= >>= &= ^= |=
ExprNode *Parser::parse_assign_expr() {
    ExprNode *cond_expr = parse_conditional_expr();
    if (cond_expr)
        return cond_expr;

    // TODO implement
    return nullptr;
}

// expression:
//      assignment-expression
//      expression , assignment-expression
ExprNode *Parser::parse_comma_expr() {
    ExprNode *lhs = parse_assign_expr();

    while (m_lex.curr().kind == TokenKind::_comma) {
        m_lex.next();
        ExprNode *rhs = parse_comma_expr();

        lhs = new BinExprNode(BinOp::_comma, lhs, rhs);
    }
    return lhs;
}

ExprNode *Parser::parse_expr() {
    return parse_comma_expr();
}

// TOOD this is going to be a long one
// only a portion of syntax from the spec is below
//
// declaration:
//      declaration-specifiers init-declarator-list opt ;
//      static_assert-declaration
// declaration-specifiers:
//      storage-class-specifier declaration-specifiersopt
//      type-specifier declaration-specifiersopt
//      type-qualifier declaration-specifiersopt
//      function-specifier declaration-specifiersopt
//      alignment-specifier declaration-specifiersopt
// init-declarator-list:
//      init-declarator
//      init-declarator-list , init-declarator
// init-declarator:
//      declarator
//      declarator = initializer
DeclNode *Parser::parse_decl(TokenKind terminator) {
    DeclNode *decl = new DeclNode();

    decl->type = parse_type();

    if (m_lex.curr().kind != TokenKind::_id)
        return nullptr;

    decl->id = m_lex.curr().id.val;
    m_lex.next();

    if (m_lex.curr().kind == terminator)
        return decl;

    if (m_lex.curr().kind == TokenKind::_equal) {
        m_lex.next();
        decl->init = parse_assign_expr();
    }

    m_lex.eat(terminator);
    return decl;
}
CaseStmntNode *Parser::parse_case_stmnt() {
    assert(false);
    return nullptr;
}

DefaultStmntNode *Parser::parse_default_stmnt() {
    assert(false);
    return nullptr;
}

IfStmntNode *Parser::parse_if_stmnt() {
    assert(false);
    return nullptr;
}

SwitchStmntNode *Parser::parse_switch_stmnt() {
    assert(false);
    return nullptr;
}

WhileStmntNode *Parser::parse_while_stmnt() {
    assert(false);
    return nullptr;
}

DoStmntNode *Parser::parse_do_stmnt() {
    assert(false);
    return nullptr;
}

ForStmntNode *Parser::parse_for_stmnt() {
    assert(false);
    return nullptr;
}

GotoStmntNode *Parser::parse_goto_stmnt() {
    assert(false);
    return nullptr;
}

ContinueStmntNode *Parser::parse_continue_stmnt() {
    assert(false);
    return nullptr;
}

BreakStmntNode *Parser::parse_break_stmnt() {
    assert(false);
    return nullptr;
}

ReturnStmntNode *Parser::parse_return_stmnt() {
    ReturnStmntNode *stmnt = new ReturnStmntNode();
    m_lex.next();
    stmnt->expr = parse_expr();

    return stmnt;
}

LabelStmntNode *Parser::parse_label_stmnt() {
    assert(false);
    return nullptr;
}

ExprStmntNode *Parser::parse_expr_stmnt() {
    assert(false);
    return nullptr;
}

// statement:
//      labeled-statement
//      compound-statement
//      expression-statement
//      selection-statement
//      iteration-statement
//      jump-statement
//
// labeled-statement:
//      identifier : statement
//      case constant-expression : statement
//      default : statement
//
// compound-statement:
//      { block-item-list opt }
// block-item-list:
//      block-item
//      block-item-list block-item
// block-item:
//      declaration
//      statement
//
// expression-statement:
//      expression opt ;
//
// selection-statement:
//      if ( expression ) statement
//      if ( expression ) statement else statement
//      switch ( expression ) statement
//
// iteration-statement:
//      while ( expression ) statement
//      do statement while ( expression ) ;
//      for ( expressionopt ; expression opt ; expression opt ) statement
//      for ( declaration expression opt ; expression opt ) statement
//
//  jump-statement:
//      goto identifier ;
//      continue ;
//      break ;
//      return expression opt ;
//
StmntNode *Parser::parse_stmnt() {
    StmntNode *stmnt = nullptr;

    switch (m_lex.curr().kind) {
    // labeled-statement
    case k_case:
        stmnt = parse_case_stmnt();
        break;
    case k_default:
        stmnt = parse_default_stmnt();
        break;
    // compound-statement
    case TokenKind::_open_curly:
        stmnt = parse_compound_stmnt();
        break;
    // selection-statement
    case k_if:
        stmnt = parse_if_stmnt();
        break;
    case k_switch:
        stmnt = parse_switch_stmnt();
        break;
    // iteration-statement
    case k_while:
        stmnt = parse_while_stmnt();
        break;
    case k_do:
        stmnt = parse_do_stmnt();
        break;
    case k_for:
        stmnt = parse_for_stmnt();
        break;
    // jump-statement
    case k_goto:
        stmnt = parse_goto_stmnt();
        break;
    case k_continue:
        stmnt = parse_continue_stmnt();
        break;
    case k_break:
        stmnt = parse_break_stmnt();
        break;
    case k_return:
        stmnt = parse_return_stmnt();
        break;
    default:
        // TODO handle the other cases
        // label statement and expression statement cases
        parse_label_stmnt();
        parse_expr_stmnt();
    }
    m_lex.eat(';');
    return stmnt;
}

// compound-statement:
//      { block-item-list opt }
//      block-item-list:
//          block-item
//          block-item-list block-item
//      block-item:
//          declaration
//          statement
CompoundStmntNode *Parser::parse_compound_stmnt() {
    CompoundStmntNode *node = new CompoundStmntNode();

    m_lex.eat('{');
    while (m_lex.curr().kind != TokenKind::_close_curly) {
        DeclNode *decl = nullptr;
        StmntNode *stmnt = nullptr;

        decl = parse_decl();
        if (decl) {
            node->decl_list.push_back(decl);
        } else {
            stmnt = parse_stmnt();
            node->stmnt_list.push_back(stmnt);
        }
    }
    m_lex.eat('}');

    return node;
}

FunctionNode *Parser::parse_function() {

    auto proto = parse_prototype();
    if (!proto)
        return nullptr;

    auto cs = parse_compound_stmnt();
    if (!cs)
        return nullptr;

    return new FunctionNode(proto, cs);
}

FileNode *Parser::parse_file() {
    FileNode *file = new FileNode;

    m_lex.next();
    while (m_lex.curr().kind != TokenKind::_eof) {
        if (is_start_of_type(m_lex.curr())) {
            file->functions.push_back(parse_function());
        }
    }

    return file;
}
