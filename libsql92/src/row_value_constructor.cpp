#include <parse.h>
#include <row_value_constructor.h>
#include "row_value_constructor.h"
#include "lex.h"
#include "parse.h"
#include <string.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <malloc.h>

Id *make_id(const char *identifier, const char *word) {
    Id *id = new Id;
    id->identifier_ = strdup(identifier);
    id->word_ = strdup(word);
    return id;
}

void free_id(Id *id) {
    if (!id)
        return;
    free((char*)id->identifier_);
    id->identifier_ = nullptr;
    free((char*)id->word_);
    id->word_ = nullptr;
    delete(id);
}

StringLiteral *make_string_literal(const char *content, const char *word) {
    StringLiteral *string_literal = new StringLiteral;
    string_literal->content_ = strdup(content);
    string_literal->word_ = strdup(word);
    return string_literal;
}

void free_string_literal(StringLiteral *string_literal) {
    if (!string_literal)
        return;
    free((char*)string_literal->content_);
    string_literal->content_ = nullptr;
    free((char*)string_literal->word_);
    string_literal->word_ = nullptr;
    delete(string_literal);
}

RowValueConstructor *make_row_value_ctor(ILex *lex, ParseResult *pr) {
    RowValueConstructor *r = new RowValueConstructor;
    IToken *tk = lex->token();
    switch (tk->type()) {
        case STR_LITERAL: {
            r->type_ = RowValueConstructor::STR_LIT;
            r->u.str_lit_ = make_string_literal(tk->word_semantic(), tk->word());
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
                char ss[256] = { 0 };
                sprintf(ss, "unexpected %s at line: %d, col: %d expect %s here",
                        lex->get_token_type_name(tk->type()).c_str(), lex->cur_pos_line(), lex->cur_pos_col(),
                        lex->get_token_type_name(NUMBER).c_str());
                pr->fill_error(PARSE_FAILED, ss);
                free_row_value_ctor(r);
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
            head->identifier_ = make_id(tk->word_semantic(), tk->word());
            head->next_ = nullptr;
            r->u.column_ref_ = head;
            lex->next();
            tk = lex->token();
            while (tk->type() == DOT) {
                lex->next();
                tk = lex->token();
                if (tk->type() == ID) {
                    IdentifierChain *n = new IdentifierChain;
                    n->identifier_ = make_id(tk->word_semantic(), tk->word());
                    n->next_ = nullptr;
                    tail->next_ = n;
                    tail = tail->next_;
                    lex->next();
                }
                else {
                    char ss[256] = { 0 };
                    sprintf(ss, "unexpected %s at line: %d, col: %d expect %s here",
                            lex->get_token_type_name(tk->type()).c_str(), lex->cur_pos_line(), lex->cur_pos_col(),
                            lex->get_token_type_name(ID).c_str());
                    pr->fill_error(PARSE_FAILED, ss);
                    free_row_value_ctor(r);
                    return nullptr;
                }
            }
        } break;
        default: {
            char ss[256] = { 0 };
            sprintf(ss, "unexpected %s at line: %d, col: %d",
                    lex->get_token_type_name(tk->type()).c_str(), lex->cur_pos_line(), lex->cur_pos_col());
            pr->fill_error(PARSE_FAILED, ss);
            return nullptr;
        }
    }
    return r;
}

void free_identifier_chain(IdentifierChain *chain) {
    if (!chain)
        return;
    for (IdentifierChain *it = chain; it != nullptr;) {
        free_id(it->identifier_);
        it->identifier_ = nullptr;
        IdentifierChain *n = it;
        it = it->next_;
        delete(n);
        n = nullptr;
    }
}

void free_row_value_ctor(RowValueConstructor *row) {
    if (!row)
        return;
    assert(row->type_ == RowValueConstructor::COLUMN_REF ||
            row->type_ == RowValueConstructor::INT_LIT ||
             row->type_ == RowValueConstructor::STR_LIT);
    switch (row->type_) {
        case RowValueConstructor::COLUMN_REF: { free_identifier_chain(row->u.column_ref_); row->u.column_ref_ = nullptr; } break;
        case RowValueConstructor::INT_LIT: {  } break;
        case RowValueConstructor::STR_LIT: { free_string_literal(row->u.str_lit_); row->u.str_lit_ = nullptr; } break;
        default: break;
    }
    delete(row);
}

void format(RowValueConstructor *A, Buf *dst) {
    switch (A->type_) {
        case RowValueConstructor::COLUMN_REF: {
            for (IdentifierChain *it = A->u.column_ref_; it != nullptr; it = it->next_) {
                dst->append(it->identifier_->word_);
                if (it->next_)
                    dst->append(".");
            }
        } break;
        case RowValueConstructor::STR_LIT: {
            dst->append(A->u.str_lit_->word_);
        } break;
        case RowValueConstructor::INT_LIT: {
            dst->append(A->u.int_lit_);
        } break;
        default: assert(false);
    }
}

