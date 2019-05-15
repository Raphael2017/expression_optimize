/*
 * symbol.h - Symbols and symbol-tables
 *
 */
#ifndef TIGER_SYMBOL_H_
#define TIGER_SYMBOL_H_

#include "table.h"
#include <string>

typedef struct Symbol *S_symbol;

struct Symbol {
    std::string symbol_;
    Symbol *next_;
};

/* S_table is a mapping from S_symbol->any, where "any" is represented
 *     here by void*  */
typedef struct TAB_table_ *S_table;

/* Make a new table */
S_table S_empty(void);

/* Enter a binding "sym->value" into "t", shadowing but not deleting
 *    any previous binding of "sym". */
void S_enter(S_table t, S_symbol sym, void *value);

/* Look up the most recent binding of "sym" in "t", or return NULL
 *    if sym is unbound. */
void *S_look(S_table t, S_symbol sym);

/* Start a new "scope" in "t".  Scopes are nested. */
void S_beginScope(S_table t);

/* Remove any bindings entered since the current scope began,
   and end the current scope. */
void S_endScope(S_table t, void(*f)(void*value));

Symbol* make_symbol(const char* str);

#endif /* TIGER_SYMBOL_H_ */
