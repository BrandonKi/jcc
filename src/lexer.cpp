#include "lexer.h"

#include <parser.h>

#include <ranges>
#include <algorithm>


using namespace jcc;

static bool PPC_DEBUG = false;

#define PPC_DEBUG_PRINT(x)                                                                                             \
    if (PPC_DEBUG)                                                                                                     \
    std::cout << std::string(__FUNCTION__).substr(12) << ", " << x << "\n "

Lexer::Lexer() : m_filename{"<string>"}, m_text{""}, m_tokens{}, m_cond_incs{}, i{0}, line{1}, m_macro_table{} {
    m_tokens.emplace_back(TokenKind::NONE);
}

Lexer::Lexer(std::string text)
    : m_filename{"<string>"}, m_text{text}, m_tokens{}, m_cond_incs{}, i{0}, line{1}, m_macro_table{} {
    m_tokens.emplace_back(TokenKind::NONE);
}

Lexer::Lexer(InputFile file)
    : m_filename{file.filepath}, m_text{file.text}, m_tokens{}, m_cond_incs{}, i{0}, line{1}, m_macro_table{} {
    m_tokens.emplace_back(TokenKind::NONE);
}

// TODO rewrite, this is temporary
// it also doesn't work 100% correct
void Lexer::report_lex_error(Token tkn) {
    int col = 0;
    for (int j = 0; j < i; ++j, ++col)
        if (m_text[j] == '\n')
            col = 1;

    std::string loc_string = m_filename + ":" + std::to_string(line) + ":" + std::to_string(col);

    println(loc_string + " " + fmt("error: ", RED) + "unexpected token " + str(tkn.kind));

    std::exit(-1);
}

