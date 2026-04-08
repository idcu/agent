#include "idcu/common/linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_passed = 0;
static int test_failed = 0;

#define TEST_ASSERT(cond, msg)                                                                     \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            printf("FAIL: %s\n", msg);                                                             \
            test_failed++;                                                                         \
        } else {                                                                                   \
            test_passed++;                                                                         \
            printf("PASS: %s\n", msg);                                                             \
        }                                                                                          \
    } while (0)

static void test_list_init_destroy(void) {
    idcu_LinkedList list;
    int ret = idcu_linked_list_init(&list, sizeof(int));
    TEST_ASSERT(ret == IDCU_ERR_OK, "list init should succeed");
    TEST_ASSERT(idcu_linked_list_size(&list) == 0, "initial size should be 0");
    TEST_ASSERT(idcu_linked_list_empty(&list) == true, "list should be empty");

    idcu_linked_list_destroy(&list);
}

static void test_list_push_pop_front(void) {
    idcu_LinkedList list;
    int val, out;

    idcu_linked_list_init(&list, sizeof(int));

    val = 10;
    int ret = idcu_linked_list_push_front(&list, &val);
    TEST_ASSERT(ret == IDCU_ERR_OK, "push front should succeed");
    TEST_ASSERT(idcu_linked_list_size(&list) == 1, "size should be 1");

    val = 20;
    ret = idcu_linked_list_push_front(&list, &val);
    TEST_ASSERT(ret == IDCU_ERR_OK, "push front again should succeed");
    TEST_ASSERT(idcu_linked_list_size(&list) == 2, "size should be 2");

    int *front = (int *)idcu_linked_list_front(&list);
    TEST_ASSERT(*front == 20, "front should be 20");

    ret = idcu_linked_list_pop_front(&list, &out);
    TEST_ASSERT(ret == IDCU_ERR_OK, "pop front should succeed");
    TEST_ASSERT(out == 20, "popped value should be 20");
    TEST_ASSERT(idcu_linked_list_size(&list) == 1, "size should be 1");

    ret = idcu_linked_list_pop_front(&list, &out);
    TEST_ASSERT(ret == IDCU_ERR_OK, "pop front again should succeed");
    TEST_ASSERT(out == 10, "popped value should be 10");
    TEST_ASSERT(idcu_linked_list_size(&list) == 0, "size should be 0");

    idcu_linked_list_destroy(&list);
}

static void test_list_push_pop_back(void) {
    idcu_LinkedList list;
    int val, out;

    idcu_linked_list_init(&list, sizeof(int));

    val = 10;
    int ret = idcu_linked_list_push_back(&list, &val);
    TEST_ASSERT(ret == IDCU_ERR_OK, "push back should succeed");

    val = 20;
    ret = idcu_linked_list_push_back(&list, &val);
    TEST_ASSERT(ret == IDCU_ERR_OK, "push back again should succeed");
    TEST_ASSERT(idcu_linked_list_size(&list) == 2, "size should be 2");

    int *back = (int *)idcu_linked_list_back(&list);
    TEST_ASSERT(*back == 20, "back should be 20");

    ret = idcu_linked_list_pop_back(&list, &out);
    TEST_ASSERT(ret == IDCU_ERR_OK, "pop back should succeed");
    TEST_ASSERT(out == 20, "popped value should be 20");

    idcu_linked_list_destroy(&list);
}

static void test_list_get(void) {
    idcu_LinkedList list;
    int vals[] = {10, 20, 30, 40, 50};

    idcu_linked_list_init(&list, sizeof(int));

    for (int i = 0; i < 5; i++) {
        idcu_linked_list_push_back(&list, &vals[i]);
    }

    int *val = (int *)idcu_linked_list_get(&list, 0);
    TEST_ASSERT(*val == 10, "index 0 should be 10");

    val = (int *)idcu_linked_list_get(&list, 2);
    TEST_ASSERT(*val == 30, "index 2 should be 30");

    val = (int *)idcu_linked_list_get(&list, 4);
    TEST_ASSERT(*val == 50, "index 4 should be 50");

    val = (int *)idcu_linked_list_get(&list, 10);
    TEST_ASSERT(val == NULL, "invalid index should return NULL");

    idcu_linked_list_destroy(&list);
}

static void test_list_insert_remove(void) {
    idcu_LinkedList list;
    int val;

    idcu_linked_list_init(&list, sizeof(int));

    val = 10;
    idcu_linked_list_push_back(&list, &val);
    val = 30;
    idcu_linked_list_push_back(&list, &val);

    val = 20;
    int ret = idcu_linked_list_insert_before(&list, 1, &val);
    TEST_ASSERT(ret == IDCU_ERR_OK, "insert before should succeed");
    TEST_ASSERT(idcu_linked_list_size(&list) == 3, "size should be 3");

    int *v = (int *)idcu_linked_list_get(&list, 1);
    TEST_ASSERT(*v == 20, "index 1 should be 20");

    ret = idcu_linked_list_remove(&list, 1);
    TEST_ASSERT(ret == IDCU_ERR_OK, "remove should succeed");
    TEST_ASSERT(idcu_linked_list_size(&list) == 2, "size should be 2");

    v = (int *)idcu_linked_list_get(&list, 1);
    TEST_ASSERT(*v == 30, "index 1 should be 30 after remove");

    idcu_linked_list_destroy(&list);
}

static void test_list_clear(void) {
    idcu_LinkedList list;
    int val;

    idcu_linked_list_init(&list, sizeof(int));

    for (int i = 0; i < 10; i++) {
        val = i;
        idcu_linked_list_push_back(&list, &val);
    }

    TEST_ASSERT(idcu_linked_list_size(&list) == 10, "size should be 10");

    idcu_linked_list_clear(&list);
    TEST_ASSERT(idcu_linked_list_size(&list) == 0, "size should be 0 after clear");
    TEST_ASSERT(idcu_linked_list_empty(&list) == true, "list should be empty after clear");

    idcu_linked_list_destroy(&list);
}

static void test_list_iterator(void) {
    idcu_LinkedList list;
    int vals[] = {1, 2, 3, 4, 5};
    idcu_LinkedListIterator iter;
    int *val;
    int count = 0;

    idcu_linked_list_init(&list, sizeof(int));

    for (int i = 0; i < 5; i++) {
        idcu_linked_list_push_back(&list, &vals[i]);
    }

    idcu_linked_list_iter_init(&iter, &list);
    while (idcu_linked_list_iter_next(&iter, (void **)&val)) {
        TEST_ASSERT(*val == vals[count], "iterator value should match");
        count++;
    }
    TEST_ASSERT(count == 5, "should iterate 5 elements");

    count = 4;
    idcu_linked_list_iter_init_reverse(&iter, &list);
    while (idcu_linked_list_iter_next(&iter, (void **)&val)) {
        TEST_ASSERT(*val == vals[count], "reverse iterator value should match");
        count--;
    }
    TEST_ASSERT(count == -1, "should iterate 5 elements in reverse");

    idcu_linked_list_iter_destroy(&iter);
    idcu_linked_list_destroy(&list);
}

int main(void) {
    printf("Running LinkedList Tests...\n\n");

    test_list_init_destroy();
    test_list_push_pop_front();
    test_list_push_pop_back();
    test_list_get();
    test_list_insert_remove();
    test_list_clear();
    test_list_iterator();

    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", test_passed);
    printf("Failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
