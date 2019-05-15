#include "lex.h"
#include <string>
#include <assert.h>
#include <map>
#include <algorithm>

#define EOI (-1)

struct Token : public IToken {
    Token();
    Token(TokenType tp, const std::string& word) : type_(tp), word_(word) {}
    virtual TokenType type() const override { return type_; }
    virtual const char *word() const override { return word_.c_str(); }
    TokenType type_;
    std::string word_;
};

struct Lex : public ILex {
    Lex(const char *sql);
    virtual IToken *token() override { return &cur_tk_; }
    virtual void next() override { scanf(); }
    virtual unsigned int cur_pos_line() const override { return line_; }
    virtual unsigned int cur_pos_col() const override { return col_; }
    virtual unsigned int cur_pos() const override { return pos_; }



    void scanf();
    void white();
    void comment();
    void single_line_comment();
    void multi_line_comment();

    void scanf_operator() {
        switch (char_at(pos())) {
            case '^': { cur_tk_ = Token(CARET, "^"); pos_inc(1); } break;
            case '%': { cur_tk_ = Token(PERCENT, "%"); pos_inc(1); } break;
            case '+': { cur_tk_ = Token(PLUS, "+"); pos_inc(1); } break;
            case '-': { cur_tk_ = Token(MINUS, "-"); pos_inc(1); } break;
            case '*': { cur_tk_ = Token(STAR, "*"); pos_inc(1); } break;
            case '/': { cur_tk_ = Token(DIVIDE, "/"); pos_inc(1); } break;
            case '.': { cur_tk_ = Token(DOT, "."); pos_inc(1); } break;
            case '(': { cur_tk_ = Token(LPAREN, "("); pos_inc(1); } break;
            case ')': { cur_tk_ = Token(RPAREN, ")"); pos_inc(1); } break;
            case ',': { cur_tk_ = Token(COMMA, ","); pos_inc(1); } break;
            case ';': { cur_tk_ = Token(SEMI, ";"); pos_inc(1); } break;
            case '?': { cur_tk_ = Token(QUES, "?"); pos_inc(1); } break;
            case '=': { cur_tk_ = Token(EQ, "="); pos_inc(1); } break;
            case '>': {
                pos_inc(1);
                if (char_at(pos()) == '=') {
                    cur_tk_ = Token(GTEQ, ">=");
                    pos_inc(1);
                }
                else {
                    cur_tk_ = Token(GT, ">");
                }

            } break;
            case '<': {
                pos_inc(1);
                if (char_at(pos()) == '=') {
                    cur_tk_ = Token(LTEQ, "<=");
                    pos_inc(1);
                }
                else if (char_at(pos()) == '>') {
                    cur_tk_ = Token(LTGT, "<>");
                    pos_inc(1);
                }
                else {
                    cur_tk_ = Token(LT, "<");
                }
            } break;
            case '|': {
                pos_inc(1);
                if (char_at(pos()) == '|') {
                    cur_tk_ = Token(BARBAR, "||");
                    pos_inc(1);
                }
                else {
                    cur_tk_ = Token(ERR, "EXPECTED '||'");
                }
            } break;
            default: {
                std::string err_s = "UNEXPECTED ";
                cur_tk_ = Token(ERR, err_s + char_at(pos()));
            } break;
        }
    }

    void scanf_str_literal() {
        unsigned int start = pos();
        assert(char_at(pos()) == '\'');
        pos_inc(1);
        char c = char_at(pos());
        while (c != EOI) {
            if (c == '\'')
                break;
            c = char_at(pos_inc(1));
        }
        if (c == '\'') {
            pos_inc(1);
            cur_tk_ = Token(STR_LITERAL, sub(start, pos()));
        }
        else
            cur_tk_ = Token(ERR, "UNTERMINATED STRING LITERAL");
    }

    void scanf_identifier() {
        unsigned int start = pos();
        char c = char_at(pos());
        switch (c) {
            case '"': {
                pos_inc(1);
                c = char_at(pos());
                while (c != EOI) {
                    if (c == '"')
                        break;
                    c = char_at(pos_inc(1));
                }
                if (c == '"') {
                    pos_inc(1);
                    cur_tk_ = Token(ID, sub(start, pos()));
                }
                else
                    cur_tk_ = Token(ERR, "IDENTIFIER WITH UNTERMINATED \"");
            } break;
            case '[': {
                pos_inc(1);
                c = char_at(pos());
                while (c != EOI) {
                    if (c == ']')
                        break;
                    c = char_at(pos_inc(1));
                }
                if (c == ']') {
                    pos_inc(1);
                    cur_tk_ = Token(ID, sub(start, pos()));
                }
                else
                    cur_tk_ = Token(ERR, "IDENTIFIER WITH UNTERMINATED [");

                /* check */
            } break;
            default: {
                if (is_identifier_begin(c)) {
                    pos_inc(1);
                    c = char_at(pos());
                    while (is_identifier_body(c)) {
                        c = char_at(pos_inc(1));
                    }

                    std::string s = sub(start, pos());
                    if (!check_reserved_keyword(s))
                        cur_tk_ = Token(ID, s);
                }
                else {
                    std::string err_s = "UNEXPECTED ";
                    cur_tk_ = Token(ERR, err_s + c);
                }

            } break;
        }
    }