// I'm being lazy for right now
// this is a very simple and inefficient lexer
// TODO:
// * switch
// * intern
// * smarter keyword lookup, perfect hash or classic jump by length
// * use a string view instead of string
Token Lexer::internal_next_no_update(bool keep_newline) {
    JCC_PROFILE();

    Token result = {};
    result.start_of_line = m_saw_newline;
    m_saw_newline = false;

    std::string id = "";

    while (isspace(m_text[i])) {
        if (m_text[i] == '\n') {
            m_saw_newline = true;
            line += 1;
            if (keep_newline)
                break;
            else
                result.start_of_line = true;
        }
        ++i;
    }

    if (i >= m_text.size()) {
        result.kind = TokenKind::_eof;
        return result;
    }

    if (isalpha(m_text[i]) || m_text[i] == '_') {
        while (isalnum(m_text[i]) || m_text[i] == '_') {
            id += m_text[i];
            ++i;
        }

        result.kind = TokenKind::_id;
        result.id.val = new std::string(id); // TODO intern
        return result;
    }

    if (isdigit(m_text[i])) {
        do {
            id += m_text[i];
            ++i;
        } while (isdigit(m_text[i]) || m_text[i] == '.');

        double val = strtod(id.c_str(), nullptr);

        result.kind = TokenKind::_num_lit;
        result.number.val = val;
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

        result.kind = TokenKind::_str_lit;
        result.str.val = new std::string(id); // TODO intern
        return result;
    }

    // FIXME these characters can appear in strings
    if (m_text[i] == '/' && m_text[i + 1] == '/') {
        while (m_text[i] != EOF && m_text[i] != '\n' && m_text[i] != '\r')
            ++i;
        ++i;
        if (m_text[i] != EOF) {
            m_saw_newline = true;
            line += 1;
            return this->internal_next_no_update(keep_newline);
        }
    }
    if (m_text[i] == '/' && m_text[i + 1] == '*') {
        while (m_text[i] != EOF && (m_text[i] != '*' || m_text[i+1] != '/')) {
            ++i;
            if(m_text[i] == '\n')
                ++line;
        }
        ++i;
        ++i;
        // ++i;
        // ++i;
        if (m_text[i] != EOF) {
            // m_saw_newline = true;
            // line += 1;
            return this->internal_next_no_update(keep_newline);
        }
    }

    if (m_text[i] == EOF) {
        result.kind = TokenKind::_eof;
        return result;
    }

    // FIXME, lazy
    if (i + 1 < m_text.size()) {
        if (m_text[i] == '&' && m_text[i + 1] == '&') {
            i += 2;
            result.kind = TokenKind::_log_and;
            return result;
        }
        if (m_text[i] == '|' && m_text[i + 1] == '|') {
            i += 2;
            result.kind = TokenKind::_log_or;
            return result;
        }


        if (m_text[i] == '<' && m_text[i + 1] == '=') {
            i += 2;
            result.kind = TokenKind::_less_than_equal;
            return result;
        }
        if (m_text[i] == '<' && m_text[i + 1] == '<') {
            if (i + 2 < m_text.size() && m_text[i + 2] == '=') {
                i += 3;
                result.kind = TokenKind::_shift_left_equal;
                return result;
            } else {
                i += 2;
                result.kind = TokenKind::_shift_left;
                return result;
            }
        }

        if (m_text[i] == '>' && m_text[i + 1] == '=') {
            i += 2;
            result.kind = TokenKind::_greater_than_equal;
            return result;
        }
        if (m_text[i] == '>' && m_text[i + 1] == '>') {
            if (i + 2 < m_text.size() && m_text[i + 2] == '=') {
                i += 3;
                result.kind = TokenKind::_shift_right_equal;
                return result;
            } else {
                i += 2;
                result.kind = TokenKind::_shift_right;
                return result;
            }
        }

        if (m_text[i] == '=' && m_text[i + 1] == '=') {
            i += 2;
            result.kind = TokenKind::_equal_equal;
            return result;
        }
        if (m_text[i] == '!' && m_text[i + 1] == '=') {
            i += 2;
            result.kind = TokenKind::_not_equal;
            return result;
        }

        if (m_text[i] == '*' && m_text[i + 1] == '=') {
            i += 2;
            result.kind = TokenKind::_star_equal;
            return result;
        }
        if (m_text[i] == '/' && m_text[i + 1] == '=') {
            i += 2;
            result.kind = TokenKind::_slash_equal;
            return result;
        }
        if (m_text[i] == '%' && m_text[i + 1] == '=') {
            i += 2;
            result.kind = TokenKind::_percent_equal;
            return result;
        }
        if (m_text[i] == '+' && m_text[i + 1] == '=') {
            i += 2;
            result.kind = TokenKind::_add_equal;
            return result;
        }
        if (m_text[i] == '-' && m_text[i + 1] == '=') {
            i += 2;
            result.kind = TokenKind::_sub_equal;
            return result;
        }
        if (m_text[i] == '&' && m_text[i + 1] == '=') {
            i += 2;
            result.kind = TokenKind::_and_equal;
            return result;
        }
        if (m_text[i] == '^' && m_text[i + 1] == '=') {
            i += 2;
            result.kind = TokenKind::_xor_equal;
            return result;
        }
        if (m_text[i] == '|' && m_text[i + 1] == '=') {
            i += 2;
            result.kind = TokenKind::_or_equal;
            return result;
        }

        if (m_text[i] == '+' && m_text[i + 1] == '+') {
            i += 2;
            result.kind = TokenKind::_inc;
            return result;
        }
        if (m_text[i] == '-' && m_text[i + 1] == '-') {
            i += 2;
            result.kind = TokenKind::_dec;
            return result;
        }

        if (m_text[i] == '#' && m_text[i + 1] == '#') {
            i += 2;
            result.kind = TokenKind::_hash_hash;
            return result;
        }
    }

    result.kind = (TokenKind)m_text[i++];
    return result;
}

