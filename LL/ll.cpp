#include "ll.h"
#include "symbol.h"
#include <assert.h>
#include <set>
#include <map>

struct SynSymbol;
S_table g_syns = S_empty();

std::set<SynSymbol*> g_terms;
std::set<SynSymbol*> g_non_terms;
std::set<SynSymbol*> g_nullable_terms;




struct Rule {
    SynSymbol *target;
    int component_cnt_;
    SynSymbol **components_;
};

struct RuleList {
    Rule *rule_;
    RuleList *next_;
};

struct SynSymbol {
    Symbol *sym;
    RuleList *rules_;
    bool is_terminal_;
    bool is_nullable_;
    std::set<SynSymbol*> first_;
    std::set<SynSymbol*> follow_;
};

int non_terms_first_set_cnt() {
    int cnt = 0;
    for (SynSymbol *it : g_non_terms)
        cnt += it->first_.size();
    return cnt;
}

int terms_follow_set_cnt() {
    int cnt = 0;
    for (SynSymbol *it : g_terms)
        cnt += it->follow_.size();
    for (SynSymbol *it : g_non_terms)
        cnt += it->follow_.size();
    return cnt;
};

void print_first() {
    for (SynSymbol *it : g_terms) {
        assert(it->first_.size() == 1);
        printf("%s <first> %s,\n", it->sym->symbol_.c_str(), (*it->first_.begin())->sym->symbol_.c_str());
    }

    for (SynSymbol *it : g_non_terms) {
        printf("%s <first> ", it->sym->symbol_.c_str());
        for (auto com : it->first_) {
            printf("%s, ", com->sym->symbol_.c_str());
        }
        printf("\n");
    }
}

void print_follow() {
    for (SynSymbol *it : g_terms) {
        printf("%s <follow> ", it->sym->symbol_.c_str());
        for (auto com : it->follow_) {
            printf("%s, ", com->sym->symbol_.c_str());
        }
        printf("\n");
    }

    for (SynSymbol *it : g_non_terms) {
        printf("%s <follow> ", it->sym->symbol_.c_str());
        for (auto com : it->follow_) {
            printf("%s, ", com->sym->symbol_.c_str());
        }
        printf("\n");
    }
}

void print_rule(Rule *rule) {
    printf("<%s> -> ", rule->target->sym->symbol_.c_str());
    for (int i = 0; i < rule->component_cnt_; ++i) {
        printf("<%s> ", rule->components_[i]->sym->symbol_.c_str());
    }
}

bool check_all_nullable(SynSymbol **begin, SynSymbol **end) {
    bool ret = true;
    for (SynSymbol **it = begin; it < end; ++it) {
        if (!(*it)->is_nullable_) {
            ret = false;
            break;
        }
    }
    return ret;
}

std::set<SynSymbol*> first_of_strand(SynSymbol **begin, SynSymbol **end) {
    assert(begin && end);
    if (begin >= end)
        return {};
    if (!(*begin)->is_nullable_)
        return (*begin)->first_;
    else {
        std::set<SynSymbol*> ret = (*begin)->first_;
        auto c = first_of_strand(begin+1, end);
        ret.insert(c.begin(), c.end());
        return ret;
    }
}

bool nullable_of_strand(SynSymbol **begin, SynSymbol **end) {
    return check_all_nullable(begin, end);
}


void def_syn_symbol(Symbol *sym, bool is_terminal) {
    SynSymbol *syn = new SynSymbol;
    syn->sym = sym;
    syn->rules_ = nullptr;
    syn->is_terminal_ = is_terminal;
    syn->is_nullable_ = false;
    if (is_terminal)
        g_terms.insert(syn);
    else
        g_non_terms.insert(syn);
    S_enter(g_syns, sym, syn);
}

void bind_rule(Symbol *sym, Rule *rule) {
    SynSymbol *syn = (SynSymbol*)S_look(g_syns, sym);
    assert(syn);
    RuleList *head = new RuleList;
    head->rule_ = rule;
    head->next_ = syn->rules_;
    syn->rules_ = head;
}

void proc_nullable() {
    g_nullable_terms.clear();
    std::set<SynSymbol*> add;
    int s1 = 0;
    do {
        s1 = g_nullable_terms.size();
        for (SynSymbol *syn : g_non_terms) {
            for (RuleList *rule = syn->rules_; rule; rule = rule->next_) {
                if (!rule->rule_) {
                    add.insert(syn);
                    syn->is_nullable_ = true;
                }
                else {
                    bool all_nullable = true;
                    for (int i = 0; i < rule->rule_->component_cnt_; ++i)
                        if (!rule->rule_->components_[i]->is_nullable_) {
                            all_nullable = false;
                            break;
                        }
                    if (all_nullable) {
                        add.insert(syn);
                        syn->is_nullable_ = true;
                    }

                }
            }
        }
        g_nullable_terms.insert(add.begin(), add.end());
    } while (s1 != g_nullable_terms.size());

    auto for_debug = g_nullable_terms;

    return;
}

