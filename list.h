#ifndef LIST_H
#define LIST_H

typedef int elem;

typedef struct node {
    elem value;
    struct node *next;
} node_t;

typedef struct list {
    node_t *head;
} list_t;

list_t *list_alloc(void);
void    list_free(list_t *l);

int     list_length(list_t *l);
char   *listToString(list_t *l);   /* caller must free() the result */

void    list_add_to_back(list_t *l, elem value);
void    list_add_to_front(list_t *l, elem value);
void    list_add_at_index(list_t *l, elem value, int index);

elem    list_remove_from_back(list_t *l);
elem    list_remove_from_front(list_t *l);
elem    list_remove_at_index(list_t *l, int index);

elem    list_get_elem_at(list_t *l, int index);

#endif