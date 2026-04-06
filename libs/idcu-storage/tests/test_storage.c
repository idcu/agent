#include "idcu/storage/storage.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static void test_storage_init_cleanup(void) {
    int ret = idcu_storage_init();
    if (ret != 0) {
        printf("test_storage_init_cleanup: FAIL - idcu_storage_init failed\n");
        return;
    }
    idcu_storage_cleanup();
    printf("test_storage_init_cleanup: PASS\n");
}

static void test_storage_open_close(void) {
    idcu_StorageDatabase* db;
    int ret = idcu_storage_open(":memory:", &db);
    if (ret != 0) {
        printf("test_storage_open_close: FAIL - idcu_storage_open failed\n");
        return;
    }
    ret = idcu_storage_close(db);
    if (ret != 0) {
        printf("test_storage_open_close: FAIL - idcu_storage_close failed\n");
        return;
    }
    printf("test_storage_open_close: PASS\n");
}

static void test_storage_execute(void) {
    idcu_StorageDatabase* db;
    idcu_storage_open(":memory:", &db);
    
    int ret = idcu_storage_execute(db, "CREATE TABLE test (id INT, name TEXT)");
    if (ret != 0) {
        printf("test_storage_execute: FAIL - idcu_storage_execute failed\n");
        idcu_storage_close(db);
        return;
    }
    
    idcu_storage_close(db);
    printf("test_storage_execute: PASS\n");
}

static void test_storage_transaction(void) {
    idcu_StorageDatabase* db;
    idcu_storage_open(":memory:", &db);
    
    int ret = idcu_storage_begin_transaction(db);
    if (ret != 0) {
        printf("test_storage_transaction: FAIL - idcu_storage_begin_transaction failed\n");
        idcu_storage_close(db);
        return;
    }
    
    ret = idcu_storage_commit_transaction(db);
    if (ret != 0) {
        printf("test_storage_transaction: FAIL - idcu_storage_commit_transaction failed\n");
        idcu_storage_close(db);
        return;
    }
    
    idcu_storage_close(db);
    printf("test_storage_transaction: PASS\n");
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    printf("=== Storage Tests ===\n");
    
    test_storage_init_cleanup();
    test_storage_open_close();
    test_storage_execute();
    test_storage_transaction();
    
    printf("=== All Tests Completed ===\n");
    
    return 0;
}
