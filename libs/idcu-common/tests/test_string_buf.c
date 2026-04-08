#include "idcu/common/string_buf.h"
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

static void test_strbuf_init_destroy(void) {
    idcu_StringBuf buf;
    int ret = idcu_strbuf_init(&buf, 64);
    TEST_ASSERT(ret == IDCU_ERR_OK, "init should succeed");
    TEST_ASSERT(idcu_strbuf_size(&buf) == 0, "initial size should be 0");
    TEST_ASSERT(idcu_strbuf_empty(&buf) == 1, "should be empty");

    const char *data = idcu_strbuf_data(&buf);
    TEST_ASSERT(data != NULL, "data should not be NULL");
    TEST_ASSERT(strcmp(data, "") == 0, "should be empty string");

    idcu_strbuf_destroy(&buf);
}

static void test_strbuf_append(void) {
    idcu_StringBuf buf;
    idcu_strbuf_init(&buf, 32);

    int ret = idcu_strbuf_append(&buf, "Hello");
    TEST_ASSERT(ret == IDCU_ERR_OK, "append should succeed");
    TEST_ASSERT(strcmp(idcu_strbuf_data(&buf), "Hello") == 0, "should be 'Hello'");
    TEST_ASSERT(idcu_strbuf_size(&buf) == 5, "size should be 5");

    ret = idcu_strbuf_append(&buf, " World");
    TEST_ASSERT(ret == IDCU_ERR_OK, "append again should succeed");
    TEST_ASSERT(strcmp(idcu_strbuf_data(&buf), "Hello World") == 0, "should be 'Hello World'");

    idcu_strbuf_destroy(&buf);
}

static void test_strbuf_append_char(void) {
    idcu_StringBuf buf;
    idcu_strbuf_init(&buf, 32);

    int ret = idcu_strbuf_append_char(&buf, 'a');
    TEST_ASSERT(ret == IDCU_ERR_OK, "append char should succeed");

    ret = idcu_strbuf_append_char(&buf, 'b');
    TEST_ASSERT(ret == IDCU_ERR_OK, "append char again should succeed");

    ret = idcu_strbuf_append_char(&buf, 'c');
    TEST_ASSERT(ret == IDCU_ERR_OK, "append char third time should succeed");

    TEST_ASSERT(strcmp(idcu_strbuf_data(&buf), "abc") == 0, "should be 'abc'");

    idcu_strbuf_destroy(&buf);
}

static void test_strbuf_append_int(void) {
    idcu_StringBuf buf;
    idcu_strbuf_init(&buf, 32);

    int ret = idcu_strbuf_append_int(&buf, 12345);
    TEST_ASSERT(ret == IDCU_ERR_OK, "append int should succeed");
    TEST_ASSERT(strcmp(idcu_strbuf_data(&buf), "12345") == 0, "should be '12345'");

    idcu_strbuf_clear(&buf);

    ret = idcu_strbuf_append_int(&buf, -6789);
    TEST_ASSERT(ret == IDCU_ERR_OK, "append negative int should succeed");
    TEST_ASSERT(strcmp(idcu_strbuf_data(&buf), "-6789") == 0, "should be '-6789'");

    idcu_strbuf_destroy(&buf);
}

static void test_strbuf_append_format(void) {
    idcu_StringBuf buf;
    idcu_strbuf_init(&buf, 64);

    int ret = idcu_strbuf_append_format(&buf, "Name: %s, Age: %d", "Alice", 30);
    TEST_ASSERT(ret == IDCU_ERR_OK, "append format should succeed");
    TEST_ASSERT(strcmp(idcu_strbuf_data(&buf), "Name: Alice, Age: 30") == 0, "format should work");

    idcu_strbuf_destroy(&buf);
}

static void test_strbuf_insert(void) {
    idcu_StringBuf buf;
    idcu_strbuf_init(&buf, 32);

    idcu_strbuf_append(&buf, "Hello World");

    int ret = idcu_strbuf_insert(&buf, 5, " beautiful");
    TEST_ASSERT(ret == IDCU_ERR_OK, "insert should succeed");
    TEST_ASSERT(strcmp(idcu_strbuf_data(&buf), "Hello beautiful World") == 0, "insert should work");

    idcu_strbuf_destroy(&buf);
}

