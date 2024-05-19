#pragma once

#include "common.h"
#include <unordered_map>

namespace jcc {

enum TokenKind : char {
    _sentinel_end = 127,

    _printable_end = '~',
    _tilde = '~',
    _close_curly = '}',
    _or = '|',
    _open_curly = '{',
    _lowercase_end = 'z',
    // ...
    _lowercase_start = 'a',
    _backtick = '`',
    _underscore = '_',
    _xor = '^',
    _close_bracket = ']',
    _backslash = '\\',
    _open_bracket = '[',
    _uppercase_end = 'Z',
    // ...
    _uppercase_begin = 'A',
    _at = '@',
    _question_mark = '?',
    _greater_than = '>',
    _equal = '=',
    _less_than = '<',
    _semicolon = ';',
    _colon = ':',
    _number_end = '9',
    // ...
    _number_begin = '0',
    _slash = '/',
    _dot = '.',
    _sub = '-',
    _comma = ',',
    _add = '+',
    _star = '*',
    _close_paren = ')',
    _open_paren = '(',
    _single_quote = '\'',
    _and = '&',
    _percent = '%',
    _dollar = '$',
    _hash = '#',
    _double_quote = '"',
    _bang = '!',
    _space = ' ',
    _printable_start = ' ',
    UNUSED_31 = 31,
    UNUSED_1 = 1,
    NONE = 0,
    _eof = -1,

    _keyword_end = -2,
    k_char = -2,
    k_double = -3,
    k_float = -4,
    k_int = -5,
    k_short = -6,
    k_long = -7,
    k_signed = -8,
    k_unsigned = -9,

    k_void = -10,
    k_enum = -11,
    k_struct = -12,
    k_union = -13,

    k_auto = -14,
    k_const = -15,
    k_extern = -16,
    k_inline = -17,
    k_register = -18,
    k_restrict = -19,
    k_static = -20,
    k_volatile = -21,

    k_break = -22,
    k_case = -23,
    k_continue = -24,
    k_default = -25,
    k_do = -26,
    k_else = -27,
    k_for = -28,
    k_goto = -29,
    k_if = -30,
    k_return = -31,
    k_switch = -32,
    k_while = -33,

    k_typedef = -34,
    k_sizeof = -35,

    k__Alignas = -36,
    k__Alignof = -37,
    k__Atomic = -38,
    k__Bool = -39,
    k__Complex = -40,
    k__Generic = -41,
    k__Imaginary = -42,
    k__Noreturn = -43,
    k__Static_assert = -44,
    k__Thread_local = -45,
    _keyword_start = -46,

    _id = -47,
    _num_lit = -48,
    _str_lit = -49,

    _log_or = -50,
    _log_and = -51,
    _equal_equal = -52,
    _not_equal = -53,
    _less_than_equal = -54,
    _greater_than_equal = -55,
    _shift_left = -56,
    _shift_right = -57,
    _inc = -58,
    _dec = -59,
    _arrow = -60,

    _sentinel_start = -128,
};

struct Token {
    TokenKind kind;

    union {
        struct {
            std::string *val; // TODO intern
        } id;
        struct {
            int val; // TODO float/int
        } number;
        struct {
            std::string *val;
        } str;
    };
};

class Lexer {
    std::string m_text;
    Token m_current;
    Token m_cache;
    int i;

public:
    Lexer();
    Lexer(std::string text);

    void eat(TokenKind);
    void eat(char);
    void try_eat(TokenKind);
    void try_eat(char);
    void eat_next(TokenKind);
    void eat_next(char);

    Token peek();

    Token next();
    Token internal_next();

    bool on(TokenKind);
    bool on(char);
    Token curr();
};

// TODO fix this
static std::string str(Token t) {
    std::string str = "";

    if (t.kind >= TokenKind::_printable_start &&
        t.kind < TokenKind::_printable_end)
        return std::string(1, (char)t.kind);

    switch (t.kind) {
    case TokenKind::_eof:
        return "EOF";
    case TokenKind::k_extern:
        return "extern";
    case TokenKind::_id:
        return *t.id.val;
    case TokenKind::_num_lit:
        return std::to_string(t.number.val);
    default:
        assert(false);
    }

    return str;
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