// FIXME this whole design is weird
Token Lexer::internal_next(bool keep_newline) {
    JCC_PROFILE();

    m_tokens.pop_back();

    if (m_tokens.empty())
        m_tokens.push_back(this->internal_next_no_update(keep_newline));

    if(m_tokens.back().kind == TokenKind::_eof && !stash_stack.empty()) {
        m_tokens.pop_back();
        unstash();
        return internal_next();
    }

    return curr();
}

void Lexer::discard_until_newline() {
    JCC_PROFILE();

    while (curr().kind != TokenKind::_newline) {
        internal_next(true);
    }
}

void Lexer::collect_until_newline(std::vector<Token> &out) {
    JCC_PROFILE();

    Token tkn = curr();
    while (tkn.kind != TokenKind::_newline) {
        out.push_back(tkn);
        tkn = internal_next(true);
    }
}

void Lexer::discard_rest_of_line() {
    JCC_PROFILE();

    while (curr().kind != TokenKind::_newline) {
        internal_next(true);
    }
    internal_next(false);
}

void Lexer::ppc_stringize(std::vector<Token> &tokens) {
    JCC_PROFILE();
}

// https://www.spinellis.gr/blog/20060626/cpp.algo.pdf
/* substitute args, handle stringize and paste */
// TODO separate into versions that do/don't need params
std::vector<Token> Lexer::ppc_subst(Macro macro, std::vector<std::vector<Token>> params, HideSet hide_set) {
    std::vector<Token> tokens = {};

    Token tkn;
    for (int i = 0; i < macro.content.size(); ++i) {
        tkn = macro.content[i];
        if (tkn.kind == TokenKind::_id && macro.params.contains(*tkn.id.val)) { // FIXME double lookup
            tokens.append_range(params[macro.params[*tkn.id.val]]);
        } else {
            tokens.push_back(tkn);
        }
    }

    for (auto &t : tokens)
        for (auto &h : hide_set)
            t.hide_set.insert(h);

    return tokens;
}

void Lexer::add_tokens(std::vector<Token> &vec) {
    JCC_PROFILE();

    PPC_DEBUG_PRINT(vec.size());

    // FIXME perf
    for (Token tkn : vec | std::ranges::views::reverse)
        m_tokens.push_back(tkn);
}

Token Lexer::ppc_expand_object_like(Macro macro) {
    JCC_PROFILE();

    Strand id = curr().id.val;
    PPC_DEBUG_PRINT(id.value() + " " + std::to_string(macro.content.size()));

    Token tkn = internal_next();
    HideSet new_hide_set = curr().hide_set;
    new_hide_set.insert(id.value());
    std::vector<Token> tokens = ppc_subst(macro, {}, new_hide_set);

    add_tokens(tokens);
    return curr();
}

Token Lexer::ppc_expand_function_like(Macro macro) {
    JCC_PROFILE();

    Strand id = curr().id.val;
    PPC_DEBUG_PRINT(id.value() + "(...) " + std::to_string(macro.content.size()));

    internal_next();
    std::vector<std::vector<Token>> params = {};

    if (curr().kind != TokenKind::_open_paren)
        report_lex_error(curr());

    Token tkn = curr();
    int parens = 1;
    int i = 0;
    while (tkn.kind != TokenKind::_close_paren) {
        params.push_back({});
        tkn = internal_next();
        while (tkn.kind != TokenKind::_comma) {
            if (tkn.kind == TokenKind::_open_paren)
                parens += 1;
            else if (tkn.kind == TokenKind::_close_paren)
                parens -= 1;

            if (parens > 0)
                params.back().push_back(tkn);
            else if (parens < 0)
                report_lex_error(curr()); // TODO report better error
            else
                break;
            tkn = internal_next();
        }
    }
    Token close_paren = curr();
    internal_next();
    auto view = tkn.hide_set | std::views::filter([&](auto e) { return close_paren.hide_set.contains(e); });
    HideSet new_hide_set(view.begin(), view.end());
    new_hide_set.insert(id.value());
    std::vector<Token> tokens = ppc_subst(macro, params, new_hide_set);

    add_tokens(tokens);
    return curr();
}

