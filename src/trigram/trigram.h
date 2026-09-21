#ifndef TRIGRAM_H
#define TRIGRAM_H

#include <stddef.h>
#include <stdint.h>

struct Trigram {
    uint32_t id1;
    uint32_t id2;
    uint32_t id3;
    uint32_t count;
};

struct TrigramList {
    struct Trigram **trigrams;
    size_t capacity;
    size_t size;
};

struct TrigramList* trigramlist_create(size_t capacity);
int trigramlist_is_empty(const struct TrigramList *list);
int trigramlist_is_full(const struct TrigramList *list);
void trigramlist_add(struct TrigramList *list, uint32_t id1, uint32_t id2, uint32_t id3);
size_t trigramlist_size(const struct TrigramList *list);
struct Trigram* trigramlist_get(const struct TrigramList *list, size_t index);
void trigramlist_sort(struct TrigramList *list);
uint32_t trigramlist_predict(const struct TrigramList *list, uint32_t id1, uint32_t id2);
void trigramlist_save_bin(const struct TrigramList *list, const char *filepath);
struct TrigramList* trigramlist_load_bin(const char *filepath);
void trigramlist_free(struct TrigramList* list);

#endif
 