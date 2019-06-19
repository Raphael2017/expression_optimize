#include <predicate.h>
#include "predicate.h"
#include "lex.h"
#include "parse.h"
#include "row_value_constructor.h"
#include "query_expression.h"
#include <assert.h>
#include <string.h>
#include <row_value_constructor.h>

ComparisonPredicate *make_comp_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first, TokenType op);
BetweenPredicate *make_between_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first, bool has_not);
InPredicate *make_in_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first, bool has_not);
LikePredicate *make_like_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first, bool has_not);
NullPredicate *make_null_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first);
QuantifiedComparisonPredicate *make_quan_comp_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first, TokenType op);
ExistsPredicate *make_exists_predicate(ILex *lex, ParseResult *pr);
MatchPredicate *make_match_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first);
OverlapsPredicate *make_overlaps_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first);

void free_comp_predicate(ComparisonPredicate*);

ComparisonPredicate *make_comp_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first, TokenType op) {
    ComparisonPredicate *cp = new ComparisonPredicate;
    cp->row1_ = first;
    cp->row2_ = nullptr;
    switch (op) {
        case EQ: { cp->comp_op_ = ComparisonPredicate::EQ; } break;
        case LTGT: { cp->comp_op_ = ComparisonPredicate::NEQ; } break;
        case LT: { cp->comp_op_ = ComparisonPredicate::LT; } break;
        case GT: { cp->comp_op_ = ComparisonPredicate::GT; } break;
        case LTEQ: { cp->comp_op_ = ComparisonPredicate::LTEQ; } break;
        case GTEQ: { cp->comp_op_ = ComparisonPredicate::GTEQ; } break;
        default: {
            assert(false);
        } break;
    }
    RowValueConstructor *row2 = make_row_value_ctor(lex, pr);
    if (error_occur(pr)) {
        free_comp_predicate(cp);
        return nullptr;
    }
    cp->row2_ = row2;
    return cp;
}

BetweenPredicate *make_between_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first, bool has_not) {
    return nullptr;
    BetweenPredicate *bp = new BetweenPredicate;
    bp->row1_ = first;
    bp->is_not_ = has_not;
    bp->row2_ = nullptr;
    bp->row3_ = nullptr;
    IToken *tk = lex->token();
    assert(tk->type() == BETWEEN);
    lex->next();
    RowValueConstructor *row2 = make_row_value_ctor(lex, pr);
    if (error_occur(pr)) {
        return nullptr;
    }
    bp->row2_ = row2;
    tk = lex->token();
    if (tk->type() == AND) {
        lex->next();
        RowValueConstructor *row3 = make_row_value_ctor(lex, pr);
        if (error_occur(pr)) {
            return nullptr;
        }
        bp->row3_ = row3;
    }
    else {
        pr->error_ = PARSE_FAILED;
        return nullptr;
    }
    return bp;
}

InPredicate *make_in_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first, bool has_not) {
    return nullptr;
    InPredicate *ip = new InPredicate;
    ip->row_ = first;
    ip->is_not_ = has_not;
    ip->u.subquery_ = nullptr;
    IToken *tk = lex->token();
    assert(tk->type() == IS);
    lex->next();
    tk = lex->token();
    if (tk->type() == LPAREN) {
        lex->next();
        RowValueConstructor *row = make_row_value_ctor(lex, pr);
        if (error_occur(pr)) {
            return nullptr;
        }
        RowValueConstructorList *r_list = make_row_value_ctor_list(row, nullptr);
        RowValueConstructorList *tail = r_list;
        tk = lex->token();
        while (tk->type() == COMMA) {
            lex->next();
            row = make_row_value_ctor(lex, pr);
            if (error_occur(pr)) {
                return nullptr;
            }
            tail->next_ = make_row_value_ctor_list(row, nullptr);
            tail = tail->next_;
        }
        /*
         * add some comment
         * */
        if (r_list->next_ == nullptr && is_subquery(r_list->row_)) {
            ip->type_ = InPredicate::SUBQUERY;
            ip->u.subquery_ = row_value_ctor_to_query_expression(r_list->row_);
        }
        else {
            ip->type_ = InPredicate::VALUE_LIST;
            ip->u.value_list_ = r_list;
        }
        tk = lex->token();
        if (tk->type() == RPAREN) {
            lex->next();
        }
        else {
            pr->error_ = PARSE_FAILED;
            return nullptr;
        }
    }
    else {
        pr->error_ = PARSE_FAILED;
        return nullptr;
    }
    return ip;
}

