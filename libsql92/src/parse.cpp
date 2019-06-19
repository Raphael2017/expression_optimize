#include "parse.h"
#include "search_condition.h"
#include "predicate.h"
#include "row_value_constructor.h"
#include "lex.h"
#include "dynamic_programming_contained.h"
#include "hash_map_double_key.h"

void ParseResult::fill_error(int err, const std::string& detail) {
    error_ = err;
    detail_ = detail;
}

void Buf::append(const char *s) {
    str_ << s;
}
void Buf::append(int i) {
    str_ << std::to_string(i);
}
std::string Buf::str() const { return str_.str(); }

bool error_occur(ParseResult *pr) { return pr->error_ != PARSE_SUCCESS; }

bool format_search_condition(const std::string& src, std::string& dst, bool optimize_flag, bool surround_parens_flag) {
    ILex *lex = make_lex(src.c_str());
    lex->next();
    ParseResult pr;
    pr.error_ = PARSE_SUCCESS;

    SearchCondition *s = make_search_condition(lex, &pr);
    if (error_occur(&pr)) {
        dst = pr.detail_;
        free_lex(lex);
        return false;
    }

    if (optimize_flag) {
#if 1
        HashMapDoubleKey *m = make_hashmap_double_key();
        proc_contained(m, s);
        optimize_d(s, m);
#else
        optimize(s);
#endif
    }

    Buf buf;
    format(s, &buf);

    if (surround_parens_flag && check_search_condition(sc(s)))
        dst = "(" + buf.str() + ")";
    else
        dst = buf.str();
    free_search_condition(s);
    s = nullptr;
    free_lex(lex);
    lex = nullptr;
    auto c = LogicTbl::Ins();
    int a = c->LLL_;

    return true;
}