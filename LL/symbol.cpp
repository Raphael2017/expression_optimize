#include <stdio.h>
#include "symbol.h"
#include "table.h"

#include "utils.h"



S_table S_empty(void)
{
	return TAB_empty();
}

void S_enter(S_table t, S_symbol sym, void *value)
{
	TAB_enter(t, sym, value);
}

void *S_look(S_table t, S_symbol sym)
{
	return TAB_look(t, sym);
}

static struct Symbol marksym = { "<mark>", 0 };

void S_beginScope(S_table t)
{
	S_enter(t, &marksym, NULL);
}

void S_endScope(S_table t, void(*f)(void*value))
{
	void* sym;
	void* value = nullptr;
	do
	{
		TAB_pop(t, &sym, &value);
		if (f) f(value);
	}
	while (sym != &marksym);
}

void S_dump(S_table t, void (*show) (S_symbol sym, void *binding))
{
	TAB_dump(t, (void (*)(void *, void *))show);
}


Symbol* make_symbol(const char* str) {
    static const size_t syms_hash_tbl_size = 109;
    static Symbol *syms_hash_tbl[syms_hash_tbl_size] = { 0 };

    uint64_t hash = utils::hash(str);
    size_t index = hash % syms_hash_tbl_size;


    for (Symbol* sym = syms_hash_tbl[index]; sym; sym = sym->next_)
        if (sym->symbol_ == str)
            return sym;

    Symbol *ret = new Symbol;
    ret->symbol_ = str;
    ret->next_ = syms_hash_tbl[index];
    syms_hash_tbl[index] = ret;
    return ret;
}