void proc_first() {
    for (auto it : g_terms)
        it->first_.insert(it);
    for (auto it : g_non_terms)
        it->first_.clear();

    int s1 = 0;
    do {
        s1 = non_terms_first_set_cnt();
        for (SynSymbol *syn : g_non_terms) {
            for (RuleList *rule = syn->rules_; rule; rule = rule->next_) {
                if (!rule->rule_) {

                }
                else {
                    bool flag = true;
                    for (int i = 0; i < rule->rule_->component_cnt_; ++i) {
                        SynSymbol *h = rule->rule_->components_[i];
                        if (flag) {
                            syn->first_.insert(h->first_.begin(), h->first_.end());
                        }
                        else
                            break;
                        flag = h->is_nullable_;
                    }

                }
            }
        }
    }
    while (s1 < non_terms_first_set_cnt());
    auto nu = g_non_terms;
    return;
}

void proc_follow() {
    for (auto it : g_terms)
        it->follow_.clear();
    for (auto it : g_non_terms)
        it->follow_.clear();

    int s1 = 0;
    do {
        s1 = terms_follow_set_cnt();
        for (SynSymbol *syn : g_non_terms) {
            for (RuleList *rule = syn->rules_; rule; rule = rule->next_) {
                if (!rule->rule_) {

                }
                else {
                    for (int i = 0; i < rule->rule_->component_cnt_; ++i) {
                        if (check_all_nullable(rule->rule_->components_+i+1, rule->rule_->components_+rule->rule_->component_cnt_)) {
                            SynSymbol *YI = rule->rule_->components_[i];
                            YI->follow_.insert(syn->follow_.begin(), syn->follow_.end());
                        }
                        for (int j = i + 1; j < rule->rule_->component_cnt_; ++j) {
                            if (check_all_nullable(rule->rule_->components_+i+1, rule->rule_->components_+j)) {
                                SynSymbol *YI = rule->rule_->components_[i];
                                SynSymbol *YJ = rule->rule_->components_[j];
                                YI->follow_.insert(YJ->first_.begin(), YJ->first_.end());
                            }
                        }
                    }

                }
            }
        }
    }
    while (s1 < terms_follow_set_cnt());
}

void proc_ll1() {
    std::map<Rule*, std::set<SynSymbol*>> tt;
    for (SynSymbol *syn : g_non_terms) {
        for (RuleList *rule = syn->rules_; rule; rule = rule->next_) {
            if (!rule->rule_) {
                Rule *rr = new Rule;
                rr->target = syn;
                rr->components_ = nullptr;
                rr->component_cnt_ = 0;
                std::set<SynSymbol*> fol = syn->follow_;
                for (SynSymbol *ssyn : fol) {
                    tt[rr].insert(ssyn);
                }
            }
            else {
                for (int i = 0; i < rule->rule_->component_cnt_; ++i) {
                    std::set<SynSymbol*> fst = first_of_strand(rule->rule_->components_, rule->rule_->components_ + rule->rule_->component_cnt_);
                    for (SynSymbol *ssyn : fst) {
                        tt[rule->rule_].insert(ssyn);
                    }
                    if (nullable_of_strand(rule->rule_->components_, rule->rule_->components_ + rule->rule_->component_cnt_)) {
                        std::set<SynSymbol*> fol = syn->follow_;
                        for (SynSymbol *ssyn : fol) {
                            tt[rule->rule_].insert(ssyn);
                        }
                    }
                }
            }
        }
    }

    /*
    for (auto it : tt) {
        print_rule(it.first);
        for (auto syn : it.second)
            printf("%s, ", syn->sym->symbol_.c_str());
        printf("\n");
    }*/

    std::map<SynSymbol*, std::map<SynSymbol*, RuleList*>> qq;

    for (auto it : tt) {
        SynSymbol *target = it.first->target;
        for (SynSymbol *syn : it.second) {
            RuleList *rs = new RuleList;
            rs->rule_ = it.first;
            rs->next_ = qq[target][syn];
            qq[target][syn] = rs;
        }
    }

    for (auto it1 : qq) {
        for (auto it2 : it1.second) {
            printf("<%s>, <%s> ==> \n   ", it1.first->sym->symbol_.c_str(), it2.first->sym->symbol_.c_str());
            int i = 0;
            for (RuleList *rs = it2.second; rs; rs = rs->next_,++i) {
                if (i > 0)
                    printf(" ## ");
                print_rule(rs->rule_);
            }
//            assert(i==1);   /* check LL(1) */
            printf("\n");
        }
    }

    return;
}


void proc() {
    proc_nullable();
    proc_first();
    proc_follow();


    print_first();
    print_follow();
    proc_ll1();
}

Rule *make_rule(Symbol *target, int component_cnt, Symbol **coms) {
    Rule *ret = new Rule;

    SynSymbol *tar = (SynSymbol*)S_look(g_syns, target);
    assert(tar);

    ret->target = tar;
    ret->components_ = new SynSymbol*[component_cnt];
    ret->component_cnt_ = component_cnt;
    for (unsigned int i = 0; i < ret->component_cnt_; ++i) {
        Symbol *component = coms[i];
        SynSymbol *com = (SynSymbol*)S_look(g_syns, component);
        assert(com);
        ret->components_[i] = com;
    }
    return ret;
}