bool is_same_ref(IdentifierChain *ref1, IdentifierChain *ref2) {
    IdentifierChain *it1 = nullptr, *it2 = nullptr;
    for (it1 = ref1, it2 = ref2; it1 != nullptr && it2 != nullptr; it1 = it1->next_, it2 = it2->next_) {
        if (0 != strcasecmp(it1->identifier_->identifier_, it2->identifier_->identifier_))
            return false;
    }
    if (it1 == nullptr && it2 == nullptr)
        return true;
    else
        return false;
}

bool is_same_lit(RowValueConstructor *lit1, RowValueConstructor *lit2) {
    assert(lit1->type_ == RowValueConstructor::INT_LIT || lit1->type_ == RowValueConstructor::STR_LIT);
    assert(lit2->type_ == RowValueConstructor::INT_LIT || lit2->type_ == RowValueConstructor::STR_LIT);
    if (lit1->type_ == RowValueConstructor::INT_LIT && lit2->type_ == RowValueConstructor::INT_LIT) {
        return lit1->u.int_lit_ == lit2->u.int_lit_;
    }
    else if (lit1->type_ == RowValueConstructor::STR_LIT && lit2->type_ == RowValueConstructor::STR_LIT) {
        return 0 == strcmp(lit1->u.str_lit_->content_, lit2->u.str_lit_->content_);
    }
    else if (lit1->type_ == RowValueConstructor::INT_LIT && lit2->type_ == RowValueConstructor::STR_LIT) {
        char *end = nullptr;
        long int l2 = strtol(lit2->u.str_lit_->content_, &end, 10);
        if (*lit2->u.str_lit_->content_ != '\0' && *end == '\0') {
            return l2 == lit1->u.int_lit_;
        }
        else
            return false;
    }
    else {
        char *end = nullptr;
        long int l1 = strtol(lit1->u.str_lit_->content_, &end, 10);
        if (*lit1->u.str_lit_->content_ != '\0' && *end == '\0') {
            return l1 == lit2->u.int_lit_;
        }
        else
            return false;
    }
}

bool is_lt_lit(RowValueConstructor *lit1, RowValueConstructor *lit2) {
    assert(lit1->type_ == RowValueConstructor::INT_LIT || lit1->type_ == RowValueConstructor::STR_LIT);
    assert(lit2->type_ == RowValueConstructor::INT_LIT || lit2->type_ == RowValueConstructor::STR_LIT);
    if (lit1->type_ == RowValueConstructor::INT_LIT && lit2->type_ == RowValueConstructor::INT_LIT) {
        return lit1->u.int_lit_ < lit2->u.int_lit_;
    }
    else if (lit1->type_ == RowValueConstructor::STR_LIT && lit2->type_ == RowValueConstructor::STR_LIT) {
        return false;   /* todo */
    }
    else if (lit1->type_ == RowValueConstructor::INT_LIT && lit2->type_ == RowValueConstructor::STR_LIT) {
        char *end = nullptr;
        long int l2 = strtol(lit2->u.str_lit_->content_, &end, 10);
        if (*lit2->u.str_lit_->content_ != '\0' && *end == '\0') {
            return l2 > lit1->u.int_lit_;
        }
        else
            return false;
    }
    else {
        char *end = nullptr;
        long int l1 = strtol(lit1->u.str_lit_->content_, &end, 10);
        if (*lit1->u.str_lit_->content_ != '\0' && *end == '\0') {
            return l1 < lit2->u.int_lit_;
        }
        else
            return false;
    }
}

bool is_gt_lit(RowValueConstructor *lit1, RowValueConstructor *lit2) {
    assert(lit1->type_ == RowValueConstructor::INT_LIT || lit1->type_ == RowValueConstructor::STR_LIT);
    assert(lit2->type_ == RowValueConstructor::INT_LIT || lit2->type_ == RowValueConstructor::STR_LIT);
    if (lit1->type_ == RowValueConstructor::INT_LIT && lit2->type_ == RowValueConstructor::INT_LIT) {
        return lit1->u.int_lit_ > lit2->u.int_lit_;
    }
    else if (lit1->type_ == RowValueConstructor::STR_LIT && lit2->type_ == RowValueConstructor::STR_LIT) {
        return false;   /* todo */
    }
    else if (lit1->type_ == RowValueConstructor::INT_LIT && lit2->type_ == RowValueConstructor::STR_LIT) {
        char *end = nullptr;
        long int l2 = strtol(lit2->u.str_lit_->content_, &end, 10);
        if (*lit2->u.str_lit_->content_ != '\0' && *end == '\0') {
            return l2 < lit1->u.int_lit_;
        }
        else
            return false;
    }
    else {
        char *end = nullptr;
        long int l1 = strtol(lit1->u.str_lit_->content_, &end, 10);
        if (*lit1->u.str_lit_->content_ != '\0' && *end == '\0') {
            return l1 > lit2->u.int_lit_;
        }
        else
            return false;
    }
}