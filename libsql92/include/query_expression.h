#ifndef QUERY_EXPRESSION_H
#define QUERY_EXPRESSION_H

struct ILex;
struct ParseResult;
struct QueryExpression;

QueryExpression *make_query_expression(ILex *lex, ParseResult *pr);

#endif