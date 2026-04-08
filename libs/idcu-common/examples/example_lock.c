#include <idcu/common/error_code.h>
#include <idcu/common/lock.h>
#include <stdio.h>

int main(void) {
    printf("=== idcu-common Locks Example ===\n\n");

    int ret;

    printf("1. Mutex Example\n");
    printf("----------------\n");

    idcu_Mutex mutex;
    ret = idcu_mutex_init(&mutex);
    if (ret != IDCU_ERR_OK) {
        printf("Mutex init failed: %s\n", idcu_err_to_str(ret));
        return 1;
    }
    printf("Mutex initialized successfully\n");

    ret = idcu_mutex_lock(&mutex);
    if (ret != IDCU_ERR_OK) {
        printf("Lock failed: %s\n", idcu_err_to_str(ret));
        idcu_mutex_destroy(&mutex);
        return 1;
    }
    printf("Lock acquired\n");

    printf("Critical section operation...\n");

    ret = idcu_mutex_unlock(&mutex);
    if (ret != IDCU_ERR_OK) {
        printf("Unlock failed: %s\n", idcu_err_to_str(ret));
        idcu_mutex_destroy(&mutex);
        return 1;
    }
    printf("Lock released\n");

    idcu_mutex_destroy(&mutex);
    printf("Mutex destroyed\n\n");

    printf("2. Read-Write Lock Example\n");
    printf("---------------------------\n");

    idcu_RwLock rwlock;
    ret = idcu_rwlock_init(&rwlock);
    if (ret != IDCU_ERR_OK) {
        printf("RwLock init failed: %s\n", idcu_err_to_str(ret));
        return 1;
    }
    printf("RwLock initialized successfully\n");

    ret = idcu_rwlock_rdlock(&rwlock);
    if (ret != IDCU_ERR_OK) {
        printf("Read lock failed: %s\n", idcu_err_to_str(ret));
        idcu_rwlock_destroy(&rwlock);
        return 1;
    }
    printf("Read lock acquired\n");

    printf("Performing read operation...\n");

    ret = idcu_rwlock_unlock(&rwlock);
    if (ret != IDCU_ERR_OK) {
        printf("Unlock failed: %s\n", idcu_err_to_str(ret));
        idcu_rwlock_destroy(&rwlock);
        return 1;
    }
    printf("Read lock released\n");

    ret = idcu_rwlock_wrlock(&rwlock);
    if (ret != IDCU_ERR_OK) {
        printf("Write lock failed: %s\n", idcu_err_to_str(ret));
        idcu_rwlock_destroy(&rwlock);
        return 1;
    }
    printf("Write lock acquired\n");

    printf("Performing write operation...\n");

    ret = idcu_rwlock_unlock(&rwlock);
    if (ret != IDCU_ERR_OK) {
        printf("Unlock failed: %s\n", idcu_err_to_str(ret));
        idcu_rwlock_destroy(&rwlock);
        return 1;
    }
    printf("Write lock released\n");

    idcu_rwlock_destroy(&rwlock);
    printf("RwLock destroyed\n\n");

    printf("=== Example Complete ===\n");

    return 0;
}