    void scanf_number() {
        unsigned int start = pos();
        char c = char_at(pos());
        assert(is_dec_body(c) || c == '.');
        if (c == '.') {
            /* [0-9]+ */
            c = char_at(pos_inc(1));
            if (is_dec_body(c)) {
                while (is_dec_body(c))
                    c = char_at(pos_inc(1));
            }
            else {
                cur_tk_ = Token(ERR, "ERR NUMBER .");
                return;
            }
        }
        else {
            pos_inc(1);
            c = char_at(pos());
            if ('X' == c || 'x' == c) {
                /* Hexadecimal Number */
                c = char_at(pos_inc(1));
                while (is_hex_body(c))
                    c = char_at(pos_inc(1));
                cur_tk_ = Token(NUMBER, sub(start, pos()));
                return;
            }
            else {
                while (is_dec_body(c))
                    c = char_at(pos_inc(1));
                if (c == '.') {
                    c = char_at(pos_inc(1));
                    /* [0-9]* */
                    while (is_dec_body(c))
                        c = char_at(pos_inc(1));
                }
            }

        }
        if (c == 'e' || c == 'E') {
            c = char_at(pos_inc(1));
            if (c == '-' || c == '+')
                c = char_at(pos_inc(1));
            if (is_dec_body(c)) {
                c = char_at(pos_inc(1));
                while (is_dec_body(c))
                    c = char_at(pos_inc(1));
            }
            else {
                cur_tk_ = Token(ERR, "ERR NUMBER E");
                return;
            }
        }
        cur_tk_ = Token(NUMBER, sub(start, pos()));
    }

    bool is_dec_body(char c) {
        return '0' <= c && c <= '9';
    }

    bool is_hex_body(char c) {
        return ('0' <= c && c <= '9') ||
               ('A' <= c && c <= 'F') ||
               ('a' <= c && c <= 'f');
    }

    bool is_identifier_begin(char c) {
        return ('a' <= c && c <= 'z') ||
               ('A' <= c && c <= 'Z') ||
               '_' == c;
    }

    bool is_identifier_body(char c) {
        return is_identifier_begin(c) || ('0' <= c && c <= '9');
    }

    bool is_white(char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
    }

    unsigned pos_inc(unsigned int inc) {
        pos_ += inc;
        col_ += inc;
        if (pos_ > sql_.length())
            cur_tk_ = Token(END_P, "");
        return pos_;
    }

    unsigned pos() const { return pos_; }

    void new_line() {
        line_++;
        col_ = 0;
    }

    bool check_reserved_keyword(const std::string& word) {
        std::string big = word;
        std::transform(big.begin(), big.end(), big.begin(), ::toupper);
        auto it = keyword_.find(big);
        if (it != keyword_.end()) {
            cur_tk_ = Token(it->second, word);
            return true;
        }
        return false;
    }

    char char_at(unsigned int pos) {
        if (pos < sql_.length())
            return sql_[pos];
        else
            return EOI;
    }

    std::string sub(unsigned int start, unsigned int end) {
        return sql_.substr(start, end - start);
    }

    Token cur_tk_;
    std::string sql_;

    unsigned int pos_;
    unsigned int line_;
    unsigned int col_;
    std::map<std::string, TokenType > keyword_;

};

Token::Token() : type_(none) {}


