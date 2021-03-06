#include "lex.h"
#include "predicate.h"
#include "search_condition.h"
#include "parse.h"
#include <assert.h>
#include <search_condition.h>
#include "row_value_constructor.h"

/* contained */
/* check A contained in B */
bool contained(Predicate *A, BooleanTerm *B);
bool contained(Predicate *A, SearchCondition *B);
bool contained(BooleanTerm *A, Predicate *B);
bool contained(BooleanTerm *A, BooleanTerm *B);
bool contained(BooleanTerm *A, SearchCondition *B);
bool contained(SearchCondition *A, Predicate *B);
bool contained(SearchCondition *A, BooleanTerm *B);
bool contained(SearchCondition *A, SearchCondition *B);

/* mutexed */
bool mutexed(Predicate *A, BooleanTerm *B);
bool mutexed(Predicate *A, SearchCondition *B);
bool mutexed(BooleanTerm *A, Predicate *B);
bool mutexed(BooleanTerm *A, BooleanTerm *B);
bool mutexed(BooleanTerm *A, SearchCondition *B);
bool mutexed(SearchCondition *A, Predicate *B);
bool mutexed(SearchCondition *A, BooleanTerm *B);
bool mutexed(SearchCondition *A, SearchCondition *B);


void optimize(BooleanTerm *A);
void format(BooleanTerm *A, Buf *dst);
BooleanTerm *bt(BooleanTerm *A);
BooleanTermList *effective_next(BooleanTermList *i);
BooleanFactorList *effective_next(BooleanFactorList *i);

BooleanTermList *make_boolean_term_list(BooleanTerm *term, BooleanTermList *list) {
    BooleanTermList *r = new BooleanTermList;
    r->term_ = term;
    r->next_ = list;
    r->is_optimized_ = false;
    return r;
}

BooleanFactorList *make_boolean_factor_list(BooleanFactor *factor, BooleanFactorList *list) {
    BooleanFactorList *r = new BooleanFactorList;
    r->is_optimized_ = false;
    r->factor_ = factor;
    r->next_ = list;
    return r;
}

void free_booleanprimary(BooleanPrimary *primary);
BooleanPrimary *make_boolean_primary(ILex *lex, ParseResult *pr) {
    BooleanPrimary *boolean_primary = new BooleanPrimary;
    boolean_primary->type_ = BooleanPrimary::PRE;
    boolean_primary->u.predicate_ = nullptr;
    IToken *tk = lex->token();
    if (tk->type() == LPAREN) {
        lex->next();
        SearchCondition *s = make_search_condition(lex, pr);
        if (error_occur(pr)) {
            free_booleanprimary(boolean_primary);
            return nullptr;
        }
        boolean_primary->type_ = BooleanPrimary::SEARCH;
        boolean_primary->u.search_ = s;
        tk = lex->token();
        if (tk->type() == RPAREN) {
            lex->next();
        }
        else {
            char ss[256] = { 0 };
            sprintf(ss, "unexpected %s at line: %d, col: %d expect %s here",
                    lex->get_token_type_name(tk->type()).c_str(), lex->cur_pos_line(), lex->cur_pos_col(),
                    lex->get_token_type_name(RPAREN).c_str());
            pr->fill_error(PARSE_FAILED, ss);
            free_booleanprimary(boolean_primary);
            return nullptr;
        }
    }
    else {
        Predicate *p = make_predicate(lex, pr);
        if (error_occur(pr)) {
            free_booleanprimary(boolean_primary);
            return nullptr;
        }
        else {
            boolean_primary->type_ = BooleanPrimary::PRE;
            boolean_primary->u.predicate_ = p;
        }
    }
    return boolean_primary;
}

