#include "lexer.h"

#include <ranges>

using namespace jcc;

Lexer::Lexer() : m_text{""}, m_current{TokenKind::NONE}, m_cache{}, i{0} {}

Lexer::Lexer(std::string text)
    : m_text{text}, m_current{TokenKind::NONE}, m_cache{}, i{0} {}

// I'm being lazy for right now
// this is a very simple and inefficient lexer
// TODO:
// * switch
// * intern
// * smarter keyword lookup, perfect hash or classic jump by length
// * use a string view instead of string
Token Lexer::internal_next_no_update(bool keep_newline) {
    JCC_PROFILE();

    Token result;
    std::string id = "";

    while (isspace(m_text[i])) {
        if (keep_newline && m_text[i] == '\n')
            break;
        ++i;
    }

    if (i >= m_text.size()) {
        result = Token(TokenKind::_eof);
        return result;
    }

    if (isalpha(m_text[i]) || m_text[i] == '_') {
        while (isalnum(m_text[i]) || m_text[i] == '_') {
            id += m_text[i];
            ++i;
        }

        auto token = Token(TokenKind::_id);
        token.id.val = new std::string(id); // TODO intern
        result = token;
        return result;
    }

    if (isdigit(m_text[i]) || m_text[i] == '.') {
        do {
            id += m_text[i];
            ++i;
        } while (isdigit(m_text[i]) || m_text[i] == '.');

        double val = strtod(id.c_str(), nullptr);

        auto token = Token(TokenKind::_num_lit);
        token.number.val = val;
        result = token;
        return result;
    }

    // TODO escape sequences
    if (m_text[i] == '"') {
        ++i;
        while (m_text[i] != '"') {
            id += m_text[i];
            ++i;
        }
        ++i;

        auto token = Token(TokenKind::_str_lit);
        token.str.val = new std::string(id); // TODO intern
        result = token;
        return result;
    }

    // TODO/FIXME ppc can handle this
    if (m_text[i] == '/' && m_text[i + 1] == '/') {
        while (m_text[i] != EOF && m_text[i] != '\n' && m_text[i] != '\r')
            ++i;
        ++i;
        if (m_text[i] != EOF)
            return this->internal_next();
    }

    if (m_text[i] == EOF) {
        result = Token(TokenKind::_eof);
        return result;
    }

    // FIXME, lazy
    if (i + 1 < m_text.size()) {
        if (m_text[i] == '<' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_less_than_equal);
        }
        if (m_text[i] == '<' && m_text[i + 1] == '<') {
            if (i + 2 < m_text.size() && m_text[i + 2] == '=') {
                i += 3;
                return Token(TokenKind::_shift_left_equal);
            } else {
                i += 2;
                return Token(TokenKind::_shift_left);
            }
        }

        if (m_text[i] == '>' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_greater_than_equal);
        }
        if (m_text[i] == '>' && m_text[i + 1] == '>') {
            if (i + 2 < m_text.size() && m_text[i + 2] == '=') {
                i += 3;
                return Token(TokenKind::_shift_right_equal);
            } else {
                i += 2;
                return Token(TokenKind::_shift_right);
            }
        }

        if (m_text[i] == '=' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_equal_equal);
        }
        if (m_text[i] == '!' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_not_equal);
        }

        if (m_text[i] == '*' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_star_equal);
        }
        if (m_text[i] == '/' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_slash_equal);
        }
        if (m_text[i] == '%' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_percent_equal);
        }
        if (m_text[i] == '+' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_add_equal);
        }
        if (m_text[i] == '-' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_sub_equal);
        }
        if (m_text[i] == '&' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_and_equal);
        }
        if (m_text[i] == '^' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_xor_equal);
        }
        if (m_text[i] == '|' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_or_equal);
        }

        if (m_text[i] == '+' && m_text[i + 1] == '+') {
            i += 2;
            return Token(TokenKind::_inc);
        }
        if (m_text[i] == '-' && m_text[i + 1] == '-') {
            i += 2;
            return Token(TokenKind::_dec);
        }

        if (m_text[i] == '#' && m_text[i + 1] == '#') {
            i += 2;
            return Token(TokenKind::_hash_hash);
        }
    }

    result = Token((TokenKind)m_text[i++]);
    return result;
}

// FIXME this whole design is weird
Token Lexer::internal_next(bool keep_newline) {
    if (m_cache.empty())
        m_current = this->internal_next_no_update(keep_newline);
    else {
        m_current = m_cache.back();
        m_cache.pop_back();
    }
    return m_current;
}

void Lexer::add_tokens(std::vector<Token> &vec) {
    JCC_PROFILE();

    // FIXME perf
    for (Token tkn : vec | std::ranges::views::reverse)
        m_cache.push_back(tkn);
}

Token Lexer::ppc_expand_object_like(Macro macro) {
    JCC_PROFILE();

    add_tokens(macro.content);

    Token result = m_cache.back();
    m_cache.pop_back();
    return result;
}

Token Lexer::ppc_expand_function_like(Macro macro) {
    return {};
}

