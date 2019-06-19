#include <dynamic_programming_contained.h>
#include "dynamic_programming_contained.h"
#include "hash_map_double_key.h"
#include "search_condition.h"
#include <assert.h>
#include <search_condition.h>
#include <ctime>

#include "predicate.h"
#include "row_value_constructor.h"

#include "time.h"
#include <stdio.h>


bool contained(Predicate *A, BooleanTerm *B, HashMapDoubleKey * contained_tbl) {
    for (BooleanFactorList *it = B->boolean_factors_; it != nullptr; it = it->next_) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                //if (!contained(A, primary->u.predicate_))
                //    return false;
                bool v = false, r = false;
                r = get_hashmap_double_key(contained_tbl, A, primary->u.predicate_, &v);
                assert(r);
                if (!v) {
                    set_hashmap_double_key(contained_tbl, A, B, false);
                    return false;
                }
            } break;
            case BooleanPrimary::SEARCH: {
                //if (!contained(A, primary->u.search_))
                //    return false;
                bool v = false, r = false;
                r = get_hashmap_double_key(contained_tbl, A, primary->u.search_, &v);
                assert(r);
                if (!v) {
                    set_hashmap_double_key(contained_tbl, A, B, false);
                    return false;
                }
            } break;
            default: assert(false);
        }
    }
    set_hashmap_double_key(contained_tbl, A, B, true);
    return true;
}

bool contained(Predicate *A, SearchCondition *B, HashMapDoubleKey * contained_tbl) {
    for (BooleanTermList *it = B->boolean_terms_; it != nullptr; it = it->next_) {
        BooleanTerm *term = it->term_;
        //if (contained(A, term))
        //    return true;
        bool v = false, r = false;
        r = get_hashmap_double_key(contained_tbl, A, term, &v);
        assert(r);
        if (v) {
            set_hashmap_double_key(contained_tbl, A, B, true);
            return true;
        }
    }
    set_hashmap_double_key(contained_tbl, A, B, false);
    return false;
}

bool contained(BooleanTerm *A, Predicate *B, HashMapDoubleKey * contained_tbl) {
    for (BooleanFactorList *it = A->boolean_factors_; it != nullptr; it = it->next_) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                //if (contained(primary->u.predicate_, B))
                //    return true;
                bool v = false, r = false;
                r = get_hashmap_double_key(contained_tbl, primary->u.predicate_, B, &v);
                assert(r);
                if (v) {
                    set_hashmap_double_key(contained_tbl, A, B, true);
                    return true;
                }
            } break;
            case BooleanPrimary::SEARCH: {
                //if (contained(primary->u.search_, B))
                //    return true;
                bool v = false, r = false;
                r = get_hashmap_double_key(contained_tbl, primary->u.search_, B, &v);
                assert(r);
                if (v) {
                    set_hashmap_double_key(contained_tbl, A, B, true);
                    return true;
                }
            } break;
            default: assert(false);
        }
    }
    set_hashmap_double_key(contained_tbl, A, B, false);
    return false;
}

bool contained(BooleanTerm *A, BooleanTerm *B, HashMapDoubleKey * contained_tbl) {
    bool ret = false;
    for (BooleanFactorList *it = A->boolean_factors_; it != nullptr; it = it->next_) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                //if (contained(primary->u.predicate_, B))
                //    return true;
                bool v = false, r = false;
                r = get_hashmap_double_key(contained_tbl, primary->u.predicate_, B, &v);
                assert(r);
                if (v) {
                    set_hashmap_double_key(contained_tbl, A, B, true);
                    return true;
                }
            } break;
            case BooleanPrimary::SEARCH: {
                //if (contained(primary->u.search_, B))
                //    return true;
                bool v = false, r = false;
                r = get_hashmap_double_key(contained_tbl, primary->u.search_, B, &v);
                assert(r);
                if (v) {
                    set_hashmap_double_key(contained_tbl, A, B, true);
                    return true;
                }
            } break;
            default: assert(false);
        }
    }

    ret = true;
    for (BooleanFactorList *it = B->boolean_factors_; it != nullptr; it = it->next_) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                //if (!contained(A, primary->u.predicate_))
                //    return false;
                bool v = false, r = false;
                r = get_hashmap_double_key(contained_tbl, A, primary->u.predicate_, &v);
                assert(r);
                if (!v) {
                    set_hashmap_double_key(contained_tbl, A, B, false);
                    return false;
                }

            } break;
            case BooleanPrimary::SEARCH: {
                //if (!contained(A, primary->u.search_))
                //    return false;
                bool v = false, r = false;
                r = get_hashmap_double_key(contained_tbl, A, primary->u.search_, &v);
                assert(r);
                if (!v) {
                    set_hashmap_double_key(contained_tbl, A, B, false);
                    return false;
                }
            } break;
            default: assert(false);
        }
    }
    set_hashmap_double_key(contained_tbl, A, B, ret);
    return ret;
}