void free_booleantest(BooleanTest *test);
BooleanTest *make_boolean_test(ILex *lex, ParseResult *pr) {
    BooleanTest *boolean_test = new BooleanTest;
    boolean_test->primary_ = nullptr;
    boolean_test->truth_value_ = BooleanTest::NIL;
    boolean_test->is_ = false;
    BooleanPrimary *primary = make_boolean_primary(lex, pr);
    if (error_occur(pr)) {
        free_booleantest(boolean_test);
        return nullptr;
    }
    boolean_test->primary_ = primary;
    IToken *tk = lex->token();
    if (tk->type() == IS) {
        lex->next();
        if ((tk = lex->token())->type() == NOT) {
            boolean_test->is_ = false;
            lex->next();
        }
        else {
            boolean_test->is_ = true;
        }
        tk = lex->token();
        switch (tk->type()) {
            case TRUE: { boolean_test->truth_value_ = BooleanTest::TRUE; } break;
            case FALSE: { boolean_test->truth_value_ = BooleanTest::FALSE; } break;
            case UNKNOWN: { boolean_test->truth_value_ = BooleanTest::UNKNOWN; } break;
            default: {
                char ss[256] = { 0 };
                sprintf(ss, "unexpected %s at line: %d, col: %d",
                        lex->get_token_type_name(tk->type()).c_str(), lex->cur_pos_line(), lex->cur_pos_col());
                pr->fill_error(PARSE_FAILED, ss);
                free_booleantest(boolean_test);
                return nullptr;
            } break;
        }
    }
    return boolean_test;
}

void free_booleanfactor(BooleanFactor *factor);
BooleanFactor *make_boolean_factor(ILex *lex, ParseResult *pr) {
    BooleanFactor *boolean_factor = new BooleanFactor;
    boolean_factor->is_not_ = false;
    boolean_factor->u.test_ = nullptr;
    IToken *tk = lex->token();
    if (tk->type() == NOT) {
        boolean_factor->is_not_ = true;
        lex->next();
        BooleanFactor *f = make_boolean_factor(lex, pr);
        if (error_occur(pr)) {
            free_booleanfactor(boolean_factor);
            return nullptr;
        }
        boolean_factor->u.factor_ = f;
    }
    else {
        boolean_factor->is_not_ = false;
        BooleanTest *test = make_boolean_test(lex, pr);
        if (error_occur(pr)) {
            free_booleanfactor(boolean_factor);
            return nullptr;
        }
        boolean_factor->u.test_ = test;
    }
    return boolean_factor;
}

void free_booleanterm(BooleanTerm *term);
BooleanTerm *make_boolean_term(ILex *lex, ParseResult *pr) {
    BooleanTerm *boolean_term = new BooleanTerm;
    boolean_term->is_optimized_ = false;
    boolean_term->boolean_factors_ = nullptr;
    BooleanFactor *factor = make_boolean_factor(lex, pr);
    if (error_occur(pr)) {
        free_booleanterm(boolean_term);
        return nullptr;
    }
    boolean_term->boolean_factors_ = make_boolean_factor_list(factor, nullptr);
    BooleanFactorList *tail = boolean_term->boolean_factors_;
    IToken *tk = nullptr;
    while ((tk = lex->token())->type() == AND) {
        lex->next();
        factor = make_boolean_factor(lex, pr);
        if (error_occur(pr)) {
            free_booleanterm(boolean_term);
            return nullptr;
        }
        tail->next_ = make_boolean_factor_list(factor, nullptr);
        tail = tail->next_;
    }
    return boolean_term;
}

SearchCondition *make_search_condition(ILex *lex, ParseResult *pr) {
    SearchCondition *search_condition = new SearchCondition;
    search_condition->is_optimized_ = false;
    search_condition->boolean_terms_ = nullptr;
    BooleanTerm *term = make_boolean_term(lex, pr);
    if (error_occur(pr)) {
        free_search_condition(search_condition);
        return nullptr;
    }
    search_condition->boolean_terms_ = make_boolean_term_list(term, nullptr);
    BooleanTermList *tail = search_condition->boolean_terms_;
    IToken *tk = nullptr;
    while ((tk = lex->token())->type() == OR) {
        lex->next();
        term = make_boolean_term(lex, pr);
        if (error_occur(pr)) {
            free_search_condition(search_condition);
            return nullptr;
        }
        tail->next_ = make_boolean_term_list(term, nullptr);
        tail = tail->next_;
    }
    return search_condition;
}

void free_booleanprimary(BooleanPrimary *primary) {
    if (!primary)
        return;
    switch (primary->type_) {
        case BooleanPrimary::PRE: { free_predicate(primary->u.predicate_); primary->u.predicate_ = nullptr; } break;
        case BooleanPrimary::SEARCH: { free_search_condition(primary->u.search_); primary->u.search_ = nullptr; } break;
        default: break;
    }
    delete(primary);
}

