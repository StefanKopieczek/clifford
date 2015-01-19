#ifndef _H_CLIFFORD_LL
#define _H_CLIFFORD_LL

typedef struct cliff_list CLIFF_LIST;
typedef struct cliff_list_node CLIFF_LIST_NODE;

struct cliff_list
{
    CLIFF_LIST_NODE *start;
};

struct cliff_list_node
{
    CLIFF_LIST_NODE *next;
    void *data;
};

CLIFF_LIST *new_cliff_list();
void free_cliff_list(CLIFF_LIST *ll);
void cliff_list_add(CLIFF_LIST *ll, void *data);
int cliff_list_remove(CLIFF_LIST *ll, void *data);

#endif
