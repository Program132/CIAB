#ifndef HASHMAPSTR_H
#define HASHMAPSTR_H

#include <stddef.h>
#include <stdint.h>

#define HASHMAPSTR_NOT_FOUND UINT64_MAX

struct HashMapNode {
    char *key;
    uint64_t value;
    struct HashMapNode *next;
};

struct HashMapStr {
    size_t capacity;
    size_t size;
    struct HashMapNode **buckets;
};

unsigned long hash_string(const char *str);

struct HashMapStr *hashmapstr_create(size_t capacity);
void hashmapstr_insert(struct HashMapStr *map, const char *key, uint64_t value);
uint64_t hashmapstr_get(const struct HashMapStr *map, const char *key);
int hashmapstr_contains(const struct HashMapStr *map, const char *key);
size_t hashmapstr_size(const struct HashMapStr *map);
void hashmapstr_free(struct HashMapStr *map);

#endif