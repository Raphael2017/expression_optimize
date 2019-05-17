#include <iostream>
#include <fstream>
#include <iostream>
#include <assert.h>
#include "search_condition.h"
#include "predicate.h"
#include "row_value_constructor.h"
#include "lex.h"
#include "parse.h"

std::string readFileContents(const std::string& file_path)
{
    std::ifstream t(file_path);
    std::string text((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
    return text;
}

int fun_(const std::string& sql);
int main() {
    std::string sql = readFileContents("/home/qwerty/CLionProjects/expression_optimize/LL/expression1.txt");
    clock_t start, finish;
    double duration;
    {
        start = clock();
        fun_(sql);
        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        printf( "%f seconds\n", duration );
    }

};

int fun_(const std::string& sql) {


    ILex *lex = make_lex(sql.c_str());
    lex->next();
    ParseResult pr;
    pr.error_ = PARSE_SUCCESS;

    SearchCondition * s = make_search_condition(lex, &pr);
    if (error_occur(&pr)) {
        free_lex(lex);
        return 0;
    }
    optimize(s);
    Buf buf;
    format(s, &buf);
    free_search_condition(s);
    s = nullptr;
    free_lex(lex);
    lex = nullptr;
    printf("%s\n", buf.str_.c_str());
    return 0;
}