LikePredicate *make_like_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first, bool has_not) {
    return nullptr;
    LikePredicate *lp = new LikePredicate;
    lp->match_value_ = first;
    lp->is_not_ = has_not;
    lp->pattern_ = nullptr;
    lp->escape_ = nullptr;
    IToken *tk = lex->token();
    assert(tk->type() == LIKE);
    lex->next();
    RowValueConstructor *pattern = make_row_value_ctor(lex, pr);
    if (error_occur(pr)) {
        return nullptr;
    }
    lp->pattern_ = pattern;
    tk = lex->token();
    if (tk->type() == ESCAPE) {
        lex->next();
        RowValueConstructor *esc = make_row_value_ctor(lex, pr);
        if (error_occur(pr)) {
            return nullptr;
        }
        lp->escape_ = esc;
    }
    return lp;
}

void free_null_predicate(NullPredicate *np);

NullPredicate *make_null_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first) {
    NullPredicate *np = new NullPredicate;
    np->row_ = first;
    np->is_not_ = false;
    IToken *tk = lex->token();
    assert(tk->type() == IS);
    lex->next();
    tk = lex->token();
    if (tk->type() == NOT) {
        np->is_not_ = true;
        lex->next();
        tk = lex->token();
    }
    if (tk->type() == NULLX) {
        lex->next();
    }
    else {
        free_null_predicate(np);
        char ss[256] = { 0 };
        sprintf(ss, "unexpected %s at line: %d, col: %d expect %s here",
                lex->get_token_type_name(tk->type()).c_str(), lex->cur_pos_line(), lex->cur_pos_col(),
                lex->get_token_type_name(NULLX).c_str());
        pr->fill_error(PARSE_FAILED, ss);
        return nullptr;
    }
    return np;
}

QuantifiedComparisonPredicate *make_quan_comp_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first, TokenType op) {
    return nullptr;
    QuantifiedComparisonPredicate *qcp = new QuantifiedComparisonPredicate;
    qcp->row_ = first;
    switch (op) {
        case EQ: { qcp->comp_op_ = QuantifiedComparisonPredicate::EQ; } break;
        case LTGT: { qcp->comp_op_ = QuantifiedComparisonPredicate::NEQ; } break;
        case LT: { qcp->comp_op_ = QuantifiedComparisonPredicate::LT; } break;
        case GT: { qcp->comp_op_ = QuantifiedComparisonPredicate::GT; } break;
        case LTEQ: { qcp->comp_op_ = QuantifiedComparisonPredicate::LTEQ; } break;
        case GTEQ: { qcp->comp_op_ = QuantifiedComparisonPredicate::GTEQ; } break;
        default: {
            assert(false);
        } break;
    }
    qcp->subquery_ = nullptr;
    IToken *tk = lex->token();
    assert(tk->type() == ALL || tk->type() == SOME);
    lex->next();
    tk = lex->token();
    if (tk->type() == LPAREN) {
        lex->next();
        QueryExpression *qe = make_query_expression(lex, pr);
        if (error_occur(pr)) {
            return nullptr;
        }
        qcp->subquery_ = qe;
        tk = lex->token();
        if (tk->type() != RPAREN) {
            pr->error_ = PARSE_FAILED;
            return nullptr;
        }
    }
    else {
        pr->error_ = PARSE_FAILED;
        return nullptr;
    }
    return qcp;
}

ExistsPredicate *make_exists_predicate(ILex *lex, ParseResult *pr) {
    return nullptr;
    ExistsPredicate *ep = new ExistsPredicate;
    ep->subquery_ = nullptr;
    IToken *tk = lex->token();
    assert(tk->type() == EXISTS);
    lex->next();
    tk = lex->token();
    if (tk->type() == LPAREN) {
        lex->next();
        QueryExpression *qe = make_query_expression(lex, pr);
        if (error_occur(pr)) {
            return nullptr;
        }
        ep->subquery_ = qe;
        tk = lex->token();
        if (tk->type() != RPAREN) {
            pr->error_ = PARSE_FAILED;
            return nullptr;
        }
    }
    else {
        pr->error_ = PARSE_FAILED;
        return nullptr;
    }
    return ep;
}

