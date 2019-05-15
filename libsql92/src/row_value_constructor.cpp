#include <parse.h>
#include <row_value_constructor.h>
#include "row_value_constructor.h"
#include "lex.h"
#include "parse.h"
#include <string.h>
#include <stdlib.h>
#include <string>
#include <assert.h>

RowValueConstructor *make_row_value_ctor(ILex *lex, ParseResult *pr) {
    RowValueConstructor *r = new RowValueConstructor;
    IToken *tk = lex->token();
    switch (tk->type()) {
        case STR_LITERAL: {
            r->type_ = RowValueConstructor::STR_LIT;
            r->u.str_lit_ = strdup(tk->word());
            lex->next();
        } break;
        case PLUS:
        case MINUS: {
            TokenType op = tk->type();
            lex->next();
            tk = lex->token();
            if (tk->type() == NUMBER) {
                std::string o = op == MINUS ? "-" : "+";
                o += tk->word();
                r->type_ = RowValueConstructor::INT_LIT;
                r->u.int_lit_ = atoi(o.c_str());
                lex->next();
            }
            else {
                pr->error_ = PARSE_FAILED;
                return nullptr;
            }
        } break;
        case NUMBER: {
            r->type_ = RowValueConstructor::INT_LIT;
            r->u.int_lit_ = atoi(tk->word());
            lex->next();
        } break;
        case ID: {
            r->type_ = RowValueConstructor::COLUMN_REF;
            IdentifierChain *head = new IdentifierChain;
            IdentifierChain *tail = head;
            head->identifier_ = strdup(tk->word());
            head->next_ = nullptr;
            r->u.column_ref_ = head;
            lex->next();
            tk = lex->token();
            while (tk->type() == DOT) {
                lex->next();
                tk = lex->token();
                if (tk->type() == ID) {
                    IdentifierChain *n = new IdentifierChain;
                    n->identifier_ = strdup(tk->word());
                    n->next_ = nullptr;
                    tail->next_ = n;
                    tail = tail->next_;
                    lex->next();
                }
                else {
                    pr->error_ = PARSE_FAILED;
                    return nullptr;
                }
            }
        } break;
        default: {
            pr->error_ = PARSE_FAILED;
            return nullptr;
        }
    }
    return r;
}

void format(RowValueConstructor *A, Buf *dst) {
    switch (A->type_) {
        case RowValueConstructor::COLUMN_REF: {
            for (IdentifierChain *it = A->u.column_ref_; it != nullptr; it = it->next_) {
                dst->append(it->identifier_);
                if (it->next_)
                    dst->append(".");
            }
        } break;
        case RowValueConstructor::STR_LIT: {
            dst->append(A->u.str_lit_);
        } break;
        case RowValueConstructor::INT_LIT: {
            dst->append(A->u.int_lit_);
        } break;
        default: assert(false);
    }
}