void free_booleantest(BooleanTest *test) {
    if (!test)
        return;
    assert(test->is_ == false);
    free_booleanprimary(test->primary_);
    test->primary_ = nullptr;
    delete(test);
}

void free_booleanfactor(BooleanFactor *factor) {
    if (!factor)
        return;
    assert(factor->is_not_ == false);
    free_booleantest(factor->u.test_);
    factor->u.test_ = nullptr;
    delete(factor);
}

void free_booleanfactor_list(BooleanFactorList *list) {
    if (!list)
        return;
    for (BooleanFactorList *it = list; it != nullptr; ) {
        BooleanFactorList *n = it;
        free_booleanfactor(it->factor_);
        it->factor_ = nullptr;
        it = it->next_;
        delete(n);
    }
}
void free_booleanterm(BooleanTerm *term) {
    if (!term)
        return;
    free_booleanfactor_list(term->boolean_factors_);
    term->boolean_factors_ = nullptr;
    delete(term);
}
void free_booleanterm_list(BooleanTermList *list) {
    if (!list)
        return;
    for (BooleanTermList *it = list; it != nullptr;) {
        BooleanTermList *n = it;
        free_booleanterm(n->term_);
        n->term_ = nullptr;
        it = it->next_;
        delete(n);
    }
}

void free_search_condition(SearchCondition *sc) {
    if (!sc)
        return;
    free_booleanterm_list(sc->boolean_terms_);
    sc->boolean_terms_ = nullptr;
    delete(sc);
}

/* contains */
/* check A contained in B */
BooleanPrimary *booleanfactor2primary(BooleanFactor *factor) {
    assert(factor->is_not_ == false);
    BooleanTest *test = factor->u.test_;
    assert(test->is_ == false);
    BooleanPrimary *primary = test->primary_;
    return primary;
}


bool contained(Predicate *A, BooleanTerm *B) {
    B = bt(B);
    for (BooleanFactorList *it = effective_next(B->boolean_factors_); it != nullptr; it = effective_next(it->next_)) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                if (!contained(A, primary->u.predicate_))
                    return false;
            } break;
            case BooleanPrimary::SEARCH: {
                if (!contained(A, primary->u.search_))
                    return false;
            } break;
            default: assert(false);
        }
    }
    return true;
}

bool contained(Predicate *A, SearchCondition *B) {
    B = sc(B);
    for (BooleanTermList *it = effective_next(B->boolean_terms_); it != nullptr; it = effective_next(it->next_)) {
        BooleanTerm *term = it->term_;
        if (contained(A, term))
            return true;
    }
    return false;
}

bool contained(BooleanTerm *A, Predicate *B) {
    A = bt(A);
    for (BooleanFactorList *it = effective_next(A->boolean_factors_); it != nullptr; it = effective_next(it->next_)) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                if (contained(primary->u.predicate_, B))
                    return true;
            } break;
            case BooleanPrimary::SEARCH: {
                if (contained(primary->u.search_, B))
                    return true;
            } break;
            default: assert(false);
        }
    }
    return false;
}

bool contained(BooleanTerm *A, BooleanTerm *B) {
    A = bt(A); B = bt(B);
    bool ret = false;
    for (BooleanFactorList *it = effective_next(A->boolean_factors_); it != nullptr; it = effective_next(it->next_)) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                if (contained(primary->u.predicate_, B))
                    return true;
            } break;
            case BooleanPrimary::SEARCH: {
                if (contained(primary->u.search_, B))
                    return true;
            } break;
            default: assert(false);
        }
    }

    ret = true;
    for (BooleanFactorList *it = effective_next(B->boolean_factors_); it != nullptr; it = effective_next(it->next_)) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                if (!contained(A, primary->u.predicate_))
                    return false;
            } break;
            case BooleanPrimary::SEARCH: {
                if (!contained(A, primary->u.search_))
                    return false;
            } break;
            default: assert(false);
        }
    }
    return ret;
}

