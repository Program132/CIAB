#include "strlist.h"
#include <stdlib.h>
#include <string.h>

struct StrList *strlist_create(const char* str) {
    struct StrList *list = (struct StrList *) malloc(sizeof(struct StrList));
    if (!list) return NULL;
    list->head = (struct StrListNode *) malloc(sizeof(struct StrListNode));
    if (!list->head) {
        free(list);
        return NULL;
    }
    list->head->value = str;
    list->head->next = NULL;
    list->size = 1;
    return list;
}

struct StrList *strlist_create_empty() {
    struct StrList *list = (struct StrList *) malloc(sizeof(struct StrList));
    if (!list) return NULL;
    list->head = NULL;
    list->size = 0;
    return list;
}

void strlist_add(struct StrList *liste, const char* str) {
    if (!liste) return;

    struct StrListNode *new_node = (struct StrListNode *) malloc(sizeof(struct StrListNode));
    if (!new_node) return;
    new_node->value = str;
    new_node->next = NULL;

    if (liste->head == NULL) {
        liste->head = new_node;
    } else {
        struct StrListNode *cc = liste->head;
        while (cc->next != NULL) {
            cc = cc->next;
        }
        cc->next = new_node;
    }
    liste->size += 1;
}

const char* strlist_get(struct StrList *liste, int indice) {
    if (!liste || indice < 0 || (size_t)indice >= liste->size) {
        return NULL;
    }
    struct StrListNode *cc = liste->head;
    int i = 0;
    while (cc != NULL && i < indice) {
        cc = cc->next;
        i += 1;
    }
    if (cc == NULL) {
        return NULL;
    }
    return cc->value;
}

int strlist_is_empty(struct StrList *liste) {
    if (!liste) return 1;
    return liste->size == 0 ? 1 : 0;
}

int strlist_size(struct StrList *liste) {
    if (!liste) return 0;
    return (int)liste->size;
}

int strlist_find(struct StrList *liste, const char* str) {
    if (!liste || !str) return -1;
    struct StrListNode *cc = liste->head;
    int i = 0;
    while (cc != NULL) {
        if (cc->value && strcmp(cc->value, str) == 0) {
            return i;
        }
        cc = cc->next;
        i += 1;
    }
    return -1;
}

void strlist_free(struct StrList *liste) {
    if (!liste) return;
    struct StrListNode *cc = liste->head;
    while (cc != NULL) {
        struct StrListNode *next = cc->next;
        free(cc);
        cc = next;
    }
    free(liste);
}