bool contained(BooleanTerm *A, SearchCondition *B, HashMapDoubleKey * contained_tbl) {
    bool ret = false;
    for (BooleanFactorList *it = A->boolean_factors_; it != nullptr; it = it->next_) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        switch (primary->type_) {
            case BooleanPrimary::PRE: {
                //if (contained(primary->u.predicate_, B))
                //    return true;
                bool v = false, r = false;
                r = get_hashmap_double_key(contained_tbl, primary->u.predicate_, B, &v);
                assert(r);
                if (v) {
                    set_hashmap_double_key(contained_tbl, A, B, true);
                    return true;
                }
            } break;
            case BooleanPrimary::SEARCH: {
                //if (contained(primary->u.search_, B))
                //    return true;
                bool v = false, r = false;
                r = get_hashmap_double_key(contained_tbl, primary->u.search_, B, &v);
                assert(r);
                if (v) {
                    set_hashmap_double_key(contained_tbl, A, B, true);
                    return true;
                }
            } break;
            default: assert(false);
        }
    }

    for (BooleanTermList *it = B->boolean_terms_; it != nullptr; it = it->next_) {
        //if (contained(A, it->term_))
        //    return true;
        bool v = false, r = false;
        r = get_hashmap_double_key(contained_tbl, A, it->term_, &v);
        assert(r);
        if (v) {
            set_hashmap_double_key(contained_tbl, A, B, true);
            return true;
        }
    }
    set_hashmap_double_key(contained_tbl, A, B, ret);
    return ret;
}

bool contained(SearchCondition *A, Predicate *B, HashMapDoubleKey * contained_tbl) {
    for (BooleanTermList *it = A->boolean_terms_; it != nullptr; it = it->next_) {
        BooleanTerm *term = it->term_;
        //if (!contained(term, B))
        //    return false;
        bool v = false, r = false;
        r = get_hashmap_double_key(contained_tbl, term, B, &v);
        assert(r);
        if (!v) {
            set_hashmap_double_key(contained_tbl, A, B, false);
            return false;
        }
    }
    set_hashmap_double_key(contained_tbl, A, B, true);
    return true;
}

bool contained(SearchCondition *A, BooleanTerm *B, HashMapDoubleKey * contained_tbl) {
    bool ret = true;
    for (BooleanTermList *it = A->boolean_terms_; it != nullptr; it = it->next_) {
        BooleanTerm *term = it->term_;
        //if (!contained(term, B)) {
        //    ret = false;
        //    break;
        //}
        bool v = false, r = false;
        r = get_hashmap_double_key(contained_tbl, term, B, &v);
        assert(r);
        if (!v) {
            ret = false;
            break;
        }
    }
    if (ret) {
        set_hashmap_double_key(contained_tbl, A, B, true);
        return true;
    }

    for (BooleanFactorList *it = B->boolean_factors_; it != nullptr; it = it->next_) {
        BooleanPrimary *p = booleanfactor2primary(it->factor_);
        switch (p->type_) {
            case BooleanPrimary::PRE: {
                //if (!contained(A, p->u.predicate_))
                //    return false;
                bool v = false, r = false;
                r = get_hashmap_double_key(contained_tbl, A, p->u.predicate_, &v);
                assert(r);
                if (!v) {
                    set_hashmap_double_key(contained_tbl, A, B, false);
                    return false;
                }
            } break;
            case BooleanPrimary::SEARCH: {
                //if (!contained(A, p->u.search_))
                //    return false;
                bool v = false, r = false;
                r = get_hashmap_double_key(contained_tbl, A, p->u.search_, &v);
                assert(r);
                if (!v) {
                    set_hashmap_double_key(contained_tbl, A, B, false);
                    return false;
                }
            } break;
            default: assert(false);
        }
    }
    set_hashmap_double_key(contained_tbl, A, B, true);
    return true;
}