bool contained(BooleanTerm *A, SearchCondition *B) {
    A = bt(A); B = sc(B);
    bool ret = false;
    for (BooleanFactorList *it = effective_next(A->boolean_factors_); it != nullptr; it = effective_next(it->next_)) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                if (contained(primary->u.predicate_, B))
                    return true;
            } break;
            case BooleanPrimary::SEARCH: {
                if (contained(primary->u.search_, B))
                    return true;
            } break;
            default: assert(false);
        }
    }

    for (BooleanTermList *it = effective_next(B->boolean_terms_); it != nullptr; it = effective_next(it->next_)) {
        if (contained(A, it->term_))
            return true;
    }

    return ret;
}

bool contained(SearchCondition *A, Predicate *B) {
    A = sc(A);
    for (BooleanTermList *it = effective_next(A->boolean_terms_); it != nullptr; it = effective_next(it->next_)) {
        BooleanTerm *term = it->term_;
        if (!contained(term, B))
            return false;
    }
    return true;
}

bool contained(SearchCondition *A, BooleanTerm *B) {
    A = sc(A); B = bt(B);
    bool ret = true;
    for (BooleanTermList *it = effective_next(A->boolean_terms_); it != nullptr; it = effective_next(it->next_)) {
        BooleanTerm *term = it->term_;
        if (!contained(term, B)) {
            ret = false;
            break;
        }
    }
    if (ret)
        return true;
    for (BooleanFactorList *it = effective_next(B->boolean_factors_); it != nullptr; it = effective_next(it->next_)) {
        BooleanPrimary *p = booleanfactor2primary(it->factor_);
        switch (p->type_) {
            case BooleanPrimary::PRE: {
                if (!contained(A, p->u.predicate_))
                    return false;
            } break;
            case BooleanPrimary::SEARCH: {
                if (!contained(A, p->u.search_))
                    return false;
            } break;
            default: assert(false);
        }
    }
    return true;
}

bool contained(SearchCondition *A, SearchCondition *B) {
    A = sc(A); B = sc(B);
    bool ret = true;
    for (BooleanTermList *it = effective_next(A->boolean_terms_); it != nullptr; it = effective_next(it->next_)) {
        BooleanTerm *term = it->term_;
        if (!contained(term, B)) {
            ret = false;
            break;
        }
    }
    if (ret)
        return ret;
    ret = false;
    for (BooleanTermList *it = effective_next(B->boolean_terms_); it != nullptr; it = effective_next(it->next_)) {
        if (contained(A, it->term_)) {
            ret = true;
            break;
        }
    }
    return ret;
}

bool mutexed(Predicate *A, BooleanTerm *B) {
    if (B->is_optimized_)
        return true;
    else {
        for (BooleanFactorList *it = effective_next(B->boolean_factors_); it != nullptr; it = effective_next(it->next_)) {
            BooleanPrimary *primary = booleanfactor2primary(it->factor_);
            switch (primary->type_) {
                case BooleanPrimary::PRE: {
                    if (mutexed(A, primary->u.predicate_))
                        return true;
                } break;
                case BooleanPrimary::SEARCH: {
                    if (mutexed(A, primary->u.search_))
                        return true;
                } break;
                default: assert(false);
            }
        }
        return false;
    }
}

bool mutexed(Predicate *A, SearchCondition *B) {
    for (BooleanTermList *it = effective_next(B->boolean_terms_); it != nullptr; it = effective_next(it->next_)) {
        if (!mutexed(A, it->term_))
            return false;
    }
    return true;
}

bool mutexed(BooleanTerm *A, Predicate *B) {
    return mutexed(B, A);
}

bool mutexed(BooleanTerm *A, BooleanTerm *B) {
    if (A->is_optimized_ || B->is_optimized_)
        return true;
    for (BooleanFactorList *it = effective_next(A->boolean_factors_); it != nullptr; it = effective_next(it->next_)) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                if (mutexed(primary->u.predicate_, B))
                    return true;
            } break;
            case BooleanPrimary::SEARCH: {
                if (mutexed(primary->u.search_, B))
                    return true;
            } break;
            default: assert(false);
        }
    }

    for (BooleanFactorList *it = effective_next(B->boolean_factors_); it != nullptr; it = effective_next(it->next_)) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                if (mutexed(A, primary->u.predicate_))
                    return true;
            } break;
            case BooleanPrimary::SEARCH: {
                if (mutexed(A, primary->u.search_))
                    return true;
            } break;
            default: assert(false);
        }
    }
    return false;
}

