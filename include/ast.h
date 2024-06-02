#pragma once

#include "common.h"
#include "lexer.h"
#include <string>

namespace jcc {

#define ARENA_CREATE_CUSTOM(name, alloc_fn)                                    \
    template <typename... T>                                                   \
    static name *create(T... ts) {                                             \
        return alloc_fn(ts...);                                                \
    }

#define ARENA_CREATE(name) ARENA_CREATE_CUSTOM(name, new (name))

// TODO, arenas
// #define ARENA_CREATE(name) ARENA_CREATE_CUSTOM(name,
// name##_arena.emplace_back)

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
    CType *base;
    int align;
    int size;
    bool is_signed;

    CType()
        : type{CTypeKind::None}, base{nullptr}, align{1}, size{1},
          is_signed{true} {}
    CType(CTypeKind type)
        : type{type}, base{nullptr}, align{1}, size{1}, is_signed{true} {}
    CType(CTypeKind type, CType *ptr)
        : type{type}, base{ptr},
          align{CType::getBuiltinType(CTypeKind::Pointer)->align},
          size{CType::getBuiltinType(CTypeKind::Pointer)->size},
          is_signed{true} {}
    CType(CTypeKind type, int align, int size)
        : type{type}, base{nullptr}, align{align}, size{size}, is_signed{true} {
    }
    CType(CTypeKind type, int align, int size, bool is_signed)
        : type{type}, base{nullptr}, align{align}, size{size},
          is_signed{is_signed} {}
    CType(CTypeKind type, CType *ptr, int align, int size, bool is_signed)
        : type{type}, base{ptr}, align{align}, size{size},
          is_signed{is_signed} {}

    static CType *getBuiltinType(CTypeKind type, bool is_signed = true);

    bool is_int_type() {
        return (type >= Char && type <= LLong);
    }

    static CType *pointer_to(CType *base) {
        CType *plt_ptr = CType::getBuiltinType(CTypeKind::Pointer);
        auto *temp = CType::create(CTypeKind::Pointer, base, plt_ptr->align,
                                   plt_ptr->size, plt_ptr->is_signed);
        return temp;
    }

    ARENA_CREATE(CType);
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
    BinExpr,
    CondExpr,
};

struct ExprNode {
    ExprKind kind;
    CType *type = CType::getBuiltinType(CTypeKind::None);

    ARENA_CREATE(ExprNode);
};

struct NumLitExprNode final : public ExprNode {
    int val;

    NumLitExprNode() : ExprNode{ExprKind::NumLitExpr}, val{0} {}

    NumLitExprNode(int val) : ExprNode{ExprKind::NumLitExpr}, val{val} {}

    ARENA_CREATE(NumLitExprNode);
};

struct StrLitExprNode final : public ExprNode {
    std::string *val;

    StrLitExprNode() : ExprNode{ExprKind::StrLitExpr}, val{nullptr} {}

    StrLitExprNode(std::string *val)
        : ExprNode{ExprKind::StrLitExpr}, val{val} {}

    ARENA_CREATE(StrLitExprNode);
};

struct IdExprNode final : public ExprNode {
    std::string *val; // TODO intern

    IdExprNode() : ExprNode{ExprKind::IdExpr}, val{nullptr} {}

    IdExprNode(std::string *val) : ExprNode{ExprKind::IdExpr}, val{val} {}

    ARENA_CREATE(IdExprNode);
};

struct CallExprNode final : public ExprNode {
    ExprNode *base;
    std::vector<ExprNode *> args;

    CallExprNode() : ExprNode{ExprKind::CallExpr}, base{nullptr}, args{} {}

    CallExprNode(ExprNode *base, std::vector<ExprNode *> args)
        : ExprNode{ExprKind::CallExpr}, base{base}, args{args} {}

    ARENA_CREATE(CallExprNode);
};

enum class UnaryOp : char {
    _none,

    _postfix_inc,
    _postfix_dec,

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

    _cast
};

// NOTE base and base_type should not be active at the same time
// could be a union
struct UnaryExprNode final : public ExprNode {
    ExprNode *base;
    CType *base_type;
    UnaryOp op;

