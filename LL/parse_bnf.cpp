#include "parse_bnf.h"
#include "ll.h"
#include <string>
#include <assert.h>
#include "symbol.h"

struct Lex {
    enum LexType { NONE, BAR, LEND, REND, LBRACE, RBRACE, LPAREN, RPAREN, ID, ID_, ID_ASSIGN, ATAT, ERR, END };
    Lex(const std::string& src) : src_(src), pos_(0) {}
    void next() {
        cur_word_type_ = token(cur_word_);
    }
    LexType tk() {
        return cur_word_type_;
    }
    std::string word() {
        return cur_word_;
    }
private:
    LexType token(std::string& out) {
        skip_white();
        if (pos_ >= src_.length())
            return END;
        switch (src_[pos_]) {
            case '@' : {
                pos_++;
                if (pos_ >= src_.length() || src_[pos_] != '@') {
                    cur_word_type_ = ERR;
                    return ERR;
                }
                out = "@@";
                pos_++;
                return ATAT;
            } break;
            case '<' : {
                pos_++;
                out = scan_ID(true);
                if (out.length() <= 0) {
                    cur_word_type_ = ERR;
                    return ERR;
                }
                if(src_[pos_] != '>') {
                    cur_word_type_ = ERR;
                    return ERR;
                }
                pos_++;
                skip_white();
                if (pos_ < src_.length()) {
                    if (src_[pos_] == ':') {
                        pos_++;
                        if(pos_ >= src_.length() || src_[pos_] != ':') {
                            cur_word_type_ = ERR;
                            return ERR;
                        }
                        pos_++;
                        if(pos_ >= src_.length() || src_[pos_] != '=') {
                            cur_word_type_ = ERR;
                            return ERR;
                        }
                        pos_++;
                        return ID_ASSIGN;
                    }
                    else {
                        return ID_;
                    }
                }
                else {
                    return ID_;
                }
            } break;
            case '|' : {
                out = "|";
                pos_++;
                return BAR;
            } break;
            case '[' : {
                out = "[";
                pos_++;
                return LEND;
            } break;
            case ']' : {
                out = "]";
                pos_++;
                return REND;
            } break;
            case '{' : {
                out = "{";
                pos_++;
                return LBRACE;
            } break;
            case '}' : {
                out = "}";
                pos_++;
                return RBRACE;
            } break;
            case '(' : {
                out = "(";
                pos_++;
                return LPAREN;
            } break;
            case ')' : {
                out = ")";
                pos_++;
                return RPAREN;
            } break;
            default: {
                out = scan_ID(false);
                if (out.length() <= 0) {
                    cur_word_type_ = ERR;
                    return ERR;
                }
                return ID;
            } break;
        }
    }
    void skip_white() {
        while (pos_ < src_.length() &&
            (src_[pos_] == ' ' || src_[pos_] == '\r' || src_[pos_] == '\t' || src_[pos_] == '\n'))
            pos_++;
    }
    std::string scan_ID(bool is_white_space) {
        if (!range_a_z_A_Z(src_[pos_]))
            return "";
        int ss = pos_;
        pos_++;
        while (pos_ < src_.length() &&(range_a_z_A_Z(src_[pos_]) ||
                                           range_0_9(src_[pos_]) ||
                                               '-' == src_[pos_] ||
                                               '_' == src_[pos_] ||
                                               ':' == src_[pos_] ||
                         (is_white_space && ' ' == src_[pos_])))
            pos_++;
        return src_.substr(ss, pos_-ss);
    };
    bool range_a_z_A_Z(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }
    bool range_0_9(char c) { return '0' <= c && c <= '9'; }

    std::string src_;
    size_t pos_;

    std::string cur_word_;
    LexType cur_word_type_;
};

enum INNER { OPTIONAL, ZEROORMORE, ONEORMORE, INN };

Symbol *generate(INNER tp) {
    static uint64_t II = UINT64_MAX;
    assert(II > 0);
    Symbol *ret = nullptr;
    switch (tp) {
        case OPTIONAL: {
            std::string m = "OPTIONAL_";
            m += std::to_string(II--);
            ret = make_symbol(m.c_str());
        } break;
        case ZEROORMORE: {
            std::string m = "ZERO_OR_MORE_";
            m += std::to_string(II--);
            ret = make_symbol(m.c_str());
        } break;
        case ONEORMORE: {
            std::string m = "ONE_OR_MORE_";
            m += std::to_string(II--);
            ret = make_symbol(m.c_str());
        } break;
        case INN: {
            std::string m = "INNER_";
            m += std::to_string(II--);
            ret = make_symbol(m.c_str());
        } break;
        default: assert(false);
    }
    def_syn_symbol(ret, false);
    return ret;
}

