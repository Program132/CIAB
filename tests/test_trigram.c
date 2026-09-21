#undef NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "trigram/trigram.h"

static int g_assert_count = 0;

#define TEST_ASSERT(expr) do { \
    g_assert_count++; \
    assert(expr); \
} while (0)

void test_create_and_null_safety() {
    printf("[TEST] Creation et securite NULL... ");

    struct TrigramList *list = trigramlist_create(8);
    TEST_ASSERT(list != NULL);
    TEST_ASSERT(list->capacity == 8);
    TEST_ASSERT(trigramlist_size(list) == 0);
    TEST_ASSERT(trigramlist_is_empty(list) == 1);
    TEST_ASSERT(trigramlist_is_full(list) == 0);
    TEST_ASSERT(trigramlist_get(list, 0) == NULL);
    TEST_ASSERT(trigramlist_get(list, 10) == NULL);
    TEST_ASSERT(trigramlist_predict(list, 1, 2) == 0);
    trigramlist_free(list);

    struct TrigramList *list_zero = trigramlist_create(0);
    TEST_ASSERT(list_zero != NULL);
    TEST_ASSERT(list_zero->capacity == 16);
    TEST_ASSERT(trigramlist_size(list_zero) == 0);
    trigramlist_free(list_zero);

    TEST_ASSERT(trigramlist_size(NULL) == 0);
    TEST_ASSERT(trigramlist_is_empty(NULL) == 1);
    TEST_ASSERT(trigramlist_is_full(NULL) == 1);
    TEST_ASSERT(trigramlist_get(NULL, 0) == NULL);
    TEST_ASSERT(trigramlist_predict(NULL, 10, 20) == 0);
    trigramlist_add(NULL, 1, 2, 3);
    trigramlist_sort(NULL);
    trigramlist_save_bin(NULL, "dummy.bin");
    TEST_ASSERT(trigramlist_load_bin(NULL) == NULL);
    TEST_ASSERT(trigramlist_load_bin("fichier_inexistant_xyz_123.bin") == NULL);
    trigramlist_free(NULL);

    printf("OK\n");
}

void test_basic_prediction_and_counts() {
    printf("[TEST] Ajout, comptage et prediction... ");
    struct TrigramList *list = trigramlist_create(4);

    trigramlist_add(list, 10, 20, 30);
    TEST_ASSERT(trigramlist_size(list) == 1);
    TEST_ASSERT(trigramlist_predict(list, 10, 20) == 30);

    trigramlist_add(list, 10, 20, 40);
    TEST_ASSERT(trigramlist_size(list) == 2);

    struct Trigram *t2 = trigramlist_get(list, 1);
    TEST_ASSERT(t2 != NULL);
    t2->count = 5;

    TEST_ASSERT(trigramlist_predict(list, 10, 20) == 40);

    TEST_ASSERT(trigramlist_predict(list, 99, 99) == 0);
    TEST_ASSERT(trigramlist_predict(list, 10, 99) == 0);

    trigramlist_free(list);
    printf("OK\n");
}

void test_sorting() {
    printf("[TEST] Tri lexicographique des trigrammes... ");
    struct TrigramList *list = trigramlist_create(8);

    trigramlist_add(list, 50, 10, 1);
    trigramlist_add(list, 10, 20, 30);
    trigramlist_add(list, 10, 10, 99);
    trigramlist_add(list, 10, 20, 15);
    trigramlist_add(list, 5, 1, 2);

    TEST_ASSERT(trigramlist_size(list) == 5);

    trigramlist_sort(list);

    struct Trigram *t;

    t = trigramlist_get(list, 0);
    TEST_ASSERT(t->id1 == 5 && t->id2 == 1 && t->id3 == 2);

    t = trigramlist_get(list, 1);
    TEST_ASSERT(t->id1 == 10 && t->id2 == 10 && t->id3 == 99);

    t = trigramlist_get(list, 2);
    TEST_ASSERT(t->id1 == 10 && t->id2 == 20 && t->id3 == 15);

    t = trigramlist_get(list, 3);
    TEST_ASSERT(t->id1 == 10 && t->id2 == 20 && t->id3 == 30);

    t = trigramlist_get(list, 4);
    TEST_ASSERT(t->id1 == 50 && t->id2 == 10 && t->id3 == 1);

    trigramlist_free(list);
    printf("OK\n");
}

void test_binary_serialization() {
    printf("[TEST] Sauvegarde et rechargement binaire (.bin)... ");
    const char *test_bin_file = "test_trigram_temp.bin";
    struct TrigramList *list = trigramlist_create(16);

    for (uint32_t i = 0; i < 50; i++) {
        trigramlist_add(list, i * 2, i * 2 + 1, i * 2 + 2);
        struct Trigram *t = trigramlist_get(list, i);
        t->count = i + 10;
    }

    TEST_ASSERT(trigramlist_size(list) == 50);

    trigramlist_save_bin(list, test_bin_file);

    struct TrigramList *loaded = trigramlist_load_bin(test_bin_file);
    TEST_ASSERT(loaded != NULL);
    TEST_ASSERT(trigramlist_size(loaded) == 50);

    for (uint32_t i = 0; i < 50; i++) {
        struct Trigram *orig = trigramlist_get(list, i);
        struct Trigram *copy = trigramlist_get(loaded, i);
        TEST_ASSERT(copy != NULL);
        TEST_ASSERT(copy->id1 == orig->id1);
        TEST_ASSERT(copy->id2 == orig->id2);
        TEST_ASSERT(copy->id3 == orig->id3);
        TEST_ASSERT(copy->count == orig->count);
    }

    trigramlist_free(list);
    trigramlist_free(loaded);
    remove(test_bin_file);
    printf("OK\n");
}

void test_bulk_growth_and_exhaustive_asserts() {
    printf("[TEST] Test d'envergure (500 trigrammes, realloc multiples et verification)... ");
    struct TrigramList *list = trigramlist_create(2);

    for (uint32_t i = 0; i < 500; i++) {
        trigramlist_add(list, i, i + 1, i + 2);
        TEST_ASSERT(trigramlist_size(list) == (size_t)(i + 1));
        TEST_ASSERT(trigramlist_is_empty(list) == 0);
    }

    TEST_ASSERT(trigramlist_size(list) == 500);

    for (uint32_t i = 0; i < 500; i++) {
        struct Trigram *t = trigramlist_get(list, i);
        TEST_ASSERT(t != NULL);
        TEST_ASSERT(t->id1 == i);
        TEST_ASSERT(t->id2 == i + 1);
        TEST_ASSERT(t->id3 == i + 2);
        TEST_ASSERT(t->count == 1);
        TEST_ASSERT(trigramlist_predict(list, i, i + 1) == i + 2);
    }

    TEST_ASSERT(trigramlist_get(list, 500) == NULL);
    TEST_ASSERT(trigramlist_get(list, 1000) == NULL);

    trigramlist_free(list);
    printf("OK\n");
}

int main(void) {
    printf("===================================\n");
    printf("       Tests TrigramList\n");
    printf("===================================\n");

    test_create_and_null_safety();
    test_basic_prediction_and_counts();
    test_sorting();
    test_binary_serialization();
    test_bulk_growth_and_exhaustive_asserts();

    printf("===================================\n");
    printf("Succes : %d assertions validees !\n", g_assert_count);
    printf("===================================\n");
    return 0;
}
