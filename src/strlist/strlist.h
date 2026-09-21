#ifndef STRLIST_H
#define STRLIST_H

#include <stddef.h>

struct StrListNode
{
    const char *value;
    struct StrListNode *next;
};

struct StrList
{
    struct StrListNode *head;
    size_t size;
};

struct StrList *strlist_create(const char* str);
struct StrList *strlist_create_empty();
void strlist_add(struct StrList *liste, const char* str);
const char* strlist_get(struct StrList *liste, int indice);
int strlist_is_empty(struct StrList *liste);
int strlist_size(struct StrList *liste);
int strlist_find(struct StrList *liste, const char* str);
void strlist_free(struct StrList *liste);

#endif