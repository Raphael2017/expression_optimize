#ifndef COMPILER_UTILS_H
#define COMPILER_UTILS_H

#include <string>
#include <assert.h>

struct utils
{
    static uint64_t hash(const std::string& str)
    {
        unsigned int ret = 0;
        for (auto c : str)
        {
            ret += ret*65599 + c;
        }
        return ret;
    }
    static uint64_t hash(void* key)
    {
        return (uint64_t)key;
    }
};

#endif