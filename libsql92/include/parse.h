#ifndef PARSE_H
#define PARSE_H

#include <string>

const int PARSE_FAILED = -1;
const int PARSE_SUCCESS = 0;


struct ParseResult {
    int error_;
};

struct Buf {
    void append(const char *s) {
        str_ += s;
    }
    void append(int i) {
        str_ += std::to_string(i);
    }

    std::string str_;
};

bool error_occur(ParseResult *pr);

#endif