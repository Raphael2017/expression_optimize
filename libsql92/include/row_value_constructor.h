#ifndef ROW_VALUE_CONSTRUCTOR
#define ROW_VALUE_CONSTRUCTOR

struct ILex;
struct ParseResult;
struct RowValueConstructor;
struct RowValueConstructorList;
struct QueryExpression;
struct Buf;

struct Id {
    const char *identifier_;
    const char *word_;
};

struct StringLiteral {
    const char *content_;
    const char *word_;
};

struct IdentifierChain {
    Id *identifier_;
    IdentifierChain *next_;
};

struct RowValueConstructor {
    enum { COLUMN_REF, INT_LIT, STR_LIT } type_;
    union {
        int int_lit_;
        StringLiteral *str_lit_;
        IdentifierChain *column_ref_;
    } u;
};

struct RowValueConstructorList {
    RowValueConstructor *row_;
    RowValueConstructorList *next_;
};

RowValueConstructor *make_row_value_ctor(ILex *lex, ParseResult *pr);
void free_row_value_ctor(RowValueConstructor *row);

RowValueConstructorList *make_row_value_ctor_list(RowValueConstructor *row, RowValueConstructorList *next);


bool is_subquery(RowValueConstructor *row);
QueryExpression *row_value_ctor_to_query_expression(RowValueConstructor *row);

void format(RowValueConstructor *A, Buf *dst);

bool is_same_ref(IdentifierChain *ref1, IdentifierChain *ref2);

bool is_same_lit(RowValueConstructor *lit1, RowValueConstructor *lit2);

bool is_lt_lit(RowValueConstructor *lit1, RowValueConstructor *lit2);

bool is_gt_lit(RowValueConstructor *lit1, RowValueConstructor *lit2);

#endif
