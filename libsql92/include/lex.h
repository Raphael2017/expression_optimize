#ifndef LEX_H
#define LEX_H

enum TokenType {
    none,
    CARET,
    PERCENT,
    PLUS,
    MINUS,
    STAR,
    DIVIDE,
    DOT,
    LPAREN,
    RPAREN,
    COMMA,
    SEMI,
    QUES,
    EQ,
    GT,
    LT,
    BARBAR,
    GTEQ,
    LTEQ,
    LTGT,
    STR_LITERAL,
    ID,
    NUMBER,

    ALL, AND, ANY, AS, ASC, AVG,
    BETWEEN, BY,
    CASE, CAST, COLLATE, CONVERT, COUNT, CROSS,
    DEFAULT, DELETE, DESC, DISTINCT,
    ELSE, END, ESCAPE, EXCEPT, EXISTS,
    FALSE, FROM, FULL,
    GROUP, GROUPING,
    HAVING,
    IN, INNER, INSERT, INTERSECT, INTERVAL, INTO, IS,
    JOIN,
    LEFT, LIKE,
    MATCH, MAX, MIN,
    NATURAL, NOT, NULLIF, NULLX,
    ON, OR, ORDER, OUTER, OVER, OVERLAPS,
    PARTIAL,
    RANK, RIGHT,
    SELECT, SET, SOME, SUM,
    THEN, TO, TRUE,
    UNION, UNIQUE, UNKNOWN, UPDATE,
    VALUES,
    WHEN, WHERE, WITH,



    ERR,
    END_P
};

struct IToken {
    virtual ~IToken() {}
    virtual TokenType type() const  = 0;
    virtual const char *word() const = 0;
};

struct ILex {
    virtual ~ILex() {}
    virtual IToken *token() = 0;
    virtual void next() = 0;
    virtual unsigned int cur_pos_line() const = 0;
    virtual unsigned int cur_pos_col() const = 0;
    virtual unsigned int cur_pos() const = 0;
};

ILex *make_lex(const char *sql);
void free_lex(ILex *lex);

#endif