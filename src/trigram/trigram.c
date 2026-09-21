#include "trigram.h"
#include <stdlib.h>
#include <stdio.h>

struct TrigramList* trigramlist_create(size_t capacity) {
    if (capacity == 0) {
        capacity = 16;
    }
    struct TrigramList* list = malloc(sizeof(struct TrigramList));
    if (list == NULL) {
        return NULL;
    }
    list->trigrams = malloc(capacity * sizeof(struct Trigram*));
    if (list->trigrams == NULL) {
        free(list);
        return NULL;
    }
    list->capacity = capacity;
    list->size = 0;
    return list;
}

int trigramlist_is_empty(const struct TrigramList *list) {
    if (list == NULL) {
        return 1;
    }
    return list->size == 0;
}

int trigramlist_is_full(const struct TrigramList *list) {
    if (list == NULL) {
        return 1;
    }
    return list->size == list->capacity;
}

void trigramlist_add(struct TrigramList *list, uint32_t id1, uint32_t id2, uint32_t id3) {
    if (list == NULL) {
        return;
    }
    if (list->size >= list->capacity) {
        size_t new_cap = list->capacity == 0 ? 16 : list->capacity * 2;
        struct Trigram **new_trigrams = realloc(list->trigrams, new_cap * sizeof(struct Trigram*));
        if (new_trigrams == NULL) {
            return;
        }
        list->trigrams = new_trigrams;
        list->capacity = new_cap;
    }

    struct Trigram* trigram = malloc(sizeof(struct Trigram));
    if (trigram == NULL) {
        return;
    }
    trigram->id1 = id1;
    trigram->id2 = id2;
    trigram->id3 = id3;
    trigram->count = 1;
    list->trigrams[list->size] = trigram;
    list->size++;
}

size_t trigramlist_size(const struct TrigramList *list) {
    if (list == NULL) {
        return 0;
    }
    return list->size;
}

struct Trigram* trigramlist_get(const struct TrigramList *list, size_t index) {
    if (list == NULL || index >= list->size) {
        return NULL;
    }
    return list->trigrams[index];
}

static int trigram_compare(const void *a, const void *b) {
    const struct Trigram *t1 = *(const struct Trigram **)a;
    const struct Trigram *t2 = *(const struct Trigram **)b;
    if (t1->id1 != t2->id1) {
        return (t1->id1 < t2->id1) ? -1 : 1;
    }
    if (t1->id2 != t2->id2) {
        return (t1->id2 < t2->id2) ? -1 : 1;
    }
    if (t1->id3 != t2->id3) {
        return (t1->id3 < t2->id3) ? -1 : 1;
    }
    return 0;
}

void trigramlist_sort(struct TrigramList *list) {
    if (list == NULL || list->size == 0) {
        return;
    }
    qsort(list->trigrams, list->size, sizeof(struct Trigram*), trigram_compare);
}

uint32_t trigramlist_predict(const struct TrigramList *list, uint32_t id1, uint32_t id2) {
    if (list == NULL || list->size == 0) {
        return 0;
    }
    uint32_t best_id3 = 0;
    uint32_t max_count = 0;
    for (size_t i = 0; i < list->size; i++) {
        if (list->trigrams[i]->id1 == id1 && list->trigrams[i]->id2 == id2) {
            if (list->trigrams[i]->count > max_count) {
                max_count = list->trigrams[i]->count;
                best_id3 = list->trigrams[i]->id3;
            }
        }
    }
    return best_id3;
}

void trigramlist_save_bin(const struct TrigramList *list, const char *filepath) {
    if (list == NULL || filepath == NULL) {
        return;
    }
    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL) {
        return;
    }
    fwrite(&list->size, sizeof(size_t), 1, fp);
    for (size_t i = 0; i < list->size; i++) {
        fwrite(list->trigrams[i], sizeof(struct Trigram), 1, fp);
    }
    fclose(fp);
}

struct TrigramList* trigramlist_load_bin(const char *filepath) {
    if (filepath == NULL) {
        return NULL;
    }
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL) {
        return NULL;
    }
    size_t size = 0;
    if (fread(&size, sizeof(size_t), 1, fp) != 1) {
        fclose(fp);
        return NULL;
    }
    struct TrigramList* list = trigramlist_create(size > 0 ? size : 16);
    if (list == NULL) {
        fclose(fp);
        return NULL;
    }
    for (size_t i = 0; i < size; i++) {
        list->trigrams[i] = malloc(sizeof(struct Trigram));
        if (list->trigrams[i] == NULL) {
            list->size = i;
            trigramlist_free(list);
            fclose(fp);
            return NULL;
        }
        if (fread(list->trigrams[i], sizeof(struct Trigram), 1, fp) != 1) {
            free(list->trigrams[i]);
            list->size = i;
            trigramlist_free(list);
            fclose(fp);
            return NULL;
        }
    }
    list->size = size;
    fclose(fp);
    return list;
}

void trigramlist_free(struct TrigramList* list) {
    if (list == NULL) {
        return;
    }
    if (list->trigrams != NULL) {
        for (size_t i = 0; i < list->size; i++) {
            free(list->trigrams[i]);
        }
        free(list->trigrams);
    }
    free(list);
}