// https://www.spinellis.gr/blog/20060626/cpp.algo.pdf
Token Lexer::ppc_expand() {
    JCC_PROFILE();

    if (curr().kind != _id)
        return curr();

    PPC_DEBUG_PRINT(*curr().id.val);

    Token tkn = curr();
    Strand id = tkn.id.val;

    if (tkn.hide_set.contains(id.value())) {
        return tkn;
    }

    if (m_macro_table.contains(id.value())) {
        // FIXME oops double lookup
        Macro macro = m_macro_table[id.value()];

        switch (macro.kind) {
        case MacroKind::none:
            ice(false);
        case MacroKind::object_like: {
            ppc_expand_object_like(macro);
            return ppc_expand();
        }
        case MacroKind::function_like: {
            return ppc_expand_function_like(macro);
        }
        default:
            ice(false);
        }
    }

    return tkn;
}

void Lexer::continue_to_cond_inc() {
    JCC_PROFILE();
    PPC_DEBUG_PRINT("");

    ice(!m_cond_incs.empty());

    bool cond = m_cond_incs.back();
    if (!cond) {
        int nested_ifs = 0;
        bool ignore = false;
        while (true) {
            switch (m_text[i]) {
            case '\n':
                ++line;
                m_saw_newline = true;
                ++i;
                continue;
            case '#':  // TODO handle text in single/multi line comments
                if (m_saw_newline && !ignore) {
                    ++i;
                    internal_next();
                    if(*curr().id.val == "endif") {
                        if(nested_ifs == 0) {
                            internal_next();
                            return;
                        }
                        else
                            ++nested_ifs;
                    } else if(*curr().id.val == "ifdef" || *curr().id.val == "ifndef" || *curr().id.val == "if") {
                        ++nested_ifs;
                    } else if(*curr().id.val == "else") {
                        if(nested_ifs > 0) {
                            --nested_ifs;
                            break;
                        }
                        internal_next();
                        return;
                    }
                }
                break;
            case '"':
                ++i;
                while (m_text[i] != '"') {
                    ++i;
                }
                ++i;
                break;
            }
            m_saw_newline = false;
            ++i;
        }
    }
}

std::string Lexer::collect_char_until(char c) {
    JCC_PROFILE();

    int start = i;
    while (m_text[i] != c) {
        ++i;
    }

    return m_text.substr(start, i-start);
}

void Lexer::ppc_internal_if(bool cond) {
    JCC_PROFILE();
    PPC_DEBUG_PRINT(std::to_string(cond));

    m_cond_incs.push_back(cond);

    if(cond)
        discard_rest_of_line();
    else
        discard_until_newline();

    continue_to_cond_inc();
}

int Lexer::ppc_const_expr() {
    JCC_PROFILE();
    PPC_DEBUG_PRINT("");

    std::vector<Token> tokens;

    Token tkn = internal_next(true);
    while(tkn.kind != TokenKind::_newline) {
        if(tkn.kind == TokenKind::_id) {
            // FIXME intern
            Token numlit{TokenKind::_num_lit, false, {}};
            if(*tkn.id.val == "defined") {
                tkn = internal_next(true);
                bool parens = tkn.kind == TokenKind::_open_paren;
                if(parens) tkn = internal_next(true);
                ice(tkn.kind == TokenKind::_id);
                bool cond = m_macro_table.contains(*tkn.id.val);
                numlit.number.val = cond ? 1 : 0;
                if(parens) {
                    tkn = internal_next(true);
                    ice(tkn.kind == TokenKind::_close_paren);
                }
            } else {
                numlit.number.val = 0;
            }
            tokens.push_back(numlit);
        } else {
            tokens.push_back(tkn);
        }
        tkn = internal_next(true);
    }
    tokens.push_back(Token{TokenKind::_eof});
    add_tokens(tokens);
    auto *expr = m_parser->parse_expr();
    return eval_const_expr(expr);
}