bool mutexed(BooleanTerm *A, SearchCondition *B) {
    if (A->is_optimized_)
        return true;
    for (BooleanFactorList *it = effective_next(A->boolean_factors_); it != nullptr; it = effective_next(it->next_)) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                if (mutexed(primary->u.predicate_, B))
                    return true;
            } break;
            case BooleanPrimary::SEARCH: {
                if (mutexed(primary->u.search_, B))
                    return true;
            } break;
            default: assert(false);
        }
    }

    for (BooleanTermList *it = effective_next(B->boolean_terms_); it != nullptr; it = effective_next(it->next_)) {
        if (!mutexed(A, it->term_))
            return false;
    }
    return true;
}

bool mutexed(SearchCondition *A, Predicate *B) {
    return mutexed(B, A);
}

bool mutexed(SearchCondition *A, BooleanTerm *B) {
    return mutexed(B, A);
}

bool mutexed(SearchCondition *A, SearchCondition *B) {
    bool ret = true;
    for (BooleanTermList *it = effective_next(A->boolean_terms_); it != nullptr; it = effective_next(it->next_)) {
        if (!mutexed(it->term_, B)) {
            ret = false;
            break;
        }
    }
    if (ret)
        return true;
    for (BooleanTermList *it = effective_next(B->boolean_terms_); it != nullptr; it = effective_next(it->next_)) {
        if (!mutexed(A, it->term_))
            return false;
    }
    return true;
}


void optimize(SearchCondition *A) {
    A = sc(A);
    for (BooleanTermList *it = A->boolean_terms_; it != nullptr; it = it->next_) {
        optimize(it->term_);
    }

    for (BooleanTermList *it1 = A->boolean_terms_; it1 != nullptr; it1 = it1->next_) {
        for (BooleanTermList *it2 = it1->next_; it2 != nullptr; it2 = it2->next_) {
            bool is_con = false;
            is_con = contained(it1->term_, it2->term_);
            if (is_con) {
                it1->is_optimized_ = true;
            }
            else {
                is_con = contained(it2->term_, it1->term_);
                if (is_con) {
                    it2->is_optimized_ = true;
                }
            }
        }
    }
}

void optimize(BooleanTerm *A) {
    A = bt(A);
    for (BooleanFactorList *it = A->boolean_factors_; it != nullptr; it = it->next_) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        if (primary->type_ == BooleanPrimary::SEARCH) {
            optimize(primary->u.search_);
        }
    }

    for (BooleanFactorList *it1 = A->boolean_factors_; it1 != nullptr; it1 = it1->next_) {
        for (BooleanFactorList *it2 = it1->next_; it2 != nullptr; it2 = it2->next_) {
            bool is_con = false;
            BooleanPrimary *p1 = booleanfactor2primary(it1->factor_);
            BooleanPrimary *p2 = booleanfactor2primary(it2->factor_);
            if (p1->type_ == BooleanPrimary::SEARCH && p2->type_ == BooleanPrimary::SEARCH) {
                //if (mutexed(p1->u.search_, p2->u.search_))
                //    A->is_optimized_ = true;
                is_con = contained(p1->u.search_, p2->u.search_);
            }
            else if (p1->type_ == BooleanPrimary::SEARCH && p2->type_ == BooleanPrimary::PRE) {
                //if (mutexed(p1->u.search_, p2->u.predicate_))
                //    A->is_optimized_ = true;
                is_con = contained(p1->u.search_, p2->u.predicate_);
            }
            else if (p1->type_ == BooleanPrimary::PRE && p2->type_ == BooleanPrimary::SEARCH) {
                //if (mutexed(p1->u.predicate_, p2->u.search_))
                //    A->is_optimized_ = true;
                is_con = contained(p1->u.predicate_, p2->u.search_);
            }
            else {
                //if (mutexed(p1->u.predicate_, p2->u.predicate_))
                //    A->is_optimized_ = true;
                is_con = contained(p1->u.predicate_, p2->u.predicate_);
            }
            if (is_con) {
                it2->is_optimized_ = true;
            }
            else {
                if (p1->type_ == BooleanPrimary::SEARCH && p2->type_ == BooleanPrimary::SEARCH) {
                    is_con = contained(p2->u.search_, p1->u.search_);
                }
                else if (p1->type_ == BooleanPrimary::SEARCH && p2->type_ == BooleanPrimary::PRE) {
                    is_con = contained(p2->u.predicate_, p1->u.search_);
                }
                else if (p1->type_ == BooleanPrimary::PRE && p2->type_ == BooleanPrimary::SEARCH) {
                    is_con = contained(p2->u.search_, p1->u.predicate_);
                }
                else {
                    is_con = contained(p2->u.predicate_, p1->u.predicate_);
                }
                if (is_con) {
                    it1->is_optimized_ = true;
                }
            }
        }
    }
}