MatchPredicate *make_match_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first) {
    return nullptr;
    MatchPredicate *mp = new MatchPredicate;
    mp->is_unique_ = false;
    mp->opt_ = MatchPredicate::NIL;
    mp->row_ = first;
    mp->subquery_ = nullptr;
    IToken *tk = lex->token();
    assert(tk->type() == MATCH);
    lex->next();
    tk = lex->token();
    if (tk->type() == UNIQUE) {
        mp->is_unique_ = true;
        lex->next();
        tk = lex->token();
    }
    if (tk->type() == PARTIAL) {
        mp->opt_ = MatchPredicate::PARTIAL;
        lex->next();
        tk = lex->token();
    }
    else if (tk->type() == FULL) {
        mp->opt_ = MatchPredicate::FULL;
        lex->next();
        tk = lex->token();
    }
    if (tk->type() == LPAREN) {
        lex->next();
        QueryExpression *qe = make_query_expression(lex, pr);
        if (error_occur(pr)) {
            return nullptr;
        }
        mp->subquery_ = qe;
        tk = lex->token();
        if (tk->type() != RPAREN) {
            pr->error_ = PARSE_FAILED;
            return nullptr;
        }
        else
            lex->next();
    }
    else {
        pr->error_ = PARSE_FAILED;
        return nullptr;
    }
    return mp;
}

OverlapsPredicate *make_overlaps_predicate(ILex *lex, ParseResult *pr, RowValueConstructor *first) {
    return nullptr;
    OverlapsPredicate *op = new OverlapsPredicate;
    op->row1_ = first;
    op->row2_ = nullptr;
    IToken *tk = lex->token();
    assert(tk->type() == OVERLAPS);
    lex->next();
    RowValueConstructor *row = make_row_value_ctor(lex, pr);
    if (error_occur(pr)) {
        return nullptr;
    }
    op->row2_ = row;
    return op;
}

Predicate *make_predicate(ILex *lex, ParseResult *pr) {
    Predicate *predicae = new Predicate;
    predicae->type_ = Predicate::NIL;
    IToken *tk = lex->token();
    switch (tk->type()) {
        case EXISTS: {
            ExistsPredicate *ep = make_exists_predicate(lex, pr);
            if (error_occur(pr)) {
                free_predicate(predicae);
                return nullptr;
            }
            predicae->type_ = Predicate::EP;
            predicae->u.ep_ = ep;
        } break;
        default: {
            RowValueConstructor *first = make_row_value_ctor(lex, pr);
            if (error_occur(pr)) {
                free_predicate(predicae);
                return nullptr;
            }
            tk = lex->token();
            switch (tk->type()) {
                case EQ:
                case LTGT:
                case LT:
                case GT:
                case LTEQ:
                case GTEQ: {
                    TokenType op = tk->type();
                    lex->next();
                    tk = lex->token();
                    if (tk->type() == ALL || tk->type() == SOME) {
                        QuantifiedComparisonPredicate *qcp = make_quan_comp_predicate(lex, pr, first, op);
                        if (error_occur(pr)) {
                            free_predicate(predicae);
                            return nullptr;
                        }
                        predicae->type_ = Predicate::QCP;
                        predicae->u.qcp_ = qcp;
                    }
                    else {
                        predicae->type_ = Predicate::CP;
                        ComparisonPredicate *cp = make_comp_predicate(lex, pr, first, op);
                        if (error_occur(pr)) {
                            free_predicate(predicae);
                            return nullptr;
                        }
                        predicae->u.cp_ = cp;
                    }
                } break;
                case NOT: {
                    lex->next();
                    tk = lex->token();
                    switch (tk->type()) {
                        case BETWEEN: {
                            BetweenPredicate *bp = make_between_predicate(lex, pr, first, true);
                            if (error_occur(pr)) {
                                free_predicate(predicae);
                                return nullptr;
                            }
                            predicae->type_ = Predicate::BP;
                            predicae->u.bp_ = bp;
                        } break;
                        case IN: {
                            InPredicate *ip = make_in_predicate(lex, pr, first, true);
                            if (error_occur(pr)) {
                                free_predicate(predicae);
                                return nullptr;
                            }
                            predicae->type_ = Predicate::IP;
                            predicae->u.ip_ = ip;
                        } break;
                        case LIKE: {
                            LikePredicate *lp = make_like_predicate(lex, pr, first, true);
                            if (error_occur(pr)) {
                                free_predicate(predicae);
                                return nullptr;
                            }
                            predicae->type_ = Predicate::LP;
                            predicae->u.lp_ = lp;
                        } break;
                        default: {
                            char ss[256] = { 0 };
                            sprintf(ss, "unexpected %s at line: %d, col: %d",
                                    lex->get_token_type_name(tk->type()).c_str(), lex->cur_pos_line(), lex->cur_pos_col());
                            return nullptr;
                        } break;
                    }
                } break;
                case BETWEEN: {
                    BetweenPredicate *bp = make_between_predicate(lex, pr, first, false);
                    if (error_occur(pr)) {
                        free_predicate(predicae);
                        return nullptr;
                    }
                    predicae->type_ = Predicate::BP;
                    predicae->u.bp_ = bp;
                } break;
                case IN: {
                    InPredicate *ip = make_in_predicate(lex, pr, first, false);
                    if (error_occur(pr)) {
                        free_predicate(predicae);
                        return nullptr;
                    }
                    predicae->type_ = Predicate::IP;
                    predicae->u.ip_ = ip;
                } break;
                case LIKE: {
                    LikePredicate *lp = make_like_predicate(lex, pr, first, false);
                    if (error_occur(pr)) {
                        free_predicate(predicae);
                        return nullptr;
                    }
                    predicae->type_ = Predicate::LP;
                    predicae->u.lp_ = lp;
                } break;
                case IS: {
                    NullPredicate *np = make_null_predicate(lex, pr, first);
                    if (error_occur(pr)) {
                        free_predicate(predicae);
                        return nullptr;
                    }
                    predicae->type_ = Predicate::NP;
                    predicae->u.np_ = np;
                } break;
                case MATCH: {
                    MatchPredicate *mp = make_match_predicate(lex, pr, first);
                    if (error_occur(pr)) {
                        free_predicate(predicae);
                        return nullptr;
                    }
                    predicae->type_ = Predicate::MP;
                    predicae->u.mp_ = mp;
                } break;
                case OVERLAPS: {
                    OverlapsPredicate *op = make_overlaps_predicate(lex, pr, first);
                    if (error_occur(pr)) {
                        free_predicate(predicae);
                        return nullptr;
                    }
                    predicae->type_ = Predicate::OP;
                    predicae->u.op_ = op;
                } break;
                default: {
                    char ss[256] = { 0 };
                    sprintf(ss, "unexpected %s at line: %d, col: %d",
                            lex->get_token_type_name(tk->type()).c_str(), lex->cur_pos_line(), lex->cur_pos_col());
                    pr->fill_error(PARSE_FAILED, ss);
                    free_row_value_ctor(first);
                    free_predicate(predicae);
                    return nullptr;
                } break;
            }
        } break;
    }
    return predicae;
}