    UnaryExprNode()
        : ExprNode{ExprKind::UnaryExpr, CType::getBuiltinType(CTypeKind::None)},
          base{nullptr}, base_type{nullptr}, op{0} {}

    UnaryExprNode(UnaryOp op, ExprNode *expr)
        : ExprNode{ExprKind::UnaryExpr, CType::getBuiltinType(CTypeKind::None)},
          base{expr}, base_type{nullptr}, op{op} {}

    UnaryExprNode(UnaryOp op, CType *type)
        : ExprNode{ExprKind::UnaryExpr}, base{nullptr}, base_type{type},
          op{op} {}

    UnaryExprNode(UnaryOp op, ExprNode *expr, CType *type)
        : ExprNode{ExprKind::UnaryExpr}, base{expr}, base_type{type}, op{op} {}

    static UnaryExprNode *create_cast(ExprNode *expr, CType *type) {
        auto *result = UnaryExprNode::create(UnaryOp::_cast, expr, type);
        result->type = type;
        return result;
    }

    ARENA_CREATE(UnaryExprNode);
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

    ARENA_CREATE(BinExprNode);
};

// TODO just generate IfStmntNode in the parser instead
struct CondExprNode final : public ExprNode {
    ExprNode *cond_expr, *true_branch, *false_branch;

    CondExprNode()
        : ExprNode{ExprKind::CondExpr}, cond_expr{nullptr},
          true_branch{nullptr}, false_branch{nullptr} {}

    CondExprNode(ExprNode *cond_expr, ExprNode *true_branch,
                 ExprNode *false_branch)
        : ExprNode{ExprKind::CondExpr}, cond_expr{cond_expr},
          true_branch{true_branch}, false_branch{false_branch} {}

    ARENA_CREATE(CondExprNode);
};

// FIXME not all of these can be active at one time
// there are some rules
struct Attributes {
    bool _typedef : 1;
    bool _extern : 1;
    bool _static : 1;
    bool __Thread_local : 1;
    bool _auto : 1;
    bool _register : 1;
};

// TODO unused
enum Qualifier : char {
    q_const,
    q_restrict,
    q_volatile,
    q_Atomic,
};

struct DeclNode {
    Attributes attr;
    Qualifier qual;
    CType *type;
    std::string *id;
    ExprNode *init;

    DeclNode()
        : attr{}, qual{}, type{CType::getBuiltinType(CTypeKind::None)},
          id{nullptr}, init{nullptr} {}

    ARENA_CREATE(DeclNode);
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

// TODO it's weird having these two separate from switch
// I think?
struct CaseStmntNode final : public StmntNode {

    CaseStmntNode() : StmntNode{StmntKind::CaseStmnt} {}

    ARENA_CREATE(CaseStmntNode);
};

struct DefaultStmntNode final : public StmntNode {

    DefaultStmntNode() : StmntNode{StmntKind::DefaultStmnt} {}

    ARENA_CREATE(DefaultStmntNode);
};

struct IfStmntNode final : public StmntNode {
    ExprNode *cond;
    StmntNode *true_branch, *false_branch;
    // StmntNode *then_branch;

    IfStmntNode() : StmntNode{StmntKind::IfStmnt} {}
    IfStmntNode(ExprNode *cond, StmntNode *true_branch, StmntNode *false_branch)
        : StmntNode{StmntKind::IfStmnt}, cond{cond}, true_branch{true_branch},
          false_branch{false_branch} {}

    ARENA_CREATE(IfStmntNode);
};

struct SwitchStmntNode final : public StmntNode {

    SwitchStmntNode() : StmntNode{StmntKind::SwitchStmnt} {}

    ARENA_CREATE(SwitchStmntNode);
};

struct WhileStmntNode final : public StmntNode {
    ExprNode *cond;
    StmntNode *body;

    WhileStmntNode()
        : StmntNode{StmntKind::WhileStmnt}, cond{nullptr}, body{nullptr} {}

    WhileStmntNode(ExprNode *cond, StmntNode *body)
        : StmntNode{StmntKind::WhileStmnt}, cond{cond}, body{body} {}