void Lexer::ppc_if() {
    JCC_PROFILE();
    PPC_DEBUG_PRINT("");
    
    bool cond = static_cast<bool>(ppc_const_expr());
    ppc_internal_if(cond);
}

void Lexer::ppc_elif() {
    JCC_PROFILE();
    PPC_DEBUG_PRINT("");
    ice(false);
}

void Lexer::ppc_else() {
    JCC_PROFILE();
    PPC_DEBUG_PRINT("");

    ice(!m_cond_incs.empty());

    m_cond_incs.back() = !m_cond_incs.back();

    internal_next();
    continue_to_cond_inc();
}

void Lexer::ppc_line() {
    JCC_PROFILE();
    PPC_DEBUG_PRINT("");

    discard_until_newline();
    internal_next();
}

void Lexer::ppc_ifdef() {
    JCC_PROFILE();

    Token tkn = internal_next();
    if (tkn.kind != TokenKind::_id)
        ice(false);

    Strand id = tkn.id.val;
    bool cond = m_macro_table.contains(id);

    PPC_DEBUG_PRINT(id.value() + " " + std::to_string(cond));

    ppc_internal_if(cond);
}

void Lexer::ppc_endif() {
    JCC_PROFILE();
    PPC_DEBUG_PRINT("");

    ice(!m_cond_incs.empty());

    m_cond_incs.pop_back();
    internal_next();
}

void Lexer::ppc_undef() {
    JCC_PROFILE();

    internal_next();
    if (curr().kind != TokenKind::_id)
        ice(false);

    Strand id = curr().id.val;

    PPC_DEBUG_PRINT(id);

    m_macro_table.erase(id.value());
    internal_next();
}

void Lexer::ppc_error() {
    JCC_PROFILE();
    PPC_DEBUG_PRINT("");

    // FIXME change this when better error reporting gets added
    report_lex_error(curr());

    ice(false);
}

void Lexer::ppc_ifndef() {
    JCC_PROFILE();

    Token tkn = internal_next();
    if (tkn.kind != TokenKind::_id)
        ice(false);

    Strand id = tkn.id.val;
    bool cond = !m_macro_table.contains(id.value());

    PPC_DEBUG_PRINT(id.value() + " " + std::to_string(cond));

    ppc_internal_if(cond);
}

void Lexer::ppc_define() {
    JCC_PROFILE();

    Token tkn = internal_next();
    if (tkn.kind != TokenKind::_id)
        ice(false);

    Strand id = tkn.id.val;
    Macro macro;
    macro.kind = MacroKind::object_like;
    macro.content = {};

    tkn = internal_next(true);
    if (tkn.kind == TokenKind::_open_paren) {
        macro.kind = MacroKind::function_like;
        macro.params = {};

        tkn = internal_next();
        int i = 0;
        while (tkn.kind != TokenKind::_close_paren) {
            macro.params[*tkn.id.val] = i;
            tkn = internal_next();
            i += 1;
            if (tkn.kind == TokenKind::_comma)
                tkn = internal_next();
        }
        internal_next();
    }

    std::string debug_str = id.value() + (macro.kind == MacroKind::object_like ? "" : "(...)");
    PPC_DEBUG_PRINT(debug_str);

    collect_until_newline(macro.content);
    internal_next();

    m_macro_table[id.value()] = macro;
}

void Lexer::ppc_pragma() {
    JCC_PROFILE();
    PPC_DEBUG_PRINT("");

    discard_until_newline();
    internal_next();
}

