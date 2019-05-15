#ifndef PREDICATE_H
#define PREDICATE_H

struct ILex;
struct ParseResult;
struct QueryExpression;
struct RowValueConstructor;
struct RowValueConstructorList;
struct ComparisonPredicate;
struct BetweenPredicate;
struct InPredicate;
struct LikePredicate;
struct NullPredicate;
struct QuantifiedComparisonPredicate;
struct ExistsPredicate;
struct MatchPredicate;
struct OverlapsPredicate;
struct Predicate;
struct Buf;

/*
 * <comparison predicate> ::= <row value constructor> <comp op> <row value constructor>
 * */
struct ComparisonPredicate {
    RowValueConstructor *row1_;
    RowValueConstructor *row2_;
    enum { EQ, NEQ, LT, GT, LTEQ, GTEQ } comp_op_;
};

/*
 * <between predicate> ::= <row value constructor> [ NOT ] BETWEEN <row value constructor> AND <row value constructor>
 * */
struct BetweenPredicate {
    RowValueConstructor *row1_;
    RowValueConstructor *row2_;
    RowValueConstructor *row3_;
    bool is_not_;
};

/*
 * <in predicate> ::= <row value constructor> [ NOT ] IN <in predicate value>
 * */
struct InPredicate {
    RowValueConstructor *row_;
    enum { SUBQUERY, VALUE_LIST } type_;
    union {
        QueryExpression *subquery_;
        RowValueConstructorList *value_list_;
    } u;
    bool is_not_;
};

/*
 * <like predicate> ::= <match value> [ NOT ] LIKE <pattern> [ ESCAPE <escape character> ]
 *
 * */
struct LikePredicate {
    RowValueConstructor *match_value_;
    RowValueConstructor *pattern_;
    bool is_not_;
    RowValueConstructor *escape_;
};

/*
 * <null predicate> ::= <row value constructor> IS [ NOT ] NULL
 * */
struct NullPredicate {
    RowValueConstructor *row_;
    bool is_not_;
};

/*
 * <quantified comparison predicate> ::= <row value constructor> <comp op> <quantifier> <table subquery>
 * */
struct QuantifiedComparisonPredicate {
    RowValueConstructor *row_;
    QueryExpression *subquery_;
    enum { EQ, NEQ, LT, GT, LTEQ, GTEQ } comp_op_;
    enum { ALL, SOME } quantifier_;
};

/*
 * <exists predicate> ::= EXISTS <table subquery>
 * */
struct ExistsPredicate {
    QueryExpression *subquery_;
};

/*
 * <match predicate> ::= <row value constructor> MATCH [ UNIQUE ] [ PARTIAL | FULL ] <table subquery>
 * */
struct MatchPredicate {
    RowValueConstructor *row_;
    QueryExpression *subquery_;
    bool is_unique_;
    enum { PARTIAL, FULL, NIL} opt_;
};

/*
 * <overlaps predicate> ::= <row value constructor 1> OVERLAPS <row value constructor 2>
 * */
struct OverlapsPredicate {
    RowValueConstructor *row1_;
    RowValueConstructor *row2_;
};

/*
 * https://github.com/Raphael2017/SQL/blob/master/sql-92.bnf
 * line 672
 * */

struct Predicate {
    enum { CP, BP, IP, LP, NP, QCP, EP, MP, OP, RW, NIL } type_;
    union {
        ComparisonPredicate *cp_;
        BetweenPredicate *bp_;
        InPredicate *ip_;
        LikePredicate *lp_;
        NullPredicate *np_;
        QuantifiedComparisonPredicate *qcp_;
        ExistsPredicate *ep_;
        MatchPredicate *mp_;
        OverlapsPredicate *op_;
        RowValueConstructor *row_;
    } u;
};

Predicate *make_predicate(ILex *lex, ParseResult *pr);

/* contains */
/* check A contained in B */
bool contained(Predicate *A, Predicate *B);

void format(Predicate *A, Buf *dst);

#endif