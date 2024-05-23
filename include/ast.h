#pragma once

#include "common.h"
#include "lexer.h"
#include <string>

namespace jcc {

enum CTypeKind : int {
    None,

    Char,
    Short,
    Int,
    Long,
    LLong,

    Float,
    Double,

    Void,

    Pointer,

    Bool,

    Function,

    Enum,
    Struct,
    Union,
    Array,

};

// TODO this can be compressed
class CType {
public:
    CTypeKind type;
    CType *ptr;
    int align;
    bool is_signed;

    CType() : type{CTypeKind::None}, ptr{nullptr}, align{1}, is_signed{true} {}
    CType(CTypeKind type)
        : type{type}, ptr{nullptr}, align{1}, is_signed{true} {}
    CType(CTypeKind type, CType *ptr)
        : type{type}, ptr{ptr},
          align{CType::getBuiltinType(CTypeKind::Pointer)->align},
          is_signed{true} {}
    CType(CTypeKind type, int align)
        : type{type}, ptr{nullptr}, align{align}, is_signed{true} {}
    CType(CTypeKind type, int align, bool is_signed)
        : type{type}, ptr{nullptr}, align{align}, is_signed{is_signed} {}
    CType(CTypeKind type, CType *ptr, int align, bool is_signed)
        : type{type}, ptr{ptr}, align{align}, is_signed{is_signed} {}

    static CType *getBuiltinType(CTypeKind type, bool is_signed = true);
};

struct AstNode {};

enum ExprKind {
    NumLitExpr,
    StrLitExpr,
    IdExpr,
    CallExpr,
    PrimaryExpr,
    PostfixExpr,
    UnaryExpr,
    CastExpr,
    BinExpr,
    CondExpr,
};

struct ExprNode {
    ExprKind kind;
    CType *type = CType::getBuiltinType(CTypeKind::None);
};

struct NumLitExprNode final : public ExprNode {
    int val;

    NumLitExprNode() : ExprNode{ExprKind::NumLitExpr}, val{0} {}

    NumLitExprNode(int val) : ExprNode{ExprKind::NumLitExpr}, val{val} {}
};

struct StrLitExprNode final : public ExprNode {
    std::string *val;

    StrLitExprNode() : ExprNode{ExprKind::StrLitExpr}, val{nullptr} {}

    StrLitExprNode(std::string *val)
        : ExprNode{ExprKind::StrLitExpr}, val{val} {}
};

struct IdExprNode final : public ExprNode {
    std::string *val; // TODO intern

    IdExprNode() : ExprNode{ExprKind::IdExpr}, val{nullptr} {}

    IdExprNode(std::string *val) : ExprNode{ExprKind::IdExpr}, val{val} {}
};

enum class UnaryOp : char {
    _none,

    _prefix_inc,
    _prefix_dec,

    _sizeof,
    __Alignof,

    _address,
    _deref,

    _add,
    _sub,

    _bit_not,
    _log_not,

    _cast,
};

struct UnaryExprNode final : public ExprNode {
    ExprNode *expr;
    CType *cast_to;
    UnaryOp op;

    UnaryExprNode()
        : ExprNode{ExprKind::UnaryExpr}, expr{nullptr}, op{0},
          cast_to{CType::getBuiltinType(CTypeKind::None)} {}

    UnaryExprNode(UnaryOp op, ExprNode *expr)
        : ExprNode{ExprKind::UnaryExpr}, expr{expr}, op{op},
          cast_to{CType::getBuiltinType(CTypeKind::None)} {}

    UnaryExprNode(UnaryOp op, CType *cast_to, ExprNode *expr)
        : ExprNode{ExprKind::UnaryExpr}, expr{expr}, op{op}, cast_to{cast_to} {}
};

struct CallExprNode final : public ExprNode {
    ExprNode *base;
    std::vector<ExprNode *> args;

    CallExprNode() : ExprNode{ExprKind::CallExpr}, base{nullptr}, args{} {}

    CallExprNode(ExprNode *base, std::vector<ExprNode *> args)
        : ExprNode{ExprKind::CallExpr}, base{base}, args{args} {}
};

enum class BinOp : char {
    _none,

    _mul,
    _div,
    _mod,

    _add,
    _sub,

    _bitshift_left,
    _bitshift_right,

    _less_than,
    _greater_than,
    _less_than_equal,
    _greater_than_equal,

    _equal,
    _not_equal,

    _bit_and,

    _xor,

    _bit_or,

    _log_and,

    _log_or,

    _assign,

    _comma,
};

struct BinExprNode final : public ExprNode {
    ExprNode *lhs, *rhs;
    BinOp op;

    BinExprNode() : ExprNode{ExprKind::BinExpr}, lhs{nullptr}, rhs{nullptr} {}

    BinExprNode(BinOp op, ExprNode *lhs, ExprNode *rhs)
        : ExprNode{ExprKind::BinExpr}, lhs{lhs}, rhs{rhs}, op{op} {}
};

struct CondExprNode final : public ExprNode {
    ExprNode *cond_expr, *true_branch, *false_branch;

