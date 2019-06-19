#include "hash_map_double_key.h"

#define TABSIZE 293

struct V_list {
    bool v_;
    void *keyi_;
    void *keyj_;
    V_list *next_;
};

V_list *binder(void *keyi, void *keyj, bool v, V_list *t) {
    V_list *r = new V_list;
    r->keyi_ = keyi; r->keyj_ = keyj; r->v_ = v; r->next_ = t;
    return r;
}

struct HashMapDoubleKey {
    V_list* data_[TABSIZE][TABSIZE] = { nullptr };
};

HashMapDoubleKey* make_hashmap_double_key() {
    HashMapDoubleKey *ret = new HashMapDoubleKey;
    return ret;
}

void set_hashmap_double_key(HashMapDoubleKey* m, void* keyi, void* keyj, bool v) {
    int i = (long long)keyi % TABSIZE;
    int j = (long long)keyj % TABSIZE;
    m->data_[i][j] = binder(keyi, keyj, v, m->data_[i][j]);
}
bool get_hashmap_double_key(HashMapDoubleKey* m, void* keyi, void* keyj, bool *v) {
    if (keyi == keyj) {
        *v = true;
        return true;
    }

    int i = (long long)keyi % TABSIZE;
    int j = (long long)keyj % TABSIZE;
    for (V_list *it = m->data_[i][j]; it != nullptr; it = it->next_) {
        if (keyi == it->keyi_ && keyj == it->keyj_) {
            *v = it->v_;
            return true;
        }
    }
    return false;
}