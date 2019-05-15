#ifndef SEARCH_CONDITION_H
#define SEARCH_CONDITION_H

struct ParseResult;
struct ILex;
struct Predicate;
struct BooleanPrimary;
struct BooleanTest;
struct BooleanFactor;
struct BooleanTerm;
struct BooleanFactorList;
struct BooleanTermList;
struct SearchCondition;
struct Buf;

BooleanPrimary *make_boolean_primary(ILex *lex, ParseResult *pr);
BooleanTest *make_boolean_test(ILex *lex, ParseResult *pr);
BooleanFactor *make_boolean_factor(ILex *lex, ParseResult *pr);
BooleanTerm *make_boolean_term(ILex *lex, ParseResult *pr);
SearchCondition *make_search_condition(ILex *lex, ParseResult *pr);

/*
 * <boolean primary> ::= <predicate> | <left paren> <search condition> <right paren>
 * ignore <left paren> <search condition> <right paren>
 * <boolean primary> ::= <predicate>
 * */
struct BooleanPrimary {
    enum { PRE, SEARCH } type_;
    union {
        Predicate *predicate_;
        SearchCondition *search_;
    } u;
};

/*
 * <boolean test> ::= <boolean primary> [ IS [ NOT ] <truth value> ]
 * */
struct BooleanTest {
    BooleanPrimary *primary_;
    enum { NIL, TRUE, FALSE, UNKNOWN } truth_value_;     /* NIL means only primary */
    bool is_;                                           /* truth_value_ is not null, is_ true means IS */
};

/*
 * <boolean factor> ::= NOT <boolean factor> | <boolean test>
 * */
struct BooleanFactor {
    bool is_not_;
    union {
        BooleanFactor *factor_;
        BooleanTest *test_;
    } u;
};

struct BooleanFactorList {
    BooleanFactor *factor_;
    BooleanFactorList *next_;
    bool is_optimized_;
};

/*
 * <boolean term> ::=
 *          <boolean factor>
 *        | <boolean term> AND <boolean factor>
 * */
struct BooleanTerm {
    BooleanFactorList *boolean_factors_;
};

struct BooleanTermList {
    BooleanTerm *term_;
    BooleanTermList *next_;
    bool is_optimized_;
};

/*
 * <search condition> ::=
 *	      <boolean term>
 *      | <search condition> OR <boolean term>
 * */
struct SearchCondition {
    BooleanTermList *boolean_terms_;
};


SearchCondition *make_search_condition(ILex *lex, ParseResult *pr);

/* contains */
/* check A contained in B */
//bool contained(Predicate *A, BooleanTerm *B);
bool contained(Predicate *A, SearchCondition *B);
bool contained(BooleanTerm *A, Predicate *B);
bool contained(BooleanTerm *A, BooleanTerm *B);
bool contained(BooleanTerm *A, SearchCondition *B);
bool contained(SearchCondition *A, Predicate *B);
bool contained(SearchCondition *A, BooleanTerm *B);
bool contained(SearchCondition *A, SearchCondition *B);

void optimize(SearchCondition *A);
void optimize(BooleanTerm *A);

void format(SearchCondition *A, Buf *dst);
void format(BooleanTerm *A, Buf *dst);
void format(BooleanFactor *A, Buf *dst);

#endif