    CondExprNode()
        : ExprNode{ExprKind::CondExpr}, cond_expr{nullptr},
          true_branch{nullptr}, false_branch{nullptr} {}

    CondExprNode(ExprNode *cond_expr, ExprNode *true_branch,
                 ExprNode *false_branch)
        : ExprNode{ExprKind::CondExpr}, cond_expr{cond_expr},
          true_branch{true_branch}, false_branch{false_branch} {}
};

struct Attributes {
    bool _typedef : 1;
    bool _extern : 1;
    bool _static : 1;
    bool __Thread_local : 1;
    bool _auto : 1;
    bool _register : 1;
};

// TODO this doesn't do anything
enum Qualifier : char {
    q_const,
    q_restrict,
    q_volatile,
    q_Atomic,
};

struct DeclNode {
    Attributes attribs;
    Qualifier qual;
    CType *type;
    std::string *id;
    ExprNode *init;

    DeclNode()
        : attribs{}, qual{}, type{CType::getBuiltinType(CTypeKind::None)},
          id{nullptr}, init{nullptr} {}
};

enum StmntKind {
    LabelStmnt,
    CaseStmnt,
    DefaultStmnt,
    IfStmnt,
    SwitchStmnt,
    WhileStmnt,
    DoStmnt,
    ForStmnt,
    GotoStmnt,
    ContinueStmnt,
    BreakStmnt,
    ReturnStmnt,
    CompoundStmnt,
    ExprStmnt,
};

struct StmntNode {
    StmntKind kind;
};

struct CaseStmntNode final : public StmntNode {

    CaseStmntNode() : StmntNode{StmntKind::CaseStmnt} {}
};

struct DefaultStmntNode final : public StmntNode {

    DefaultStmntNode() : StmntNode{StmntKind::DefaultStmnt} {}
};

struct IfStmntNode final : public StmntNode {

    IfStmntNode() : StmntNode{StmntKind::IfStmnt} {}
};

struct SwitchStmntNode final : public StmntNode {

    SwitchStmntNode() : StmntNode{StmntKind::SwitchStmnt} {}
};

struct WhileStmntNode final : public StmntNode {

    WhileStmntNode() : StmntNode{StmntKind::WhileStmnt} {}
};

struct DoStmntNode final : public StmntNode {

    DoStmntNode() : StmntNode{StmntKind::DoStmnt} {}
};

struct ForStmntNode final : public StmntNode {

    ForStmntNode() : StmntNode{StmntKind::ForStmnt} {}
};

struct GotoStmntNode final : public StmntNode {

    GotoStmntNode() : StmntNode{StmntKind::GotoStmnt} {}
};

struct ContinueStmntNode final : public StmntNode {

    ContinueStmntNode() : StmntNode{StmntKind::ContinueStmnt} {}
};

struct BreakStmntNode final : public StmntNode {

    BreakStmntNode() : StmntNode{StmntKind::BreakStmnt} {}
};

struct ReturnStmntNode final : public StmntNode {
    ExprNode *expr;

    ReturnStmntNode() : StmntNode{StmntKind::ReturnStmnt}, expr{nullptr} {}

    ReturnStmntNode(ExprNode *expr)
        : StmntNode{StmntKind::ReturnStmnt}, expr{expr} {}
};

struct LabelStmntNode final : public StmntNode {

    LabelStmntNode() : StmntNode{StmntKind::LabelStmnt} {}
};

struct ExprStmntNode final : public StmntNode {
    ExprNode *expr;

    ExprStmntNode() : StmntNode{StmntKind::ExprStmnt}, expr{nullptr} {}

    ExprStmntNode(ExprNode *expr)
        : StmntNode{StmntKind::ExprStmnt}, expr{expr} {}
};

struct CompoundStmntNode final : public StmntNode {
    std::vector<DeclNode *> decl_list;
    std::vector<StmntNode *> stmnt_list;

    CompoundStmntNode()
        : StmntNode{StmntKind::CompoundStmnt}, decl_list{}, stmnt_list{} {}
};

struct PrototypeNode {
    Attributes attribs;
    CType *ret_type;
    std::string *id;
    std::vector<DeclNode *> args; // TODO intern

    PrototypeNode()
        : attribs{}, ret_type{CType::getBuiltinType(CTypeKind::None)},
          id{nullptr}, args{} {}

    PrototypeNode(Attributes attribs, CType *ret_type, std::string *id,
                  std::vector<DeclNode *> args)
        : attribs{attribs}, ret_type{ret_type}, id{id}, args{args} {}
};

struct FunctionNode {
    PrototypeNode *proto = nullptr;
    CompoundStmntNode *body = nullptr;

    FunctionNode(PrototypeNode *proto, CompoundStmntNode *body)
        : proto{proto}, body{body} {}
};

struct FileNode {
    bool is_inline : 1;
    bool is_noreturn : 1;

    // TODO merge these into one
    std::vector<PrototypeNode *> prototypes = {};
    std::vector<FunctionNode *> functions = {};
};

} // namespace jcc
