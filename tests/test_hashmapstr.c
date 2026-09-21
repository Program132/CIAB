#undef NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "hashmapstr/hashmapstr.h"

static int g_assert_count = 0;

#define TEST_ASSERT(expr) do { \
    g_assert_count++; \
    assert(expr); \
} while (0)

void test_create() {
    printf("[TEST] Creation et proprietes initiales... ");
    struct HashMapStr *map = hashmapstr_create(32);
    TEST_ASSERT(map != NULL);
    TEST_ASSERT(map->capacity == 32);
    TEST_ASSERT(hashmapstr_size(map) == 0);
    TEST_ASSERT(hashmapstr_get(map, "inconnu") == HASHMAPSTR_NOT_FOUND);
    TEST_ASSERT(hashmapstr_contains(map, "inconnu") == 0);
    hashmapstr_free(map);

    struct HashMapStr *map_zero = hashmapstr_create(0);
    TEST_ASSERT(map_zero != NULL);
    TEST_ASSERT(map_zero->capacity == 16);
    TEST_ASSERT(hashmapstr_size(map_zero) == 0);
    hashmapstr_free(map_zero);
    printf("OK\n");
}

void test_null_safety() {
    printf("[TEST] Robustesse face aux pointeurs NULL... ");
    TEST_ASSERT(hashmapstr_size(NULL) == 0);
    TEST_ASSERT(hashmapstr_get(NULL, "cle") == HASHMAPSTR_NOT_FOUND);
    TEST_ASSERT(hashmapstr_get(NULL, NULL) == HASHMAPSTR_NOT_FOUND);
    TEST_ASSERT(hashmapstr_contains(NULL, "cle") == 0);
    TEST_ASSERT(hashmapstr_contains(NULL, NULL) == 0);

    struct HashMapStr *map = hashmapstr_create(16);
    TEST_ASSERT(hashmapstr_get(map, NULL) == HASHMAPSTR_NOT_FOUND);
    TEST_ASSERT(hashmapstr_contains(map, NULL) == 0);
    hashmapstr_insert(map, NULL, 42);
    TEST_ASSERT(hashmapstr_size(map) == 0);

    hashmapstr_insert(NULL, "cle", 42);
    hashmapstr_free(NULL);
    hashmapstr_free(map);
    printf("OK\n");
}

void test_id_zero_and_basic_ops() {
    printf("[TEST] Validation du token ID 0 et operations basiques... ");
    struct HashMapStr *map = hashmapstr_create(16);

    hashmapstr_insert(map, "<BOS>", 0);
    TEST_ASSERT(hashmapstr_size(map) == 1);
    TEST_ASSERT(hashmapstr_contains(map, "<BOS>") == 1);
    TEST_ASSERT(hashmapstr_get(map, "<BOS>") == 0);

    hashmapstr_insert(map, "<EOS>", 1);
    hashmapstr_insert(map, "chat", 2);
    hashmapstr_insert(map, "chien", 3);

    TEST_ASSERT(hashmapstr_size(map) == 4);
    TEST_ASSERT(hashmapstr_contains(map, "<EOS>") == 1);
    TEST_ASSERT(hashmapstr_contains(map, "chat") == 1);
    TEST_ASSERT(hashmapstr_contains(map, "chien") == 1);
    TEST_ASSERT(hashmapstr_contains(map, "oiseau") == 0);

    TEST_ASSERT(hashmapstr_get(map, "<EOS>") == 1);
    TEST_ASSERT(hashmapstr_get(map, "chat") == 2);
    TEST_ASSERT(hashmapstr_get(map, "chien") == 3);
    TEST_ASSERT(hashmapstr_get(map, "oiseau") == HASHMAPSTR_NOT_FOUND);

    hashmapstr_free(map);
    printf("OK\n");
}

void test_update_existing_key() {
    printf("[TEST] Mise a jour de valeur existante... ");
    struct HashMapStr *map = hashmapstr_create(16);

    hashmapstr_insert(map, "compteur", 10);
    TEST_ASSERT(hashmapstr_size(map) == 1);
    TEST_ASSERT(hashmapstr_get(map, "compteur") == 10);

    hashmapstr_insert(map, "compteur", 25);
    TEST_ASSERT(hashmapstr_size(map) == 1);
    TEST_ASSERT(hashmapstr_get(map, "compteur") == 25);

    hashmapstr_insert(map, "compteur", 0);
    TEST_ASSERT(hashmapstr_size(map) == 1);
    TEST_ASSERT(hashmapstr_get(map, "compteur") == 0);

    hashmapstr_free(map);
    printf("OK\n");
}

void test_collisions_with_small_capacity() {
    printf("[TEST] Gestion des collisions (capacite reduite a 2)... ");
    struct HashMapStr *map = hashmapstr_create(2);

    const char *words[] = {
        "pomme", "poire", "banane", "fraise", "orange",
        "kiwi", "ananas", "mangue", "cerise", "peche"
    };
    int count = sizeof(words) / sizeof(words[0]);

    for (int i = 0; i < count; i++) {
        hashmapstr_insert(map, words[i], (uint64_t)(i + 100));
        TEST_ASSERT(hashmapstr_size(map) == (size_t)(i + 1));
    }

    for (int i = 0; i < count; i++) {
        TEST_ASSERT(hashmapstr_contains(map, words[i]) == 1);
        TEST_ASSERT(hashmapstr_get(map, words[i]) == (uint64_t)(i + 100));
    }

    TEST_ASSERT(hashmapstr_contains(map, "inexistant") == 0);
    TEST_ASSERT(hashmapstr_get(map, "inexistant") == HASHMAPSTR_NOT_FOUND);

    hashmapstr_free(map);
    printf("OK\n");
}

void test_bulk_1000_entries() {
    printf("[TEST] Test de charge (1000 cles uniques)... ");
    struct HashMapStr *map = hashmapstr_create(128);
    char buffer[64];

    for (int i = 0; i < 1000; i++) {
        snprintf(buffer, sizeof(buffer), "token_%04d", i);
        hashmapstr_insert(map, buffer, (uint64_t)i);
        TEST_ASSERT(hashmapstr_size(map) == (size_t)(i + 1));
    }

    TEST_ASSERT(hashmapstr_size(map) == 1000);

    for (int i = 0; i < 1000; i++) {
        snprintf(buffer, sizeof(buffer), "token_%04d", i);
        TEST_ASSERT(hashmapstr_contains(map, buffer) == 1);
        TEST_ASSERT(hashmapstr_get(map, buffer) == (uint64_t)i);
    }

    TEST_ASSERT(hashmapstr_contains(map, "token_1000") == 0);
    TEST_ASSERT(hashmapstr_get(map, "token_1000") == HASHMAPSTR_NOT_FOUND);

    hashmapstr_free(map);
    printf("OK\n");
}

int main(void) {
    printf("===================================\n");
    printf("       Tests HashMapStr\n");
    printf("===================================\n");

    test_create();
    test_null_safety();
    test_id_zero_and_basic_ops();
    test_update_existing_key();
    test_collisions_with_small_capacity();
    test_bulk_1000_entries();

    printf("===================================\n");
    printf("Succes : %d assertions validees !\n", g_assert_count);
    printf("===================================\n");
    return 0;
}
