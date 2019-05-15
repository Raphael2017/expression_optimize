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

int main() {

    std::string sql = readFileContents("/home/uiop/CLionProjects/sql92/LL/expression3.txt");
    ILex *lex = make_lex(sql.c_str());
    lex->next();
    ParseResult pr;
    pr.error_ = PARSE_SUCCESS;

    SearchCondition * s = make_search_condition(lex, &pr);
    optimize(s);
    Buf buf;
    format(s, &buf);
    printf("%s", buf.str_.c_str());
    return 0;
}