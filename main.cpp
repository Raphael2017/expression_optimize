#include <iostream>
#include <fstream>
#include <iostream>
#include <assert.h>
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
    std::string sql = readFileContents("/home/qwerty/CLionProjects/expression_optimize/LL/expression_large.txt");
    clock_t start, finish;
    double duration;
    {
        start = clock();
        fun_(sql);
        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        printf( "%f seconds\n", duration );
    }

}

int fun_(const std::string& sql) {
    std::string new_sql = "";
    bool f = format_search_condition(sql, new_sql, true, true);
    if (f)
        printf("new condition:\n %s\n", new_sql.c_str());
    else
        printf("%s\n", new_sql.c_str());
    return 0;
}