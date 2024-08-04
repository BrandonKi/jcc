#pragma once

#include "common.h"
#include <unordered_map>
#include <unordered_set>

namespace jcc {

#define X(a, b, _) a = b,
enum TokenKind : char {

#include "inc/token_kinds.inc"

};
#undef X

// TODO intern
using HideSet = std::unordered_set<std::string>;

struct Token {
    TokenKind kind;
    bool start_of_line;

    HideSet hide_set;

    union {
        struct {
            std::string *val; // TODO intern
        } id;
        struct {
            i64 val; // TODO float/int
        } number;
        struct {
            std::string *val;
        } str;
    };
};

enum class MacroKind : char { none, object_like, function_like };

struct Macro {
    MacroKind kind;
    // TODO intern
    std::unordered_map<std::string, int> params;

    std::vector<Token> content;
};

// struct IfDirective {};

class Lexer {
    std::string m_filename;
    std::string m_text;
    std::vector<Token> m_tokens;
    std::vector<bool> m_cond_incs;
    int i;
    int line;
    bool m_saw_newline = true;

    // TODO intern
    std::unordered_map<std::string, Macro> m_macro_table;

    // std::vector<IfDirective> m_if_stack;

    // TODO find include paths at runtime, need to extend find_windows.h
    // functionality which is currently used to find the linker
    std::vector<std::string> m_include_paths = {
        "C:/Program Files/Microsoft Visual "
        "Studio/2022/Community/VC/Tools/MSVC/14.37.32822/include"

        "C:/Program Files/Microsoft Visual "
        "Studio/2022/Community/VC/Tools/MSVC/14.37.32822/atlmfc/include"

        "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/ucrt"
        "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/shared"
        "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/um"
        "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/winrt"
        "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/cppwinrt"};

public:
    Lexer();
    Lexer(std::string text);
    Lexer(InputFile);

    void report_lex_error(Token);

    Token internal_next_no_update(bool keep_newline);
    Token internal_next(bool keep_newline = false);

    void discard_until_newline();
    void collect_until_newline(std::vector<Token> &);
    void discard_rest_of_line();

    void ppc_stringize(std::vector<Token> &);

    std::vector<Token> ppc_subst(Macro, std::vector<std::vector<Token>>,
                                 HideSet);

    void add_tokens(std::vector<Token> &);

    Token ppc_expand_object_like(Macro);
    Token ppc_expand_function_like(Macro);

    Token ppc_expand();

    void continue_to_cond_inc();
    void ppc_internal_if(bool);
    void ppc_if();
    void ppc_elif();
    void ppc_else();
    void ppc_line();
    void ppc_ifdef();
    void ppc_endif();
    void ppc_undef();
    void ppc_error();
    void ppc_ifndef();
    void ppc_define();
    void ppc_pragma();
    void ppc_include();

    void ppc_directive();

    Token ppc_next();

    Token next();

    Token peek();

    void eat(TokenKind);
    void eat(char);
    void try_eat(TokenKind);
    void try_eat(char);
    void eat_next(TokenKind);
    void eat_next(char);

    bool on(TokenKind);
    bool on(char);
    Token curr();

    std::string lexer__debug_token_to_string(Token);
    void lexer__debug_dump();
};

static std::string str(TokenKind kind) {
    std::string result = "";

#define X(a, b, _)                                                             \
    case TokenKind::a:                                                         \
        result = #a;                                                           \
        break;

    switch (kind) {

#include "inc/token_kinds.inc"

    default:
        ice(false);
    }

#undef X

    return result;
}

static std::string str_rep(Token token) {
    std::string result = "";

    if (token.kind == TokenKind::_id) {
        return *token.id.val;
    } else if (token.kind == TokenKind::_num_lit) {
        return std::to_string(token.number.val);
    } else if (token.kind == TokenKind::_str_lit) {
        return *token.str.val;
    } else if ((int)token.kind >= (int)TokenKind::_newline) {
        return std::string(1, (char)(token.kind));
    }

#define X(a, b, c)                                                             \
    case TokenKind::a:                                                         \
        result = c;                                                            \
        break;

    switch (token.kind) {

#include "inc/token_kinds.inc"

    default:
        ice(false);
    }

#undef X

    return result;
}

static std::unordered_map<std::string, TokenKind> keywords = {
    {"char", TokenKind::k_char},
    {"double", TokenKind::k_double},
    {"float", TokenKind::k_float},
    {"int", TokenKind::k_int},
    {"short", TokenKind::k_short},
    {"long", TokenKind::k_long},
    {"signed", TokenKind::k_signed},
    {"unsigned", TokenKind::k_unsigned},
    {"void", TokenKind::k_void},
    {"enum", TokenKind::k_enum},
    {"struct", TokenKind::k_struct},
    {"union", TokenKind::k_union},
    {"auto", TokenKind::k_auto},
    {"const", TokenKind::k_const},
    {"extern", TokenKind::k_extern},
    {"inline", TokenKind::k_inline},
    {"register", TokenKind::k_register},
    {"restrict", TokenKind::k_restrict},
    {"static", TokenKind::k_static},
    {"volatile", TokenKind::k_volatile},
    {"break", TokenKind::k_break},
    {"case", TokenKind::k_case},
    {"continue", TokenKind::k_continue},
    {"default", TokenKind::k_default},
    {"do", TokenKind::k_do},
    {"else", TokenKind::k_else},
    {"for", TokenKind::k_for},
    {"goto", TokenKind::k_goto},
    {"if", TokenKind::k_if},
    {"return", TokenKind::k_return},
    {"switch", TokenKind::k_switch},
    {"while", TokenKind::k_while},
    {"typedef", TokenKind::k_typedef},
    {"sizeof", TokenKind::k_sizeof},
    {"_Alignas", TokenKind::k__Alignas},
    {"_Alignof", TokenKind::k__Alignof},
    {"_Atomic", TokenKind::k__Atomic},
    {"_Bool", TokenKind::k__Bool},
    {"_Complex", TokenKind::k__Complex},
    {"_Generic", TokenKind::k__Generic},
    {"_Imaginary", TokenKind::k__Imaginary},
    {"_Noreturn", TokenKind::k__Noreturn},
    {"_Static_assert", TokenKind::k__Static_assert},
    {"_Thread_local", TokenKind::k__Thread_local},
};

} // namespace jcc