void Lexer::ppc_include() {
    JCC_PROFILE();
    PPC_DEBUG_PRINT("");

    Token tkn = internal_next();
    if (tkn.kind != TokenKind::_less_than && tkn.kind != TokenKind::_double_quote)
        ice(false);

    if(tkn.kind == TokenKind::_less_than) {
        std::string name = collect_char_until('>');
        ++i;
        std::string content = get_sys_include(name);
        stash();
        m_filename = name;
        m_text = content;
        i = 0;
        m_saw_newline = true;
        line = 1;
        m_tokens = {}; // HACK
        m_tokens.emplace_back(TokenKind::NONE);
        m_cond_incs = {}; // FIXME
    }

    // discard_until_newline();
    internal_next();
}

void Lexer::ppc_directive() {
    JCC_PROFILE();

    PPC_DEBUG_PRINT("");

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

// TODO clean up this function, it's a mess
Token Lexer::ppc_next() {
    JCC_PROFILE();

    Token result = internal_next();

    PPC_DEBUG_PRINT(lexer__debug_token_to_string(result));

    bool expanded = false;
    while (true) {
        expanded = false;
        if (result.kind == TokenKind::_id) {
            result = ppc_expand();
            expanded = true;
        } else
            result = curr();

        if (result.kind == TokenKind::_hash && result.start_of_line)
            ppc_directive();
        else
            break;
    }

    if (!expanded && result.kind == TokenKind::_id)
        result = ppc_expand();

    if (result.kind == TokenKind::_id && keywords.contains(*result.id.val)) {
        result = Token(keywords[*result.id.val]);
        m_tokens.back() = result;
        return result;
    }

    return result;
}

Token Lexer::next() {
    JCC_PROFILE();

    this->ppc_next();
    return curr();
}

Token Lexer::peek() {
    JCC_PROFILE();

    // TODO perf
    auto saved_tokens = m_tokens;
    auto saved_i = i;
    auto saved_line = line;
    auto cond_incs = m_cond_incs;
    auto res = this->ppc_next();
    m_tokens = saved_tokens;
    i = saved_i;
    line = saved_line;
    m_cond_incs = cond_incs;
    return res;
}

void Lexer::eat(TokenKind t) {
    JCC_PROFILE();

    if (this->curr().kind != t)
        report_lex_error(curr());

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
    if ((char)this->curr().kind != c)
        report_lex_error(curr());
    this->next();
}

bool Lexer::on(TokenKind kind) {
    JCC_PROFILE();

    return curr().kind == kind;
}

bool Lexer::on(char c) {
    JCC_PROFILE();

    return (char)curr().kind == c;
}

Token Lexer::curr() {
    JCC_PROFILE();
    if (!m_tokens.empty())
        return m_tokens.back();
    return Token(TokenKind::_eof);
}

void Lexer::unstash() {
    JCC_PROFILE();
    StashState state = stash_stack.back();
    stash_stack.pop_back();

    m_filename = state.filename;
    m_text = state.text;
    m_tokens = state.tokens;
    m_cond_incs = state.cond_incs; // FIXME
    i = state.i;
    line = state.line;
    m_saw_newline = state.saw_newline;
}

void Lexer::stash() {
    JCC_PROFILE();
    StashState state;
    state.filename = m_filename;
    state.text = m_text;
    state.tokens = m_tokens;
    state.cond_incs = m_cond_incs; // FIXME
    state.i = i;
    state.line = line;
    state.saw_newline = m_saw_newline;

    stash_stack.push_back(state);
}

std::string Lexer::lexer__debug_token_to_string(Token token) {
    std::string extra = "";
    if (token.kind == TokenKind::_id)
        extra = ", " + *curr().id.val;
    else if (token.kind == TokenKind::_num_lit)
        extra = ",  " + std::to_string(token.number.val);
    return str(token.kind) + extra;
}

void Lexer::lexer__debug_dump() {
    PPC_DEBUG = true;

    std::string result = "";
    next();
    while (curr().kind != TokenKind::_eof) {
        result += str_rep(curr()) + " ";
        std::cout << line << "," << i << ":\t" << lexer__debug_token_to_string(curr()) << "\n";
        next();
    }

    std::cout << "\n" << result << "\n";
    std::exit(0);
}