    ARENA_CREATE(WhileStmntNode);
};

struct DoStmntNode final : public StmntNode {
    StmntNode *body;
    ExprNode *cond;

    DoStmntNode()
        : StmntNode{StmntKind::DoStmnt}, body{nullptr}, cond{nullptr} {}

    DoStmntNode(StmntNode *body, ExprNode *cond)
        : StmntNode{StmntKind::DoStmnt}, body{body}, cond{cond} {}

    ARENA_CREATE(DoStmntNode);
};

struct ForStmntNode final : public StmntNode {
    DeclNode *init;
    ExprNode *cond, *inc;
    StmntNode *body;

    ForStmntNode()
        : StmntNode{StmntKind::ForStmnt}, init{nullptr}, cond{nullptr},
          inc{nullptr}, body{nullptr} {}

    ForStmntNode(DeclNode *init, ExprNode *cond, ExprNode *inc, StmntNode *body)
        : StmntNode{StmntKind::ForStmnt}, init{init}, cond{cond}, inc{inc},
          body{body} {}

    ARENA_CREATE(ForStmntNode);
};

struct GotoStmntNode final : public StmntNode {

    GotoStmntNode() : StmntNode{StmntKind::GotoStmnt} {}

    ARENA_CREATE(GotoStmntNode);
};

struct ContinueStmntNode final : public StmntNode {

    ContinueStmntNode() : StmntNode{StmntKind::ContinueStmnt} {}

    ARENA_CREATE(ContinueStmntNode);
};

struct BreakStmntNode final : public StmntNode {

    BreakStmntNode() : StmntNode{StmntKind::BreakStmnt} {}

    ARENA_CREATE(BreakStmntNode);
};

struct ReturnStmntNode final : public StmntNode {
    ExprNode *expr;

    ReturnStmntNode() : StmntNode{StmntKind::ReturnStmnt}, expr{nullptr} {}

    ReturnStmntNode(ExprNode *expr)
        : StmntNode{StmntKind::ReturnStmnt}, expr{expr} {}

    ARENA_CREATE(ReturnStmntNode);
};

struct LabelStmntNode final : public StmntNode {

    LabelStmntNode() : StmntNode{StmntKind::LabelStmnt} {}

    ARENA_CREATE(LabelStmntNode);
};

struct ExprStmntNode final : public StmntNode {
    ExprNode *expr;

    ExprStmntNode() : StmntNode{StmntKind::ExprStmnt}, expr{nullptr} {}

    ExprStmntNode(ExprNode *expr)
        : StmntNode{StmntKind::ExprStmnt}, expr{expr} {}

    ARENA_CREATE(ExprStmntNode);
};

struct CompoundStmntNode final : public StmntNode {
    std::vector<DeclNode *> decl_list;
    std::vector<StmntNode *> stmnt_list;

    CompoundStmntNode()
        : StmntNode{StmntKind::CompoundStmnt}, decl_list{}, stmnt_list{} {}

    ARENA_CREATE(CompoundStmntNode);
};

struct PrototypeNode {
    Attributes attr;
    CType *ret_type;
    std::string *id;
    std::vector<DeclNode *> args; // TODO intern

    PrototypeNode()
        : attr{}, ret_type{CType::getBuiltinType(CTypeKind::None)}, id{nullptr},
          args{} {}

    PrototypeNode(Attributes attr, CType *ret_type, std::string *id,
                  std::vector<DeclNode *> args)
        : attr{attr}, ret_type{ret_type}, id{id}, args{args} {}

    ARENA_CREATE(PrototypeNode);
};

struct FunctionNode {
    PrototypeNode *proto = nullptr;
    CompoundStmntNode *body = nullptr;
    // bool is_inline : 1;
    // bool is_noreturn : 1;

    FunctionNode(PrototypeNode *proto) : proto{proto}, body{nullptr} {}

    FunctionNode(PrototypeNode *proto, CompoundStmntNode *body)
        : proto{proto}, body{body} {}

    ARENA_CREATE(FunctionNode);
};

struct FileNode {
    std::vector<FunctionNode *> functions = {};

    ARENA_CREATE(FileNode);
};

} // namespace jcc