/* todo */
Lex::Lex(const char *sql) : sql_(sql), pos_(0), line_(0), col_(0), keyword_{
        {"ALL", ALL}, {"AND", AND}, {"ANY", ANY}, {"AS", AS}, {"ASC", ASC}, {"AVG", AVG},
        {"BETWEEN", BETWEEN}, {"BY", BY},
        {"CASE", CASE}, {"CAST", CAST}, {"COLLATE", COLLATE}, {"CONVERT", CONVERT}, {"COUNT", COUNT}, {"CROSS", CROSS},
        {"DEFAULT", DEFAULT}, {"DELETE", DELETE}, {"DESC", DESC}, {"DISTINCT", DISTINCT},
        {"ELSE", ELSE}, {"END", END}, {"ESCAPE", ESCAPE}, {"EXCEPT", EXCEPT}, {"EXISTS", EXISTS},
        {"FALSE", FALSE}, {"FROM", FROM}, {"FULL", FULL},
        {"GROUP", GROUP}, {"GROUPING", GROUPING},
        {"HAVING", HAVING},
        {"IN", IN}, {"INNER", INNER}, {"INSERT", INSERT}, {"INTERSECT", INTERSECT}, {"INTERVAL", INTERVAL}, {"INTO", INTO}, {"IS", IS},
        {"JOIN", JOIN},
        {"LEFT", LEFT}, {"LIKE", LIKE},
        {"MAX", MAX}, {"MATCH", MATCH},
        {"MIN", MIN}, {"NATURAL", NATURAL}, {"NOT", NOT}, {"NULLIF", NULLIF}, {"NULL", NULLX},
        {"ON", ON}, {"OR", OR}, {"ORDER", ORDER}, {"OUTER", OUTER}, {"OVER", OVER}, {"OVERLAPS", OVERLAPS},
        {"PARTIAL", PARTIAL},
        {"RANK", RANK}, {"RIGHT", RIGHT},
        {"SELECT", SELECT}, {"SET", SET}, {"SOME", SOME}, {"SUM", SUM},
        {"THEN", THEN}, {"TO", TO}, {"TRUE", TRUE},
        {"UNION", UNION}, {"UNIQUE", UNIQUE}, {"UNKNOWN", UNKNOWN}, {"UPDATE", UPDATE},
        {"VALUES", VALUES},
        {"WHEN", WHEN}, {"WHERE", WHERE}, {"WITH", WITH}
} {}

void Lex::scanf() {
    if (cur_tk_.type_ == ERR || cur_tk_.type_ == END_P) return;
    while (true) {
        switch (char_at(pos())) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '\f': {
                white();
            } break;
            case '-': {
                if (char_at(pos()+1) == '-')
                    comment();
                else
                    return scanf_operator();
            } break;
            case '/': {
                if (char_at(pos()+1) == '*')
                    comment();
                else
                    return scanf_operator();
            } break;
            case '^':
            case '%':
            case '*':
            case '+':
            case '.':
            case '(':
            case ')':
            case ',':
            case ';':
            case '?':
            case '=':
            case '>':
            case '<':
            case '|': {
                if ('.' == char_at(pos()) && is_dec_body(char_at(pos()+1)))
                    return scanf_number();
                return scanf_operator();
            } break;
            case '\'': { return  scanf_str_literal(); } break;
            case '"':
            case '[': { return scanf_identifier(); } break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': { return scanf_number(); } break;
            case EOI: { return; } break;
            default: {
                if (is_identifier_begin(char_at(pos()))) {
                    return scanf_identifier();
                }
                else {
                    char c = char_at(pos());
                    std::string err_s = "UNEXPECTED ";
                    cur_tk_ = Token(ERR, err_s+c);
                    return;
                }
            } break;
        }
    }
}

void Lex::white() {
    char c = EOI;
    while (is_white(c = char_at(pos()))) {
        pos_inc(1);
        if (c == '\n')
            new_line();
    }

}

void Lex::comment() {
    if (char_at(pos()) == '-' && char_at(pos()+1) == '-') {
        single_line_comment();
    }
    else if (char_at(pos()) == '/' && char_at(pos()+1) == '*') {
        multi_line_comment();
    }
}

void Lex::single_line_comment() {
    pos_inc(2);    /* skip -- */
    while (char_at(pos()) != EOI && char_at(pos()) != '\n')
        pos_inc(1);
    if (char_at(pos()) == '\n') {
        pos_inc(1);
        new_line();
    }

}

void Lex::multi_line_comment() {
    pos_inc(2);  /* skip /* */
    char c1 = char_at(pos()), c2 = char_at(pos()+1);
    while (c1 != EOI && c2 != EOI) {
        if (c1 == '*' && c2 == '/')
            break;
        pos_inc(1);
        c1 = char_at(pos()), c2 = char_at(pos()+1);
    }
    if (c1 == '*' && c2 == '/')
        pos_inc(2);  /* skip  */
    else {
        /* unterminated multiline comment */
        cur_tk_ = Token(ERR, "unterminated multiline comment");
    }
}

ILex *make_lex(const char *sql) {
    return new Lex(sql);
}
