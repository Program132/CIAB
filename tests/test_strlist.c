#undef NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "strlist/strlist.h"

static int g_assert_count = 0;

#define TEST_ASSERT(expr) do { \
    g_assert_count++; \
    assert(expr); \
} while (0)

void test_create() {
    printf("[TEST] Creation et proprietes initiales... ");
    struct StrList *list = strlist_create("premier");
    TEST_ASSERT(list != NULL);
    TEST_ASSERT(list->head != NULL);
    TEST_ASSERT(strlist_size(list) == 1);
    TEST_ASSERT(strlist_is_empty(list) == 0);
    TEST_ASSERT(strcmp(strlist_get(list, 0), "premier") == 0);
    TEST_ASSERT(strlist_get(list, 1) == NULL);
    TEST_ASSERT(strlist_get(list, 2) == NULL);
    TEST_ASSERT(strlist_get(list, -1) == NULL);
    TEST_ASSERT(strlist_find(list, "premier") == 0);
    TEST_ASSERT(strlist_find(list, "inconnu") == -1);
    strlist_free(list);
    printf("OK\n");
}

void test_create_empty() {
    printf("[TEST] Liste vide et cas limites... ");
    struct StrList *list = strlist_create_empty();
    TEST_ASSERT(list != NULL);
    TEST_ASSERT(list->head == NULL);
    TEST_ASSERT(strlist_size(list) == 0);
    TEST_ASSERT(strlist_is_empty(list) == 1);
    TEST_ASSERT(strlist_get(list, 0) == NULL);
    TEST_ASSERT(strlist_get(list, 1) == NULL);
    TEST_ASSERT(strlist_get(list, -1) == NULL);
    TEST_ASSERT(strlist_get(list, -99) == NULL);
    TEST_ASSERT(strlist_find(list, "test") == -1);
    TEST_ASSERT(strlist_find(list, "") == -1);
    TEST_ASSERT(strlist_find(list, NULL) == -1);
    strlist_free(list);
    printf("OK\n");
}

void test_null_safety() {
    printf("[TEST] Robustesse face aux pointeurs NULL... ");
    TEST_ASSERT(strlist_size(NULL) == 0);
    TEST_ASSERT(strlist_is_empty(NULL) == 1);
    TEST_ASSERT(strlist_get(NULL, 0) == NULL);
    TEST_ASSERT(strlist_get(NULL, -1) == NULL);
    TEST_ASSERT(strlist_get(NULL, 10) == NULL);
    TEST_ASSERT(strlist_find(NULL, "a") == -1);
    TEST_ASSERT(strlist_find(NULL, NULL) == -1);
    strlist_add(NULL, "invalide");
    strlist_free(NULL);
    printf("OK\n");
}

void test_add_and_get() {
    printf("[TEST] Ajout sequentiel et acces par index... ");
    struct StrList *list = strlist_create_empty();
    TEST_ASSERT(strlist_size(list) == 0);

    strlist_add(list, "alpha");
    TEST_ASSERT(strlist_size(list) == 1);
    TEST_ASSERT(strlist_is_empty(list) == 0);
    TEST_ASSERT(strcmp(strlist_get(list, 0), "alpha") == 0);
    TEST_ASSERT(strlist_get(list, 1) == NULL);

    strlist_add(list, "beta");
    TEST_ASSERT(strlist_size(list) == 2);
    TEST_ASSERT(strcmp(strlist_get(list, 0), "alpha") == 0);
    TEST_ASSERT(strcmp(strlist_get(list, 1), "beta") == 0);
    TEST_ASSERT(strlist_get(list, 2) == NULL);

    strlist_add(list, "gamma");
    TEST_ASSERT(strlist_size(list) == 3);
    TEST_ASSERT(strcmp(strlist_get(list, 0), "alpha") == 0);
    TEST_ASSERT(strcmp(strlist_get(list, 1), "beta") == 0);
    TEST_ASSERT(strcmp(strlist_get(list, 2), "gamma") == 0);
    TEST_ASSERT(strlist_get(list, 3) == NULL);
    TEST_ASSERT(strlist_get(list, -1) == NULL);

    strlist_free(list);
    printf("OK\n");
}

void test_special_strings() {
    printf("[TEST] Chaines vides, doublons et caracteres speciaux... ");
    struct StrList *list = strlist_create("");
    TEST_ASSERT(strlist_size(list) == 1);
    TEST_ASSERT(strcmp(strlist_get(list, 0), "") == 0);
    TEST_ASSERT(strlist_find(list, "") == 0);

    strlist_add(list, "   ");
    strlist_add(list, "\n\t\r");
    strlist_add(list, "doublon");
    strlist_add(list, "autre");
    strlist_add(list, "doublon");

    TEST_ASSERT(strlist_size(list) == 6);
    TEST_ASSERT(strcmp(strlist_get(list, 1), "   ") == 0);
    TEST_ASSERT(strcmp(strlist_get(list, 2), "\n\t\r") == 0);
    TEST_ASSERT(strcmp(strlist_get(list, 3), "doublon") == 0);
    TEST_ASSERT(strcmp(strlist_get(list, 4), "autre") == 0);
    TEST_ASSERT(strcmp(strlist_get(list, 5), "doublon") == 0);

    TEST_ASSERT(strlist_find(list, "doublon") == 3);
    TEST_ASSERT(strlist_find(list, "autre") == 4);
    TEST_ASSERT(strlist_find(list, "\n\t\r") == 2);
    TEST_ASSERT(strlist_find(list, "inexistant") == -1);

    strlist_free(list);
    printf("OK\n");
}

void test_bulk_100_elements() {
    printf("[TEST] Test d'envergure (100 elements et verification exhaustive)... ");
    struct StrList *list = strlist_create_empty();
    char buffers[100][32];

    for (int i = 0; i < 100; i++) {
        snprintf(buffers[i], sizeof(buffers[i]), "element_%03d", i);
        strlist_add(list, buffers[i]);
        TEST_ASSERT(strlist_size(list) == i + 1);
        TEST_ASSERT(strlist_is_empty(list) == 0);
    }

    TEST_ASSERT(strlist_size(list) == 100);

    for (int i = 0; i < 100; i++) {
        const char *val = strlist_get(list, i);
        TEST_ASSERT(val != NULL);
        TEST_ASSERT(strcmp(val, buffers[i]) == 0);
        TEST_ASSERT(strlist_find(list, buffers[i]) == i);
    }

    TEST_ASSERT(strlist_get(list, -1) == NULL);
    TEST_ASSERT(strlist_get(list, -100) == NULL);
    TEST_ASSERT(strlist_get(list, 100) == NULL);
    TEST_ASSERT(strlist_get(list, 101) == NULL);
    TEST_ASSERT(strlist_get(list, 500) == NULL);
    TEST_ASSERT(strlist_find(list, "element_999") == -1);

    strlist_free(list);
    printf("OK\n");
}

int main(void) {
    printf("===================================\n");
    printf("       Tests StrList exhaustifs\n");
    printf("===================================\n");

    test_create();
    test_create_empty();
    test_null_safety();
    test_add_and_get();
    test_special_strings();
    test_bulk_100_elements();

    printf("===================================\n");
    printf("Succes : %d assertions validees !\n", g_assert_count);
    printf("===================================\n");
    return 0;
}