bool contained(SearchCondition *A, SearchCondition *B, HashMapDoubleKey * contained_tbl) {
    bool ret = true;
    for (BooleanTermList *it = (A->boolean_terms_); it != nullptr; it = (it->next_)) {
        BooleanTerm *term = it->term_;
        //if (!contained(term, B)) {
        //    ret = false;
        //    break;
        //}
        bool v = false, r = false;
        r = get_hashmap_double_key(contained_tbl, term, B, &v);
        assert(r);
        if (!v) {
            ret = false;
            break;
        }
    }
    if (ret) {
        set_hashmap_double_key(contained_tbl, A, B, true);
        return ret;
    }

    ret = false;
    for (BooleanTermList *it = (B->boolean_terms_); it != nullptr; it = (it->next_)) {
        //if (contained(A, it->term_)) {
        //    ret = true;
        //    break;
        //}
        bool v = false, r = false;
        r = get_hashmap_double_key(contained_tbl, A, it->term_, &v);
        assert(r);
        if (v) {
            ret = true;
            break;
        }
    }
    set_hashmap_double_key(contained_tbl, A, B, ret);
    return ret;
}


LogicNodeListList *make_logic_node_list_list(LogicNodeListList *head, LogicNodeList *ls) {
    LogicNodeListList *ret = new LogicNodeListList;
    ret->ls_ = ls; ret->next_ = head;
    return ret;
}

LogicNodeList *make_logic_node_list(LogicNodeList *head, LogicNode *logic_node) {
    LogicNodeList *ret = new LogicNodeList;
    ret->logic_node_ = logic_node; ret->next_ = head;
    return ret;
}

LogicNode *make_logic_node(SearchCondition *sc) {
    LogicNode *ret = new LogicNode;
    ret->type_ = LogicNode::OR; ret->u.or_ = sc;
    return ret;
}

LogicNode *make_logic_node(BooleanTerm *bt) {
    LogicNode *ret = new LogicNode;
    ret->type_ = LogicNode::AND; ret->u.and_ = bt;
    return ret;
}

LogicNode *make_logic_node(Predicate *p) {
    LogicNode *ret = new LogicNode;
    ret->type_ = LogicNode::PRE; ret->u.predicate_ = p;
    return ret;
}