BooleanTermList *effective_next(BooleanTermList *i) {
    if (!i)
        return nullptr;
    else {
        BooleanTermList *it = i;
        while (it) {
            if (it->is_optimized_ == false)
                break;
            it = it->next_;
        }
        return it;
    }
}

BooleanFactorList *effective_next(BooleanFactorList *i) {
    if (!i)
        return nullptr;
    else {
        BooleanFactorList *it = i;
        while (it) {
            if (it->is_optimized_ == false)
                break;
            it = it->next_;
        }
        return it;
    }
}

void format(SearchCondition *A, Buf *dst) {
    for (BooleanTermList *it = effective_next(A->boolean_terms_); it != nullptr; ) {
        format(it->term_, dst);
        BooleanTermList *next = effective_next(it->next_);
        if (next) {
            dst->append(" OR ");
        }
        it = next;
    }
}

bool check_term(BooleanTerm *A) {
    BooleanFactorList *a = effective_next(A->boolean_factors_);
    bool paren_flag = effective_next(a->next_) != nullptr;
    return paren_flag;
}

bool check_search_condition(SearchCondition *A) {
    BooleanTermList *a = effective_next(A->boolean_terms_);
    bool paren_flag = effective_next(a->next_) != nullptr;
    return paren_flag;
}

SearchCondition *sc(SearchCondition *A) {
    BooleanTerm *t = nullptr;
    if (!check_search_condition(A)) {
        t = effective_next(A->boolean_terms_)->term_;
    }
    else
        return A;
    if (!check_term(t)) {
        auto f = effective_next(t->boolean_factors_)->factor_;
        auto p = booleanfactor2primary(f);
        if (p->type_ == BooleanPrimary::SEARCH)
            return sc(p->u.search_);
        else {
            return A;
        }
    }
    else {
        return A;
    }
}

BooleanTerm *bt(BooleanTerm *A) {
    BooleanFactor *f = nullptr;
    if (!check_term(A))
        f = effective_next(A->boolean_factors_)->factor_;
    else
        return A;
    BooleanPrimary *p = booleanfactor2primary(f);
    if (p->type_ == BooleanPrimary::SEARCH) {
        if (!check_search_condition(p->u.search_)){
            return (effective_next(p->u.search_->boolean_terms_)->term_);
        }
        else {
            return A;
        }
    }
    else {
        return A;
    }
}

void format(BooleanTerm *A, Buf *dst) {
    if (A->is_optimized_) {
        dst->append("1 = 0");
        return;
    }
    bool paren_flag = check_term(A);
    int i = 0;
    for (BooleanFactorList *it = effective_next(A->boolean_factors_); it != nullptr; ++i) {
        BooleanPrimary *p = booleanfactor2primary(it->factor_);
        if (p->type_ == BooleanPrimary::PRE) {
            format(p->u.predicate_, dst);
        }
        else {
            /* first primary and without OR */
            bool flag1 = false;
            if (paren_flag ) {
                if (i == 0) {
                    if ((flag1 = check_search_condition(sc(p->u.search_)))) {
                        dst->append("(");
                    }
                }
                else {
                    dst->append("(");
                }
            }
            format(p->u.search_, dst);
            if (paren_flag ) {
                if (i == 0) {
                    if (flag1) {
                        dst->append(")");
                    }
                }
                else {
                    dst->append(")");
                }
            }
        }
        BooleanFactorList *next = effective_next(it->next_);
        if (next) {
            dst->append(" AND ");
        }
        it = next;
    }
}

