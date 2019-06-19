#ifndef DYNAMIC_PROGRAMMING_CONTAINED
#define DYNAMIC_PROGRAMMING_CONTAINED

struct SearchCondition;
struct BooleanTerm;
struct Predicate;
struct HashMapDoubleKey;
struct LogicNodeList;
struct LogicNodeList;
struct LogicNodeListList;

struct LogicNode {
    enum { OR, AND, PRE } type_;
    union {
        SearchCondition *or_;
        BooleanTerm *and_;
        Predicate *predicate_;
    } u;
};

struct LogicNodeList {
    LogicNode *logic_node_;
    LogicNodeList *next_;
};

struct LogicNodeListList {
    LogicNodeList *ls_;
    LogicNodeListList *next_;
};

void proc_contained(HashMapDoubleKey *, SearchCondition*);

void traverse_lvl(SearchCondition *, LogicNodeListList **);

void optimize_d(SearchCondition *A, HashMapDoubleKey *contained_tbl);
void optimize_d(BooleanTerm *A, HashMapDoubleKey *contained_tbl);

#endif