void proc_contained(HashMapDoubleKey * contained_tbl, SearchCondition *A) {
    LogicNodeList *logic_node_list = nullptr;
    LogicNodeList *logic_node_list_pre = nullptr;

    LogicNodeListList *lvl_list = nullptr;
    traverse_lvl(A, &lvl_list);

    clock_t start, finish;
    double duration;
    start = clock();

    for (LogicNodeListList *lvl = lvl_list; lvl != nullptr; lvl = lvl->next_) {
        // proc cur_lvl 's node with previous lvl 's node
        for (LogicNodeList *it = lvl->ls_; it != nullptr; it = it->next_) {
            for (LogicNodeListList *i = lvl_list; i != lvl; i = i->next_) {
                for (LogicNodeList *proced = i->ls_; proced; proced = proced->next_) {
                    if (it->logic_node_->type_ == LogicNode::PRE && proced->logic_node_->type_ == LogicNode::PRE) {
                        set_hashmap_double_key(contained_tbl, it->logic_node_->u.predicate_, proced->logic_node_->u.predicate_,
                                contained(it->logic_node_->u.predicate_, proced->logic_node_->u.predicate_));
                        set_hashmap_double_key(contained_tbl, proced->logic_node_->u.predicate_, it->logic_node_->u.predicate_,
                                               contained(proced->logic_node_->u.predicate_, it->logic_node_->u.predicate_));
                    }
                    else if (it->logic_node_->type_ == LogicNode::PRE && proced->logic_node_->type_ == LogicNode::AND) {
                        contained(it->logic_node_->u.predicate_, proced->logic_node_->u.and_, contained_tbl);
                        contained(proced->logic_node_->u.and_, it->logic_node_->u.predicate_, contained_tbl);
                    }
                    else if (it->logic_node_->type_ == LogicNode::PRE && proced->logic_node_->type_ == LogicNode::OR) {
                        contained(it->logic_node_->u.predicate_, proced->logic_node_->u.or_, contained_tbl);
                        contained(proced->logic_node_->u.or_, it->logic_node_->u.predicate_, contained_tbl);
                    }
                    else if (it->logic_node_->type_ == LogicNode::AND && proced->logic_node_->type_ == LogicNode::PRE) {
                        contained(it->logic_node_->u.and_, proced->logic_node_->u.predicate_, contained_tbl);
                        contained(proced->logic_node_->u.predicate_, it->logic_node_->u.and_, contained_tbl);
                    }
                    else if (it->logic_node_->type_ == LogicNode::AND && proced->logic_node_->type_ == LogicNode::AND) {
                        contained(it->logic_node_->u.and_, proced->logic_node_->u.and_, contained_tbl);
                        contained(proced->logic_node_->u.and_, it->logic_node_->u.and_, contained_tbl);
                    }
                    else if (it->logic_node_->type_ == LogicNode::AND && proced->logic_node_->type_ == LogicNode::OR) {
                        contained(it->logic_node_->u.and_, proced->logic_node_->u.or_, contained_tbl);
                        contained(proced->logic_node_->u.or_, it->logic_node_->u.and_, contained_tbl);
                    }
                    else if (it->logic_node_->type_ == LogicNode::OR && proced->logic_node_->type_ == LogicNode::PRE) {
                        contained(it->logic_node_->u.or_, proced->logic_node_->u.predicate_, contained_tbl);
                        contained(proced->logic_node_->u.predicate_, it->logic_node_->u.or_, contained_tbl);
                    }
                    else if (it->logic_node_->type_ == LogicNode::OR && proced->logic_node_->type_ == LogicNode::AND) {
                        contained(it->logic_node_->u.or_, proced->logic_node_->u.and_, contained_tbl);
                        contained(proced->logic_node_->u.and_, it->logic_node_->u.or_, contained_tbl);
                    }
                    else if (it->logic_node_->type_ == LogicNode::OR && proced->logic_node_->type_ == LogicNode::OR) {
                        contained(it->logic_node_->u.or_, proced->logic_node_->u.or_, contained_tbl);
                        contained(proced->logic_node_->u.or_, it->logic_node_->u.or_, contained_tbl);
                    }
                    else { assert(false); }
                }
            }
        }

        // proc cur_lvl 's node with other node in this lvl
        for (LogicNodeList *it1 = lvl->ls_; it1->next_ != nullptr; it1 = it1->next_) {
            for (LogicNodeList *it2 = it1->next_; it2 != nullptr; it2 = it2->next_) {
                LogicNodeList *it = it1;
                LogicNodeList *proced = it2;
                if (it->logic_node_->type_ == LogicNode::PRE && proced->logic_node_->type_ == LogicNode::PRE) {
                    set_hashmap_double_key(contained_tbl, it->logic_node_->u.predicate_, proced->logic_node_->u.predicate_,
                                           contained(it->logic_node_->u.predicate_, proced->logic_node_->u.predicate_));
                    set_hashmap_double_key(contained_tbl, proced->logic_node_->u.predicate_, it->logic_node_->u.predicate_,
                                           contained(proced->logic_node_->u.predicate_, it->logic_node_->u.predicate_));
                }
                else if (it->logic_node_->type_ == LogicNode::PRE && proced->logic_node_->type_ == LogicNode::AND) {
                    contained(it->logic_node_->u.predicate_, proced->logic_node_->u.and_, contained_tbl);
                    contained(proced->logic_node_->u.and_, it->logic_node_->u.predicate_, contained_tbl);
                }
                else if (it->logic_node_->type_ == LogicNode::PRE && proced->logic_node_->type_ == LogicNode::OR) {
                    contained(it->logic_node_->u.predicate_, proced->logic_node_->u.or_, contained_tbl);
                    contained(proced->logic_node_->u.or_, it->logic_node_->u.predicate_, contained_tbl);
                }
                else if (it->logic_node_->type_ == LogicNode::AND && proced->logic_node_->type_ == LogicNode::PRE) {
                    contained(it->logic_node_->u.and_, proced->logic_node_->u.predicate_, contained_tbl);
                    contained(proced->logic_node_->u.predicate_, it->logic_node_->u.and_, contained_tbl);
                }
                else if (it->logic_node_->type_ == LogicNode::AND && proced->logic_node_->type_ == LogicNode::AND) {
                    contained(it->logic_node_->u.and_, proced->logic_node_->u.and_, contained_tbl);
                    contained(proced->logic_node_->u.and_, it->logic_node_->u.and_, contained_tbl);
                }
                else if (it->logic_node_->type_ == LogicNode::AND && proced->logic_node_->type_ == LogicNode::OR) {
                    contained(it->logic_node_->u.and_, proced->logic_node_->u.or_, contained_tbl);
                    contained(proced->logic_node_->u.or_, it->logic_node_->u.and_, contained_tbl);
                }
                else if (it->logic_node_->type_ == LogicNode::OR && proced->logic_node_->type_ == LogicNode::PRE) {
                    contained(it->logic_node_->u.or_, proced->logic_node_->u.predicate_, contained_tbl);
                    contained(proced->logic_node_->u.predicate_, it->logic_node_->u.or_, contained_tbl);
                }
                else if (it->logic_node_->type_ == LogicNode::OR && proced->logic_node_->type_ == LogicNode::AND) {
                    contained(it->logic_node_->u.or_, proced->logic_node_->u.and_, contained_tbl);
                    contained(proced->logic_node_->u.and_, it->logic_node_->u.or_, contained_tbl);
                }
                else if (it->logic_node_->type_ == LogicNode::OR && proced->logic_node_->type_ == LogicNode::OR) {
                    contained(it->logic_node_->u.or_, proced->logic_node_->u.or_, contained_tbl);
                    contained(proced->logic_node_->u.or_, it->logic_node_->u.or_, contained_tbl);
                }
                else { assert(false); }
            }
        }
    }

    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    printf( "stage2 %f seconds\n", duration );
    return;
}


