#ifndef LL_H
#define LL_H

struct Symbol;
struct SymbolList;
struct Rule;
void def_syn_symbol(Symbol *sym, bool is_terminal);
Rule *make_rule(Symbol *target, int component_cnt, Symbol **coms);
void bind_rule(Symbol *sym, Rule *rule);
void proc();


#endif