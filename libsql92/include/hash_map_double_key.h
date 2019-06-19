#ifndef HASH_MAP_DOUBLE_KEY
#define HASH_MAP_DOUBLE_KEY

struct HashMapDoubleKey;

HashMapDoubleKey *make_hashmap_double_key();
void set_hashmap_double_key(HashMapDoubleKey*, void*, void*, bool);
bool get_hashmap_double_key(HashMapDoubleKey*, void*, void*, bool *v);

#endif /* HASH_MAP_DOUBLE_KEY */