void free_comp_predicate(ComparisonPredicate *cp) {
    if (!cp)
        return;
    free_row_value_ctor(cp->row1_);
    cp->row1_ = nullptr;
    free_row_value_ctor(cp->row2_);
    cp->row2_ = nullptr;
    delete(cp);
}

void free_null_predicate(NullPredicate *np) {
    if (!np)
        return;
    free_row_value_ctor(np->row_);
    np->row_ = nullptr;
    delete(np);
}

void free_predicate(Predicate *predicate) {
    if (!predicate)
        return;
    switch (predicate->type_) {
        case Predicate::CP: { free_comp_predicate(predicate->u.cp_); predicate->u.cp_ = nullptr; } break;
        case Predicate::NP: { free_null_predicate(predicate->u.np_); predicate->u.np_ = nullptr; } break;
        default: break;
    }
    delete(predicate);
}

ComparisonPredicate *to_comparison_predicate(Predicate *predicate) {
    assert(predicate->type_ == Predicate::CP);
    return predicate->u.cp_;
}

bool contained(Predicate *A, Predicate *B) {

    if (A->type_ != B->type_)
    {
        return false;
    }

    if (A->type_ == Predicate::NP) {
        return A->u.np_->is_not_ == B->u.np_->is_not_ &&
                is_same_ref(A->u.np_->row_->u.column_ref_, B->u.np_->row_->u.column_ref_);
    }
    ComparisonPredicate *cA = to_comparison_predicate(A);
    ComparisonPredicate *cB = to_comparison_predicate(B);

    RowValueConstructor *lA = cA->row1_;
    RowValueConstructor *rA = cA->row2_;
    RowValueConstructor *lB = cB->row1_;
    RowValueConstructor *rB = cB->row2_;


    /* l is REF, r is LIT */
    if (!is_same_ref(lA->u.column_ref_, lB->u.column_ref_)) {
        return false;
    }


    /* EQ, NEQ, LT, GT, LTEQ, GTEQ  */
    /* add predicate optimize rule here */
    switch (cA->comp_op_) {
        case ComparisonPredicate::EQ: {
            switch (cB->comp_op_) {
                case ComparisonPredicate::EQ: {
                    return is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::NEQ: {
                    return !is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::LT: {
                    return is_lt_lit(rA, rB);
                } break;
                case ComparisonPredicate::GT: {
                    return is_gt_lit(rA, rB);
                } break;
                case ComparisonPredicate::LTEQ: {
                    return is_lt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::GTEQ: {
                    return is_gt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                default: assert(false);
            }
        } break;
        case ComparisonPredicate::NEQ: {
            switch (cB->comp_op_) {
                case ComparisonPredicate::EQ: {
                    return !is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::NEQ: {
                    return is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::LT: {
                    return false;
                } break;
                case ComparisonPredicate::GT: {
                    return false;
                } break;
                case ComparisonPredicate::LTEQ: {
                    return false;
                } break;
                case ComparisonPredicate::GTEQ: {
                    return false;
                } break;
                default: assert(false);
            }
        } break;
        case ComparisonPredicate::LT: {
            switch (cB->comp_op_) {
                case ComparisonPredicate::EQ: {
                    return false;
                } break;
                case ComparisonPredicate::NEQ: {
                    return is_lt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::LT: {
                    return is_lt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::GT: {
                    return false;
                } break;
                case ComparisonPredicate::LTEQ: {
                    return is_lt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::GTEQ: {
                    return false;
                } break;
                default: assert(false);
            }
        } break;
        case ComparisonPredicate::GT: {
            switch (cB->comp_op_) {
                case ComparisonPredicate::EQ: {
                    return false;
                } break;
                case ComparisonPredicate::NEQ: {
                    return is_gt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::LT: {
                    return false;
                } break;
                case ComparisonPredicate::GT: {
                    return is_gt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::LTEQ: {
                    return false;
                } break;
                case ComparisonPredicate::GTEQ: {
                    return is_lt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                default: assert(false);
            }
        } break;
        case ComparisonPredicate::LTEQ: {
            switch (cB->comp_op_) {
                case ComparisonPredicate::EQ: {
                    return false;
                } break;
                case ComparisonPredicate::NEQ: {
                    return is_lt_lit(rA, rB);
                } break;
                case ComparisonPredicate::LT: {
                    return is_lt_lit(rA, rB);
                } break;
                case ComparisonPredicate::GT: {
                    return false;
                } break;
                case ComparisonPredicate::LTEQ: {
                    return is_lt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::GTEQ: {
                    return false;
                } break;
                default: assert(false);
            }
        } break;
        case ComparisonPredicate::GTEQ: {
            switch (cB->comp_op_) {
                case ComparisonPredicate::EQ: {
                    return false;
                } break;
                case ComparisonPredicate::NEQ: {
                    return is_gt_lit(rA, rB);
                } break;
                case ComparisonPredicate::LT: {
                    return false;
                } break;
                case ComparisonPredicate::GT: {
                    return is_gt_lit(rA, rB);
                } break;
                case ComparisonPredicate::LTEQ: {
                    return false;
                } break;
                case ComparisonPredicate::GTEQ: {
                    return is_lt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                default: assert(false);
            }
        } break;
        default: assert(false);
    }
    return false;
}

bool mutexed(Predicate *A, Predicate *B) {
    if (A->type_ != B->type_)
        return false;
    if (A->type_ == Predicate::NP) {
        return false;
    }
    ComparisonPredicate *cA = to_comparison_predicate(A);
    ComparisonPredicate *cB = to_comparison_predicate(B);

    RowValueConstructor *lA = cA->row1_;
    RowValueConstructor *rA = cA->row2_;
    RowValueConstructor *lB = cB->row1_;
    RowValueConstructor *rB = cB->row2_;


    /* l is REF, r is LIT */
    if (!is_same_ref(lA->u.column_ref_, lB->u.column_ref_))
        return false;

    /* EQ, NEQ, LT, GT, LTEQ, GTEQ  */
    /* add predicate optimize rule here */
    switch (cA->comp_op_) {
        case ComparisonPredicate::EQ: {
            switch (cB->comp_op_) {
                case ComparisonPredicate::EQ: {
                    return !is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::NEQ: {
                    return is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::LT: {
                    return is_gt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::GT: {
                    return is_lt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::LTEQ: {
                    return is_gt_lit(rA, rB);
                } break;
                case ComparisonPredicate::GTEQ: {
                    return is_lt_lit(rA, rB);
                } break;
                default: assert(false);
            }
        } break;
        case ComparisonPredicate::NEQ: {
            switch (cB->comp_op_) {
                case ComparisonPredicate::EQ: {
                    return is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::NEQ: {
                    return !is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::LT: {
                    return false;
                } break;
                case ComparisonPredicate::GT: {
                    return false;
                } break;
                case ComparisonPredicate::LTEQ: {
                    return false;
                } break;
                case ComparisonPredicate::GTEQ: {
                    return false;
                } break;
                default: assert(false);
            }
        } break;
        case ComparisonPredicate::LT: {
            switch (cB->comp_op_) {
                case ComparisonPredicate::EQ: {
                    return is_lt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::NEQ: {
                    return false;
                } break;
                case ComparisonPredicate::LT: {
                    return false;
                } break;
                case ComparisonPredicate::GT: {
                    return is_lt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::LTEQ: {
                    return false;
                } break;
                case ComparisonPredicate::GTEQ: {
                    return is_lt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                default: assert(false);
            }
        } break;
        case ComparisonPredicate::GT: {
            switch (cB->comp_op_) {
                case ComparisonPredicate::EQ: {
                    return is_gt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::NEQ: {
                    return false;
                } break;
                case ComparisonPredicate::LT: {
                    return is_gt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::GT: {
                    return false;
                } break;
                case ComparisonPredicate::LTEQ: {
                    return is_gt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::GTEQ: {
                    return false;
                } break;
                default: assert(false);
            }
        } break;
        case ComparisonPredicate::LTEQ: {
            switch (cB->comp_op_) {
                case ComparisonPredicate::EQ: {
                    return is_lt_lit(rA, rB);
                } break;
                case ComparisonPredicate::NEQ: {
                    return false;
                } break;
                case ComparisonPredicate::LT: {
                    return false;
                } break;
                case ComparisonPredicate::GT: {
                    return is_lt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::LTEQ: {
                    return false;
                } break;
                case ComparisonPredicate::GTEQ: {
                    return is_lt_lit(rA, rB);
                } break;
                default: assert(false);
            }
        } break;
        case ComparisonPredicate::GTEQ: {
            switch (cB->comp_op_) {
                case ComparisonPredicate::EQ: {
                    return is_gt_lit(rA, rB);
                } break;
                case ComparisonPredicate::NEQ: {
                    return false;
                } break;
                case ComparisonPredicate::LT: {
                    return is_gt_lit(rA, rB) || is_same_lit(rA, rB);
                } break;
                case ComparisonPredicate::GT: {
                    return false;
                } break;
                case ComparisonPredicate::LTEQ: {
                    return is_gt_lit(rA, rB);
                } break;
                case ComparisonPredicate::GTEQ: {
                    return false;
                } break;
                default: assert(false);
            }
        } break;
        default: assert(false);
    }
    return false;
}

void format(Predicate *A, Buf *dst) {
    switch (A->type_) {
        case Predicate::NP: {
            NullPredicate *np = A->u.np_;
            assert(np->row_->type_ == RowValueConstructor::COLUMN_REF);
            format(np->row_, dst);
            if (np->is_not_) {
                dst->append(" IS NOT NULL");
            }
            else {
                dst->append(" IS NULL");
            }
        } break;
        case Predicate::CP: {
            ComparisonPredicate *cp = A->u.cp_;
            format(cp->row1_, dst);
            /* EQ, NEQ, LT, GT, LTEQ, GTEQ */
            switch (cp->comp_op_) {
                case ComparisonPredicate::EQ: {
                    dst->append(" = ");
                } break;
                case ComparisonPredicate::NEQ: {
                    dst->append(" <> ");
                } break;
                case ComparisonPredicate::LT: {
                    dst->append(" < ");
                } break;
                case ComparisonPredicate::GT: {
                    dst->append(" > ");
                } break;
                case ComparisonPredicate::LTEQ: {
                    dst->append(" <= ");
                } break;
                case ComparisonPredicate::GTEQ: {
                    dst->append(" >= ");
                } break;
                default:
                    assert(false);
            }
            format(cp->row2_, dst);
        } break;
        default: assert(false);
    }
}