// https://www.spinellis.gr/blog/20060626/cpp.algo.pdf
Token Lexer::ppc_expand() {
    JCC_PROFILE();

    Token tkn = m_current;
    std::string *id = m_current.id.val;

    if (m_current.hide_set.contains(id)) {
        return m_current;
    }

    if (m_macro_table.contains(*id)) {
        // FIXME oops double lookup
        Macro macro = m_macro_table[*id];

        switch (macro.kind) {
        case MacroKind::none:
            ice(false);
        case MacroKind::object_like:
            m_current.hide_set.insert(id);
            return ppc_expand_object_like(macro);
        case MacroKind::function_like:
            return ppc_expand_function_like(macro);
        default:
            ice(false);
        }
    }

    return m_current;
}

void Lexer::ppc_if() {
    JCC_PROFILE();
    ice(false);
}

void Lexer::ppc_elif() {
    JCC_PROFILE();
    ice(false);
}

void Lexer::ppc_else() {
    JCC_PROFILE();
    ice(false);
}

void Lexer::ppc_line() {
    JCC_PROFILE();
    ice(false);
}

void Lexer::ppc_ifdef() {
    JCC_PROFILE();
    ice(false);
}

void Lexer::ppc_endif() {
    JCC_PROFILE();
    ice(false);
}

void Lexer::ppc_undef() {
    JCC_PROFILE();
    ice(false);
}

void Lexer::ppc_error() {
    JCC_PROFILE();
    ice(false);
}

void Lexer::ppc_ifndef() {
    JCC_PROFILE();
    ice(false);
}

void Lexer::ppc_define() {
    JCC_PROFILE();

    Token tkn = internal_next();
    if (tkn.kind != TokenKind::_id)
        ice(false);

    std::string *id = tkn.id.val;
    Macro macro;
    macro.kind = MacroKind::object_like;
    macro.content = {};

    tkn = internal_next(true);
    while (tkn.kind != TokenKind::_newline) {
        macro.content.push_back(tkn);
        tkn = internal_next(true);
    }
    internal_next();

    m_macro_table[*id] = macro;
}

void Lexer::ppc_pragma() {
    JCC_PROFILE();
    ice(false);
}

void Lexer::ppc_include() {
    JCC_PROFILE();
    ice(false);
}

void Lexer::ppc_directive() {
    JCC_PROFILE();

    Token tkn = this->internal_next();
    if (tkn.kind != TokenKind::_id)
        ice(false);

    std::string id = *tkn.id.val;
    switch (tkn.id.val->size()) {
    case 2:
        if (id == "if")
            ppc_if();
        break;
    case 4:
        if (id == "elif")
            ppc_elif();
        else if (id == "else")
            ppc_else();
        else if (id == "line")
            ppc_line();
        break;
    case 5:
        if (id == "ifdef")
            ppc_ifdef();
        else if (id == "endif")
            ppc_endif();
        else if (id == "undef")
            ppc_undef();
        else if (id == "error")
            ppc_error();
        break;
    case 6:
        if (id == "ifndef")
            ppc_ifndef();
        else if (id == "define")
            ppc_define();
        else if (id == "pragma")
            ppc_pragma();
        break;
    case 7:
        if (id == "include")
            ppc_include();
        break;
    default:
        ice(false);
    }
}

Token Lexer::ppc_next() {
    JCC_PROFILE();

    Token result = internal_next();

    while (true) {
        if (result.kind == TokenKind::_id)
            result = ppc_expand();
        else
            result = m_current;
        if (result.kind == TokenKind::_hash)
            ppc_directive();
        else
            break;
    }

    if (result.kind == TokenKind::_id)
        result = ppc_expand();

    if (result.kind == TokenKind::_id && keywords.contains(*result.id.val)) {
        result = Token(keywords[*result.id.val]);
        return result;
    }

    return result;
}

Token Lexer::next() {
    JCC_PROFILE();

    // if (m_cache.kind != TokenKind::NONE) {
    //     m_current = m_cache;
    //     m_cache = Token{TokenKind::NONE};
    //     return m_current;
    // }
    m_current = this->ppc_next();
    return m_current;
}

Token Lexer::peek() {
    JCC_PROFILE();

    // if (m_cache.kind != TokenKind::NONE)
    //     return m_cache;
    auto saved_current = m_current;
    auto saved_cache = m_cache;
    auto saved_i = i;
    auto res = this->ppc_next();
    m_current = saved_current;
    m_cache = saved_cache;
    i = saved_i;
    return res;
}

void Lexer::eat(TokenKind t) {
    JCC_PROFILE();

    ice(this->curr().kind == t);
    this->next();
}

void Lexer::eat(char c) {
    JCC_PROFILE();

    eat((TokenKind)c);
}

void Lexer::try_eat(TokenKind t) {
    JCC_PROFILE();

    if (this->curr().kind == t)
        this->next();
}

void Lexer::try_eat(char c) {
    JCC_PROFILE();

    try_eat((TokenKind)c);
}

void Lexer::eat_next(TokenKind t) {
    JCC_PROFILE();

    eat_next((char)t);
}

void Lexer::eat_next(char c) {
    JCC_PROFILE();

    this->next();
    ice((char)this->curr().kind == c);
    this->next();
}

bool Lexer::on(TokenKind kind) {
    JCC_PROFILE();

    return m_current.kind == kind;
}

bool Lexer::on(char c) {
    JCC_PROFILE();

    return (char)m_current.kind == c;
}

Token Lexer::curr() {
    JCC_PROFILE();

    return m_current;
}
