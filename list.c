#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

/* Invalid index / empty list sentinel for remove and get. */
#define LIST_ERR (-1)

list_t *list_alloc(void) {
    list_t *l = malloc(sizeof(list_t));
    if (l) l->head = NULL;
    return l;
}

void list_free(list_t *l) {
    if (!l) return;
    node_t *cur = l->head;
    while (cur) {
        node_t *next = cur->next;
        free(cur);
        cur = next;
    }
    free(l);
}

int list_length(list_t *l) {
    int n = 0;
    for (node_t *c = l->head; c; c = c->next) n++;
    return n;
}

/* "1->2->3->NULL"; caller frees. */
char *listToString(list_t *l) {
    size_t cap = (size_t)list_length(l) * 13 + 8;   /* 11 digits + "->" per node */
    char *buf = malloc(cap);
    if (!buf) return NULL;
    char *p = buf;
    for (node_t *c = l->head; c; c = c->next)
        p += sprintf(p, "%d->", c->value);
    strcpy(p, "NULL");
    return buf;
}

void list_add_to_front(list_t *l, elem value) {
    node_t *n = malloc(sizeof(node_t));
    if (!n) return;
    n->value = value;
    n->next = l->head;
    l->head = n;
}

void list_add_to_back(list_t *l, elem value) {
    node_t *n = malloc(sizeof(node_t));
    if (!n) return;
    n->value = value;
    n->next = NULL;
    if (!l->head) { l->head = n; return; }
    node_t *c = l->head;
    while (c->next) c = c->next;
    c->next = n;
}

/* 0-based; valid range 0..length (length == append). Out of range: no-op. */
void list_add_at_index(list_t *l, elem value, int index) {
    if (index < 0 || index > list_length(l)) return;
    if (index == 0) { list_add_to_front(l, value); return; }
    node_t *prev = l->head;
    for (int i = 0; i < index - 1; i++) prev = prev->next;
    node_t *n = malloc(sizeof(node_t));
    if (!n) return;
    n->value = value;
    n->next = prev->next;
    prev->next = n;
}

elem list_remove_from_front(list_t *l) {
    if (!l->head) return LIST_ERR;
    node_t *n = l->head;
    elem v = n->value;
    l->head = n->next;
    free(n);
    return v;
}

elem list_remove_from_back(list_t *l) {
    if (!l->head) return LIST_ERR;
    if (!l->head->next) return list_remove_from_front(l);
    node_t *prev = l->head;
    while (prev->next->next) prev = prev->next;
    elem v = prev->next->value;
    free(prev->next);
    prev->next = NULL;
    return v;
}

elem list_remove_at_index(list_t *l, int index) {
    if (index < 0 || index >= list_length(l)) return LIST_ERR;
    if (index == 0) return list_remove_from_front(l);
    node_t *prev = l->head;
    for (int i = 0; i < index - 1; i++) prev = prev->next;
    node_t *n = prev->next;
    elem v = n->value;
    prev->next = n->next;
    free(n);
    return v;
}

elem list_get_elem_at(list_t *l, int index) {
    if (index < 0) return LIST_ERR;
    node_t *c = l->head;
    for (int i = 0; c && i < index; i++) c = c->next;
    return c ? c->value : LIST_ERR;
}