#include <stdio.h>
#include <idcu/common/lock.h>
#include <idcu/common/error_code.h>

int main(void) {
    printf("=== idcu-common 锁示�?===\n\n");

    int ret;

    printf("1. 互斥锁演示\n");
    printf("----------------\n");

    idcu_Mutex mutex;
    ret = idcu_mutex_init(&mutex);
    if (ret != IDCU_ERR_OK) {
        printf("互斥锁初始化失败: %s\n", idcu_err_to_str(ret));
        return 1;
    }
    printf("互斥锁初始化成功\n");

    ret = idcu_mutex_lock(&mutex);
    if (ret != IDCU_ERR_OK) {
        printf("加锁失败: %s\n", idcu_err_to_str(ret));
        idcu_mutex_destroy(&mutex);
        return 1;
    }
    printf("加锁成功\n");

    printf("临界区操作中...\n");

    ret = idcu_mutex_unlock(&mutex);
    if (ret != IDCU_ERR_OK) {
        printf("解锁失败: %s\n", idcu_err_to_str(ret));
        idcu_mutex_destroy(&mutex);
        return 1;
    }
    printf("解锁成功\n");

    idcu_mutex_destroy(&mutex);
    printf("互斥锁已销毁\n\n");

    printf("2. 读写锁演示\n");
    printf("----------------\n");

    idcu_RwLock rwlock;
    ret = idcu_rwlock_init(&rwlock);
    if (ret != IDCU_ERR_OK) {
        printf("读写锁初始化失败: %s\n", idcu_err_to_str(ret));
        return 1;
    }
    printf("读写锁初始化成功\n");

    ret = idcu_rwlock_rdlock(&rwlock);
    if (ret != IDCU_ERR_OK) {
        printf("读锁失败: %s\n", idcu_err_to_str(ret));
        idcu_rwlock_destroy(&rwlock);
        return 1;
    }
    printf("读锁获取成功\n");

    printf("执行读操�?..\n");

    ret = idcu_rwlock_unlock(&rwlock);
    if (ret != IDCU_ERR_OK) {
        printf("解锁失败: %s\n", idcu_err_to_str(ret));
        idcu_rwlock_destroy(&rwlock);
        return 1;
    }
    printf("读锁释放成功\n");

    ret = idcu_rwlock_wrlock(&rwlock);
    if (ret != IDCU_ERR_OK) {
        printf("写锁失败: %s\n", idcu_err_to_str(ret));
        idcu_rwlock_destroy(&rwlock);
        return 1;
    }
    printf("写锁获取成功\n");

    printf("执行写操�?..\n");

    ret = idcu_rwlock_unlock(&rwlock);
    if (ret != IDCU_ERR_OK) {
        printf("解锁失败: %s\n", idcu_err_to_str(ret));
        idcu_rwlock_destroy(&rwlock);
        return 1;
    }
    printf("写锁释放成功\n");

    idcu_rwlock_destroy(&rwlock);
    printf("读写锁已销毁\n\n");

    printf("=== 示例完成 ===\n");

    return 0;
}
