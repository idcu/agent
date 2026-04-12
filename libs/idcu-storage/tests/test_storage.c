#include <idcu/testframework/testframework.h>
#include <idcu/storage/storage.h>
#include <idcu/storage/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IDCU_TEST_CASE(kvstore, init_destroy) {
    idcu_KVStore store;
    int ret = idcu_kvstore_init(&store, ":memory:");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_kvstore_destroy(&store);
}

IDCU_TEST_CASE(kvstore, put_get_string) {
    idcu_KVStore store;
    idcu_kvstore_init(&store, ":memory:");
    
    int ret = idcu_kvstore_put_string(&store, "key1", "value1");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    char buffer[64];
    ret = idcu_kvstore_get_string(&store, "key1", buffer, sizeof(buffer));
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_STRING_EQUAL("value1", buffer);
    
    idcu_kvstore_destroy(&store);
}

IDCU_TEST_CASE(kvstore, put_get_int) {
    idcu_KVStore store;
    idcu_kvstore_init(&store, ":memory:");
    
    int ret = idcu_kvstore_put_int(&store, "key1", 42);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    int64_t value;
    ret = idcu_kvstore_get_int(&store, "key1", &value);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_EQUAL(42, value);
    
    idcu_kvstore_destroy(&store);
}

IDCU_TEST_CASE(kvstore, put_get_double) {
    idcu_KVStore store;
    idcu_kvstore_init(&store, ":memory:");
    
    int ret = idcu_kvstore_put_double(&store, "key1", 3.14159);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    double value;
    ret = idcu_kvstore_get_double(&store, "key1", &value);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_DOUBLE_EQUAL(3.14159, value, 0.00001);
    
    idcu_kvstore_destroy(&store);
}

IDCU_TEST_CASE(kvstore, remove) {
    idcu_KVStore store;
    idcu_kvstore_init(&store, ":memory:");
    
    idcu_kvstore_put_string(&store, "key1", "value1");
    
    int ret = idcu_kvstore_remove(&store, "key1");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    char buffer[64];
    ret = idcu_kvstore_get_string(&store, "key1", buffer, sizeof(buffer));
    IDCU_TEST_ASSERT_NOT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_kvstore_destroy(&store);
}

IDCU_TEST_CASE(kvstore, count) {
    idcu_KVStore store;
    idcu_kvstore_init(&store, ":memory:");
    
    IDCU_TEST_ASSERT_EQUAL(0, idcu_kvstore_count(&store));
    
    idcu_kvstore_put_string(&store, "key1", "value1");
    idcu_kvstore_put_string(&store, "key2", "value2");
    idcu_kvstore_put_string(&store, "key3", "value3");
    
    IDCU_TEST_ASSERT_EQUAL(3, idcu_kvstore_count(&store));
    
    idcu_kvstore_destroy(&store);
}

IDCU_TEST_CASE(kvstore, clear) {
    idcu_KVStore store;
    idcu_kvstore_init(&store, ":memory:");
    
    idcu_kvstore_put_string(&store, "key1", "value1");
    idcu_kvstore_put_string(&store, "key2", "value2");
    
    IDCU_TEST_ASSERT_EQUAL(2, idcu_kvstore_count(&store));
    
    int ret = idcu_kvstore_clear(&store);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_EQUAL(0, idcu_kvstore_count(&store));
    
    idcu_kvstore_destroy(&store);
}

IDCU_TEST_CASE(kvstore, overwrite) {
    idcu_KVStore store;
    idcu_kvstore_init(&store, ":memory:");
    
    idcu_kvstore_put_string(&store, "key1", "value1");
    
    char buffer[64];
    idcu_kvstore_get_string(&store, "key1", buffer, sizeof(buffer));
    IDCU_TEST_ASSERT_STRING_EQUAL("value1", buffer);
    
    idcu_kvstore_put_string(&store, "key1", "new_value");
    idcu_kvstore_get_string(&store, "key1", buffer, sizeof(buffer));
    IDCU_TEST_ASSERT_STRING_EQUAL("new_value", buffer);
    
    idcu_kvstore_destroy(&store);
}

IDCU_TEST_CASE(kvstore, get_nonexistent) {
    idcu_KVStore store;
    idcu_kvstore_init(&store, ":memory:");
    
    char buffer[64];
    int ret = idcu_kvstore_get_string(&store, "nonexistent", buffer, sizeof(buffer));
    IDCU_TEST_ASSERT_NOT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_kvstore_destroy(&store);
}

IDCU_TEST_CASE(kvstore, cache_stats) {
    idcu_KVStore store;
    idcu_kvstore_init(&store, ":memory:");
    
    idcu_kvstore_enable_cache(&store, 1);
    
    idcu_kvstore_put_string(&store, "key1", "value1");
    
    char buffer[64];
    idcu_kvstore_get_string(&store, "key1", buffer, sizeof(buffer));
    
    size_t hits = idcu_kvstore_get_cache_hit_count(&store);
    size_t misses = idcu_kvstore_get_cache_miss_count(&store);
    
    IDCU_TEST_ASSERT(misses > 0);
    
    idcu_kvstore_clear_cache(&store);
    idcu_kvstore_destroy(&store);
}

IDCU_TEST_CASE(sqlite, init_destroy) {
    idcu_SQLiteDB db;
    int ret = idcu_sqlite_init(&db, ":memory:");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_sqlite_destroy(&db);
}

IDCU_TEST_CASE(sqlite, execute) {
    idcu_SQLiteDB db;
    idcu_sqlite_init(&db, ":memory:");
    
    int ret = idcu_sqlite_execute(&db, "CREATE TABLE test (id INT, name TEXT);");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_sqlite_destroy(&db);
}

IDCU_TEST_CASE(sqlite, transaction) {
    idcu_SQLiteDB db;
    idcu_sqlite_init(&db, ":memory:");
    
    int ret = idcu_sqlite_execute(&db, "CREATE TABLE test (id INT, name TEXT);");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    ret = idcu_sqlite_begin_transaction(&db);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    ret = idcu_sqlite_execute(&db, "INSERT INTO test VALUES (1, 'test');");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    ret = idcu_sqlite_commit(&db);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_sqlite_destroy(&db);
}

IDCU_TEST_CASE(sqlite, rollback) {
    idcu_SQLiteDB db;
    idcu_sqlite_init(&db, ":memory:");
    
    int ret = idcu_sqlite_execute(&db, "CREATE TABLE test (id INT, name TEXT);");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    ret = idcu_sqlite_begin_transaction(&db);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    ret = idcu_sqlite_execute(&db, "INSERT INTO test VALUES (1, 'test');");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    ret = idcu_sqlite_rollback(&db);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_sqlite_destroy(&db);
}

int main(void) {
    return idcu_test_run_all();
}