void tks(Lex *lex, bool is_terminal);
void rulelist(Lex *lex);
void rule_(Lex *lex);
Symbol *lhs(Lex *lex);
void rhs(Lex *lex, Symbol *target);
void alternatives(Lex *lex, Symbol *target);
void alternative(Lex *lex, Symbol *target);
Symbol *element(Lex *lex);
Symbol *optional(Lex *lex, Symbol *target);
Symbol *zeroormore(Lex *lex, Symbol *target);
Symbol *oneormore(Lex *lex, Symbol *target);
Symbol *text(Lex *lex);

void tks(Lex *lex, bool is_terminal) {
    while (lex->tk() == Lex::ID || lex->tk() == Lex::ID_) {
        def_syn_symbol(make_symbol(lex->word().c_str()), is_terminal);
        lex->next();
    }
}

void rulelist(Lex *lex) {
    while (lex->tk() == Lex::ID_ASSIGN)
        rule_(lex);
}

void rule_(Lex *lex) {
    Symbol *target = lhs(lex);
    rhs(lex, target);
}

Symbol *lhs(Lex *lex) {
    assert(lex->tk() == Lex::ID_ASSIGN);
    Symbol *ret = make_symbol(lex->word().c_str());
    lex->next();
    return ret;
}

void rhs(Lex *lex, Symbol *target) {
    alternatives(lex, target);
}

void alternatives(Lex *lex, Symbol *target) {
    alternative(lex, target);
    while (lex->tk() == Lex::BAR) {
        lex->next();
        alternative(lex, target);
    }
}

void alternative(Lex *lex, Symbol *target) {
    auto t = lex->tk();
    int cnt = 0;
    Symbol *coms[32] = { 0 };
    while (t == Lex::LEND || t == Lex::LBRACE || t == Lex::LPAREN ||
            t == Lex::ID || t == Lex::ID_) {
        Symbol *ele = element(lex);
        t = lex->tk();
        assert(ele);
        coms[cnt++] = ele;
    }
    if (cnt > 0)
        bind_rule(target, make_rule(target, cnt, coms));
    else
        bind_rule(target, nullptr);
}

Symbol *element(Lex *lex) {
    Symbol *ret = nullptr;
    switch (lex->tk()) {
        case Lex::LEND : {
            Symbol *inner_target = generate(INN);
            optional(lex, inner_target);
            ret = generate(OPTIONAL);
            bind_rule(ret, nullptr);
            bind_rule(ret, make_rule(ret, 1, &inner_target));
        } break;
        case Lex::LBRACE : {
            Symbol *inner_target = generate(INN);
            zeroormore(lex, inner_target);
            ret = generate(ZEROORMORE);
            bind_rule(ret, nullptr);
            Symbol *coms[2] = { inner_target, ret };
            bind_rule(ret, make_rule(ret, 2, coms));
        } break;
        case Lex::LPAREN : {
            Symbol *inner_target = generate(INN);
            oneormore(lex, inner_target);
            ret = generate(ONEORMORE);
            Symbol *coms[2] = { inner_target, ret };
            bind_rule(ret, make_rule(ret, 2, coms));
        } break;
        case Lex::ID : {
            return text(lex);
        } break;
        case Lex::ID_ : {
            ret = make_symbol(lex->word().c_str());
            lex->next();
        } break;
        default: assert(false);
    }
    return ret;
}

Symbol *optional(Lex *lex, Symbol *target) {
    assert(lex->tk() == Lex::LEND);
    lex->next();
    alternatives(lex, target);
    assert(lex->tk() == Lex::REND);
    lex->next();
    return target;
}

Symbol *zeroormore(Lex *lex, Symbol *target) {
    assert(lex->tk() == Lex::LBRACE);
    lex->next();
    alternatives(lex, target);
    assert(lex->tk() == Lex::RBRACE);
    lex->next();
    return target;
}

Symbol *oneormore(Lex *lex, Symbol *target) {
    assert(lex->tk() == Lex::LPAREN);
    lex->next();
    alternatives(lex, target);
    assert(lex->tk() == Lex::RPAREN);
    lex->next();
    return target;
}

Symbol *text(Lex *lex) {
    assert(lex->tk() == Lex::ID);
    Symbol *ret = make_symbol(lex->word().c_str());
    lex->next();
    return ret;
}

void parse_bnf(const char *bnf) {
    std::string src(bnf);
    Lex lex(src);
    Lex::LexType tp;
    std::string word;
    lex.next();
    assert(lex.tk() == Lex::ATAT);
    lex.next();
    tks(&lex, true);
    assert(lex.tk() == Lex::ATAT);
    lex.next();
    tks(&lex, false);
    assert(lex.tk() == Lex::ATAT);
    lex.next();
    rulelist(&lex);
    assert(lex.tk() == Lex::END);
    proc();
}