void traverse_lvl(SearchCondition * A, LogicNodeListList **plvl_list) {
    LogicNodeList *logic_node_list = make_logic_node_list(nullptr, make_logic_node(A));
    LogicNodeList *last = logic_node_list;
    LogicNodeListList *logic_node_list_list = make_logic_node_list_list(nullptr, last);
    bool flag = true;
    while (flag) {
        flag = false;
        logic_node_list = nullptr;
        for (LogicNodeList *it = last; it != nullptr; it = it->next_) {
            switch (it->logic_node_->type_) {
                case LogicNode::OR: {
                    for (BooleanTermList *bt = it->logic_node_->u.or_->boolean_terms_; bt != nullptr; bt = bt->next_) {
                        logic_node_list = make_logic_node_list(logic_node_list, make_logic_node(bt->term_));
                    }
                    flag = true;
                } break;
                case LogicNode::AND: {
                    for (BooleanFactorList *bf = it->logic_node_->u.and_->boolean_factors_; bf != nullptr; bf = bf->next_) {
                        BooleanPrimary *bp = booleanfactor2primary(bf->factor_);
                        if (bp->type_ == BooleanPrimary::SEARCH) {
                            logic_node_list = make_logic_node_list(logic_node_list, make_logic_node(bp->u.search_));
                        }
                        else {
                            logic_node_list = make_logic_node_list(logic_node_list, make_logic_node(bp->u.predicate_));
                        }
                    }
                    flag = true;
                } break;
                case LogicNode::PRE: {
                    // leaf node
                } break;
            }
        }
        if (flag) {
            last = logic_node_list;
            logic_node_list_list = make_logic_node_list_list(logic_node_list_list, last);
        }
    }
    *plvl_list = logic_node_list_list;

}

