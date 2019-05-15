#ifndef ROW_VALUE_CONSTRUCTOR
#define ROW_VALUE_CONSTRUCTOR

struct ILex;
struct ParseResult;
struct RowValueConstructor;
struct RowValueConstructorList;
struct QueryExpression;
struct Buf;

struct IdentifierChain {
    const char *identifier_;
    IdentifierChain *next_;
};

struct RowValueConstructor {
    enum { COLUMN_REF, INT_LIT, STR_LIT } type_;
    union {
        int int_lit_;
        const char *str_lit_;
        IdentifierChain *column_ref_;
    } u;
};

struct RowValueConstructorList {
    RowValueConstructor *row_;
    RowValueConstructorList *next_;
};

RowValueConstructor *make_row_value_ctor(ILex *lex, ParseResult *pr);
RowValueConstructorList *make_row_value_ctor_list(RowValueConstructor *row, RowValueConstructorList *next);


bool is_subquery(RowValueConstructor *row);
QueryExpression *row_value_ctor_to_query_expression(RowValueConstructor *row);

void format(RowValueConstructor *A, Buf *dst);

#endif
