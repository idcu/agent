#include "idcu/common/lock.h"
#include <stdio.h>

static void test_mutex_init_destroy(void) {
    idcu_Mutex lock;
    
    int ret = idcu_mutex_init(&lock);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: Mutex init should succeed\n");
        return;
    }
    
    idcu_mutex_destroy(&lock);
    printf("Test passed: mutex_init_destroy\n");
}

static void test_mutex_lock_unlock(void) {
    idcu_Mutex lock;
    int ret = idcu_mutex_init(&lock);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: Mutex init should succeed\n");
        return;
    }
    
    ret = idcu_mutex_lock(&lock);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: Mutex lock should succeed\n");
        idcu_mutex_destroy(&lock);
        return;
    }
    
    ret = idcu_mutex_unlock(&lock);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: Mutex unlock should succeed\n");
        idcu_mutex_destroy(&lock);
        return;
    }
    
    idcu_mutex_destroy(&lock);
    printf("Test passed: mutex_lock_unlock\n");
}

static void test_rwlock_init_destroy(void) {
    idcu_RwLock lock;
    
    int ret = idcu_rwlock_init(&lock);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: RWLock init should succeed\n");
        return;
    }
    
    idcu_rwlock_destroy(&lock);
    printf("Test passed: rwlock_init_destroy\n");
}

static void test_rwlock_rdlock_wrlock_unlock(void) {
    idcu_RwLock lock;
    int ret = idcu_rwlock_init(&lock);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: RWLock init should succeed\n");
        return;
    }
    
    ret = idcu_rwlock_rdlock(&lock);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: RWLock rdlock should succeed\n");
        idcu_rwlock_destroy(&lock);
        return;
    }
    
    ret = idcu_rwlock_unlock(&lock);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: RWLock unlock should succeed\n");
        idcu_rwlock_destroy(&lock);
        return;
    }
    
    ret = idcu_rwlock_wrlock(&lock);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: RWLock wrlock should succeed\n");
        idcu_rwlock_destroy(&lock);
        return;
    }
    
    ret = idcu_rwlock_unlock(&lock);
    if (ret != IDCU_ERR_OK) {
        printf("Test failed: RWLock unlock should succeed\n");
        idcu_rwlock_destroy(&lock);
        return;
    }
    
    idcu_rwlock_destroy(&lock);
    printf("Test passed: rwlock_rdlock_wrlock_unlock\n");
}

int main(void) {
    printf("Running Lock Tests...\n\n");
    
    test_mutex_init_destroy();
    test_mutex_lock_unlock();
    test_rwlock_init_destroy();
    test_rwlock_rdlock_wrlock_unlock();
    
    printf("\nAll tests completed!\n");
    return 0;
}
