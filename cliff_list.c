#include "cliff_list.h"
#include <stdlib.h>

CLIFF_LIST *new_cliff_list()
{
    return calloc(1, sizeof(CLIFF_LIST));
}

void free_cliff_list(CLIFF_LIST *ll)
{
    CLIFF_LIST_NODE *node = ll->start;
    while (node != 0)
    {
        // Note: node->data is not freed. The caller needs to ensure this is
        // done first if needed.
        CLIFF_LIST_NODE *next = node->next;
        free(node);
        node = next;
    }

    free(ll);
}

void cliff_list_add(CLIFF_LIST *ll, void *data)
{
    CLIFF_LIST_NODE *new = malloc(sizeof(CLIFF_LIST_NODE));
    new->data = data;

    CLIFF_LIST_NODE *old_head = ll->start;
    ll->start = new;
    new->next = old_head;
}

int cliff_list_remove(CLIFF_LIST *ll, void *data)
{
    int found = 0;
    CLIFF_LIST_NODE *current = ll->start;
    if (current == 0)
    {
        // List is empty; nothing to do.
    }
    else if (current->data == data)
    {
        ll->start = current->next;
        free(current);
        found = 1;
    }
    else
    {
        CLIFF_LIST_NODE *previous = current;
        while (current = current->next)
        {
            if (current->data == data)
            {
                previous->next = current->next;
                free(current);
                found = 1;
                break;
            }

            previous = current;
        }
    }

    return found;
}
