#include "lexer.h"

using namespace jcc;

Lexer::Lexer()
    : m_text{""}, m_current{TokenKind::NONE}, m_cache{TokenKind::NONE}, i{0} {}

Lexer::Lexer(std::string text)
    : m_text{text}, m_current{TokenKind::NONE}, m_cache{TokenKind::NONE}, i{0} {
}

void Lexer::eat(char c) {
    JCC_PROFILE()
    eat((TokenKind)c);
}

void Lexer::eat(TokenKind t) {
    JCC_PROFILE()
    assert(this->curr().kind == t);
    this->next();
}

void Lexer::try_eat(char c) {
    JCC_PROFILE()
    try_eat((TokenKind)c);
}

void Lexer::try_eat(TokenKind t) {
    JCC_PROFILE()
    if (this->curr().kind == t)
        this->next();
}

void Lexer::eat_next(char c) {
    JCC_PROFILE()
    this->next();
    assert((char)this->curr().kind == c);
    this->next();
}

void Lexer::eat_next(TokenKind t) {
    JCC_PROFILE()
    eat_next((char)t);
}

Token Lexer::peek() {
    JCC_PROFILE()
    if (m_cache.kind != TokenKind::NONE)
        return m_cache;
    m_cache = this->internal_next();
    return m_cache;
}

Token Lexer::next() {
    JCC_PROFILE()
    if (m_cache.kind != TokenKind::NONE) {
        m_current = m_cache;
        m_cache = Token{TokenKind::NONE};
        return m_current;
    }
    m_current = this->internal_next();
    return m_current;
}

// I'm being lazy for right now
// this is a very simple version
// TODO:
// * switch
// * intern
// * smarter keyword lookup, perfect hash/classic jump by length
// * use a string view instead of string
Token Lexer::internal_next() {
    JCC_PROFILE()
    Token result;
    std::string id = "";

    while (isspace(m_text[i])) {
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

        if (keywords.contains(id)) {
            result = Token(keywords[id]);
            return result;
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
            i += 2;
            return Token(TokenKind::_shift_left);
        }

        if (m_text[i] == '>' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_greater_than_equal);
        }
        if (m_text[i] == '>' && m_text[i + 1] == '>') {
            i += 2;
            return Token(TokenKind::_shift_right);
        }

        if (m_text[i] == '=' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_equal_equal);
        }
        if (m_text[i] == '!' && m_text[i + 1] == '=') {
            i += 2;
            return Token(TokenKind::_not_equal);
        }
    }

    result = Token((TokenKind)m_text[i++]);
    return result;
}

bool Lexer::on(TokenKind kind) {
    JCC_PROFILE()
    return m_current.kind == kind;
}

bool Lexer::on(char c) {
    JCC_PROFILE()
    return (char)m_current.kind == c;
}

Token Lexer::curr() {
    JCC_PROFILE()
    return m_current;
}