void optimize_d(SearchCondition *A, HashMapDoubleKey *contained_tbl) {
    for (BooleanTermList *it = A->boolean_terms_; it != nullptr; it = it->next_) {
        optimize_d(it->term_,contained_tbl);
    }

    for (BooleanTermList *it1 = A->boolean_terms_; it1 != nullptr; it1 = it1->next_) {
        for (BooleanTermList *it2 = it1->next_; it2 != nullptr; it2 = it2->next_) {
            bool v = false, r = false;
            r = get_hashmap_double_key(contained_tbl, it1->term_, it2->term_, &v);
            assert(r);
            if (v) {
                it1->is_optimized_ = true;
            }
            else {
                r = get_hashmap_double_key(contained_tbl, it2->term_, it1->term_, &v);
                assert(r);
                if (v) {
                    it2->is_optimized_ = true;
                }
            }
        }
    }
}

void optimize_d(BooleanTerm *A, HashMapDoubleKey *contained_tbl) {
    for (BooleanFactorList *it = A->boolean_factors_; it != nullptr; it = it->next_) {
        BooleanPrimary *primary = booleanfactor2primary(it->factor_);
        if (primary->type_ == BooleanPrimary::SEARCH) {
            optimize_d(primary->u.search_, contained_tbl);
        }
    }

    for (BooleanFactorList *it1 = A->boolean_factors_; it1 != nullptr; it1 = it1->next_) {
        for (BooleanFactorList *it2 = it1->next_; it2 != nullptr; it2 = it2->next_) {
            bool is_con = false;
            BooleanPrimary *p1 = booleanfactor2primary(it1->factor_);
            BooleanPrimary *p2 = booleanfactor2primary(it2->factor_);
            bool r = false;
            if (p1->type_ == BooleanPrimary::SEARCH && p2->type_ == BooleanPrimary::SEARCH) {
                //is_con = contained(p1->u.search_, p2->u.search_);
                r = get_hashmap_double_key(contained_tbl, p1->u.search_, p2->u.search_, &is_con);
            }
            else if (p1->type_ == BooleanPrimary::SEARCH && p2->type_ == BooleanPrimary::PRE) {
                //is_con = contained(p1->u.search_, p2->u.predicate_);
                r = get_hashmap_double_key(contained_tbl, p1->u.search_, p2->u.predicate_, &is_con);
            }
            else if (p1->type_ == BooleanPrimary::PRE && p2->type_ == BooleanPrimary::SEARCH) {
                //is_con = contained(p1->u.predicate_, p2->u.search_);
                r = get_hashmap_double_key(contained_tbl, p1->u.predicate_, p2->u.search_, &is_con);
            }
            else {
                //is_con = contained(p1->u.predicate_, p2->u.predicate_);
                r = get_hashmap_double_key(contained_tbl, p1->u.predicate_, p2->u.predicate_, &is_con);
            }
            assert(r);
            if (is_con) {
                it2->is_optimized_ = true;
            }
            else {
                if (p1->type_ == BooleanPrimary::SEARCH && p2->type_ == BooleanPrimary::SEARCH) {
                    //is_con = contained(p2->u.search_, p1->u.search_);
                    r = get_hashmap_double_key(contained_tbl, p2->u.search_, p1->u.search_, &is_con);
                }
                else if (p1->type_ == BooleanPrimary::SEARCH && p2->type_ == BooleanPrimary::PRE) {
                    //is_con = contained(p2->u.predicate_, p1->u.search_);
                    r = get_hashmap_double_key(contained_tbl, p2->u.predicate_, p1->u.search_, &is_con);
                }
                else if (p1->type_ == BooleanPrimary::PRE && p2->type_ == BooleanPrimary::SEARCH) {
                    //is_con = contained(p2->u.search_, p1->u.predicate_);
                    r = get_hashmap_double_key(contained_tbl, p2->u.search_, p1->u.predicate_, &is_con);
                }
                else {
                    //is_con = contained(p2->u.predicate_, p1->u.predicate_);
                    r = get_hashmap_double_key(contained_tbl, p2->u.predicate_, p1->u.predicate_, &is_con);
                }
                if (is_con) {
                    it1->is_optimized_ = true;
                }
            }
        }
    }
}



