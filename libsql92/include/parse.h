#ifndef PARSE_H
#define PARSE_H

#include <sstream>
#include <string>
#include <map>
#include <set>

const int PARSE_FAILED = -1;
const int PARSE_SUCCESS = 0;

struct Ret {
    void *p1;
    void *p2;
    bool ret;
    Ret *next_;
};

struct LogicTbl {
    static LogicTbl *Ins() {
        static LogicTbl gg;
        return &gg;
    }

    LogicTbl() : contained_result_map_{ 0 } {}

    bool contained(void* p1, void* p2, bool& ret) {
        uint64_t a1 = (uint64_t)p1, a2 = (uint64_t)p2;
        a1 = a1 % 97; a2 = a2 % 97;
        Ret *r = contained_result_map_[a1][a2];
        for (Ret *it = r; it != nullptr; it = it->next_) {
            if (it->p1 == p1 && it->p2 == p2)
                return it->ret;
        }
        return false;
    }
    bool mutexed(void* p1, void* p2, bool& ret) {
        char key[64] = { 0 };
        sprintf(key, "%x%x", p1, p2);
        auto f = mutexed_result_map_.find(key);
        if (f != mutexed_result_map_.end()) {
            ret = f->second;
            return true;
        }
        else
            return false;
    }

    void update_contained(void* p1, void* p2, bool result) {
        uint64_t a1 = (uint64_t)p1, a2 = (uint64_t)p2;
        a1 = a1 % 97; a2 = a2 % 97;


        Ret *r = contained_result_map_[a1][a2];
        Ret *new_r = new Ret;
        new_r->p1 = p1; new_r->p2 = p2; new_r->ret = result; new_r->next_ = r;
        contained_result_map_[a1][a2] = new_r;
        LLL_++;
    }

    void update_mutexed(void* p1, void* p2, bool result) {
        char key[64] = { 0 };
        sprintf(key, "%x%x", p1, p2);
        mutexed_result_map_[key] = result;
    }

    Ret *contained_result_map_[100][100];
    std::map<std::string, bool> mutexed_result_map_;

    int LLL_ = 0;
};

struct ParseResult {
    int error_;
    std::string detail_;
    void fill_error(int err, const std::string& detail);
};

struct Buf {
    void append(const char *s);
    void append(int i);
    std::string str() const;
    std::stringstream str_;
};

bool error_occur(ParseResult *pr);

/* return true dst will be new condition, false dst will be error detail */
bool format_search_condition(const std::string& src, std::string& dst, bool optimize_flag, bool surround_parens_flag);

#endif