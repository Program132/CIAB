#include "hashmapstr.h"
#include <stdlib.h>
#include <string.h>

unsigned long hash_string(const char *str) {
    if (!str) return 0;
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

struct HashMapStr *hashmapstr_create(size_t capacity) {
    if (capacity == 0) {
        capacity = 16;
    }
    struct HashMapStr *map = malloc(sizeof(struct HashMapStr));
    if (!map) return NULL;
    map->capacity = capacity;
    map->size = 0;
    map->buckets = calloc(capacity, sizeof(struct HashMapNode *));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    return map;
}

void hashmapstr_insert(struct HashMapStr *map, const char *key, uint64_t value) {
    if (!map || !key || map->capacity == 0) return;

    unsigned long hash = hash_string(key);
    size_t index = hash % map->capacity;
    struct HashMapNode *node = map->buckets[index];
    while (node) {
        if (node->key && strcmp(node->key, key) == 0) {
            node->value = value;
            return;
        }
        node = node->next;
    }

    node = malloc(sizeof(struct HashMapNode));
    if (!node) return;
    node->key = strdup(key);
    if (!node->key) {
        free(node);
        return;
    }
    node->value = value;
    node->next = map->buckets[index];
    map->buckets[index] = node;
    map->size++;
}

uint64_t hashmapstr_get(const struct HashMapStr *map, const char *key) {
    if (!map || !key || map->capacity == 0) return HASHMAPSTR_NOT_FOUND;

    unsigned long hash = hash_string(key);
    size_t index = hash % map->capacity;
    struct HashMapNode *node = map->buckets[index];
    while (node) {
        if (node->key && strcmp(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    return HASHMAPSTR_NOT_FOUND;
}

int hashmapstr_contains(const struct HashMapStr *map, const char *key) {
    return hashmapstr_get(map, key) != HASHMAPSTR_NOT_FOUND;
}

size_t hashmapstr_size(const struct HashMapStr *map) {
    return map ? map->size : 0;
}

void hashmapstr_free(struct HashMapStr *map) {
    if (!map) return;
    if (map->buckets) {
        for (size_t i = 0; i < map->capacity; i++) {
            struct HashMapNode *node = map->buckets[i];
            while (node) {
                struct HashMapNode *next = node->next;
                free(node->key);
                free(node);
                node = next;
            }
        }
        free(map->buckets);
    }
    free(map);
}