static void test_strbuf_remove(void) {
    idcu_StringBuf buf;
    idcu_strbuf_init(&buf, 32);

    idcu_strbuf_append(&buf, "HelloXXXWorld");

    int ret = idcu_strbuf_remove(&buf, 5, 3);
    TEST_ASSERT(ret == IDCU_ERR_OK, "remove should succeed");
    TEST_ASSERT(strcmp(idcu_strbuf_data(&buf), "HelloWorld") == 0, "remove should work");

    idcu_strbuf_destroy(&buf);
}

static void test_strbuf_clear(void) {
    idcu_StringBuf buf;
    idcu_strbuf_init(&buf, 32);

    idcu_strbuf_append(&buf, "Some long text here");
    TEST_ASSERT(idcu_strbuf_size(&buf) > 0, "should have content");

    int ret = idcu_strbuf_clear(&buf);
    TEST_ASSERT(ret == IDCU_ERR_OK, "clear should succeed");
    TEST_ASSERT(idcu_strbuf_size(&buf) == 0, "size should be 0");
    TEST_ASSERT(strcmp(idcu_strbuf_data(&buf), "") == 0, "should be empty");

    idcu_strbuf_destroy(&buf);
}

static void test_strbuf_compare(void) {
    idcu_StringBuf buf;
    idcu_strbuf_init(&buf, 32);

    idcu_strbuf_append(&buf, "test string");

    int ret = idcu_strbuf_compare(&buf, "test string");
    TEST_ASSERT(ret == 0, "compare equal should return 0");

    ret = idcu_strbuf_compare(&buf, "test");
    TEST_ASSERT(ret > 0, "compare longer should return >0");

    ret = idcu_strbuf_compare(&buf, "test string longer");
    TEST_ASSERT(ret < 0, "compare shorter should return <0");

    idcu_strbuf_destroy(&buf);
}

static void test_strbuf_find(void) {
    idcu_StringBuf buf;
    idcu_strbuf_init(&buf, 64);

    idcu_strbuf_append(&buf, "Hello World, Hello Again");

    int pos = idcu_strbuf_find(&buf, "World", 0);
    TEST_ASSERT(pos == 6, "should find 'World' at position 6");

    pos = idcu_strbuf_find(&buf, "Hello", 7);
    TEST_ASSERT(pos == 13, "should find second 'Hello' at position 13");

    pos = idcu_strbuf_find(&buf, "NotFound", 0);
    TEST_ASSERT(pos == -1, "should not find 'NotFound'");

    idcu_strbuf_destroy(&buf);
}

static void test_strbuf_replace(void) {
    idcu_StringBuf buf;
    idcu_strbuf_init(&buf, 64);

    idcu_strbuf_append(&buf, "Hello old, old world");

    int count = idcu_strbuf_replace(&buf, "old", "new");
    TEST_ASSERT(count == 2, "should replace 2 occurrences");
    TEST_ASSERT(strcmp(idcu_strbuf_data(&buf), "Hello new, new world") == 0, "replace should work");

    idcu_strbuf_destroy(&buf);
}

static void test_strbuf_detach(void) {
    idcu_StringBuf buf;
    idcu_strbuf_init(&buf, 32);

    idcu_strbuf_append(&buf, "Detach this string");

    char *detached = idcu_strbuf_detach(&buf);
    TEST_ASSERT(detached != NULL, "detach should return non-NULL");
    TEST_ASSERT(strcmp(detached, "Detach this string") == 0, "detached string should match");

    TEST_ASSERT(idcu_strbuf_data(&buf) == NULL, "buf data should be NULL after detach");

    free(detached);
}

int main(void) {
    printf("Running StringBuf Tests...\n\n");

    test_strbuf_init_destroy();
    test_strbuf_append();
    test_strbuf_append_char();
    test_strbuf_append_int();
    test_strbuf_append_format();
    test_strbuf_insert();
    test_strbuf_remove();
    test_strbuf_clear();
    test_strbuf_compare();
    test_strbuf_find();
    test_strbuf_replace();
    test_strbuf_detach();

    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", test_passed);
    printf("Failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
