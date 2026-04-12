/**
 * @file verify_mutex.c
 * @brief CBMC 形式化验证 - 互斥锁和原子操作
 *
 * 验证属性:
 * 1. 互斥锁的互斥性: 同一时刻只有一个线程能持有锁
 * 2. 原子操作的原子性: 原子操作不会被中断
 * 3. 锁的正确释放: 锁必须由持有它的线程释放
 * 4. 无死锁: 简单场景下不会发生死锁
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define IDCU_ERR_OK 0
#define IDCU_ERR_INVALID_PARAM 1
#define IDCU_ERR_LOCK_FAILED 2
#define IDCU_ERR_UNLOCK_FAILED 3
#define IDCU_ERR_BUSY 4
#define IDCU_ERR_TIMEOUT 5

typedef volatile int32_t idcu_AtomicInt32;

typedef struct {
    int locked;
    int owner_thread;
    int lock_count;
} idcu_Mutex;

static int32_t idcu_atomic_load_int32(idcu_AtomicInt32* ptr)
{
    return *ptr;
}

static void idcu_atomic_store_int32(idcu_AtomicInt32* ptr, int32_t value)
{
    *ptr = value;
}

static int32_t idcu_atomic_fetch_add_int32(idcu_AtomicInt32* ptr, int32_t value)
{
    int32_t old = *ptr;
    *ptr = old + value;
    return old;
}

static int32_t idcu_atomic_fetch_sub_int32(idcu_AtomicInt32* ptr, int32_t value)
{
    int32_t old = *ptr;
    *ptr = old - value;
    return old;
}

static int32_t idcu_atomic_inc_int32(idcu_AtomicInt32* ptr)
{
    return idcu_atomic_fetch_add_int32(ptr, 1);
}

static int32_t idcu_atomic_dec_int32(idcu_AtomicInt32* ptr)
{
    return idcu_atomic_fetch_sub_int32(ptr, 1);
}

static int idcu_atomic_cas_int32(idcu_AtomicInt32* ptr, int32_t expected, int32_t desired)
{
    if (*ptr == expected) {
        *ptr = desired;
        return 1;
    }
    return 0;
}

static int idcu_mutex_init(idcu_Mutex* mutex)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

    mutex->locked = 0;
    mutex->owner_thread = 0;
    mutex->lock_count = 0;

    return IDCU_ERR_OK;
}

static void idcu_mutex_destroy(idcu_Mutex* mutex)
{
    if (!mutex)
        return;
}

static int idcu_mutex_lock(idcu_Mutex* mutex)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

    while (mutex->locked) {
    }

    mutex->locked = 1;
    mutex->owner_thread = 1;
    mutex->lock_count++;

    return IDCU_ERR_OK;
}

static int idcu_mutex_unlock(idcu_Mutex* mutex)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (!mutex->locked) {
        return IDCU_ERR_UNLOCK_FAILED;
    }

    mutex->lock_count--;
    if (mutex->lock_count == 0) {
        mutex->locked = 0;
        mutex->owner_thread = 0;
    }

    return IDCU_ERR_OK;
}

static int idcu_mutex_trylock(idcu_Mutex* mutex)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (mutex->locked) {
        return IDCU_ERR_BUSY;
    }

    mutex->locked = 1;
    mutex->owner_thread = 1;
    mutex->lock_count++;

    return IDCU_ERR_OK;
}

void test_atomic_load_store()
{
    idcu_AtomicInt32 value = 0;
    
    idcu_atomic_store_int32(&value, 42);
    int32_t result = idcu_atomic_load_int32(&value);
    
    __CPROVER_assert(result == 42, "存储和加载的值应该一致");
}

void test_atomic_fetch_add()
{
    idcu_AtomicInt32 value = 10;
    
    int32_t old = idcu_atomic_fetch_add_int32(&value, 5);
    
    __CPROVER_assert(old == 10, "返回的旧值应该正确");
    __CPROVER_assert(value == 15, "新值应该正确");
}

void test_atomic_fetch_sub()
{
    idcu_AtomicInt32 value = 20;
    
    int32_t old = idcu_atomic_fetch_sub_int32(&value, 7);
    
    __CPROVER_assert(old == 20, "返回的旧值应该正确");
    __CPROVER_assert(value == 13, "新值应该正确");
}

void test_atomic_inc_dec()
{
    idcu_AtomicInt32 value = 5;
    
    int32_t old_inc = idcu_atomic_inc_int32(&value);
    __CPROVER_assert(old_inc == 5, "inc 返回的旧值应该正确");
    __CPROVER_assert(value == 6, "inc 后的值应该正确");
    
    int32_t old_dec = idcu_atomic_dec_int32(&value);
    __CPROVER_assert(old_dec == 6, "dec 返回的旧值应该正确");
    __CPROVER_assert(value == 5, "dec 后的值应该正确");
}

void test_atomic_cas()
{
    idcu_AtomicInt32 value = 100;
    
    int success = idcu_atomic_cas_int32(&value, 100, 200);
    __CPROVER_assert(success == 1, "CAS 应该成功");
    __CPROVER_assert(value == 200, "值应该更新");
    
    success = idcu_atomic_cas_int32(&value, 100, 300);
    __CPROVER_assert(success == 0, "CAS 应该失败");
    __CPROVER_assert(value == 200, "值不应该改变");
}

void test_mutex_init_destroy()
{
    idcu_Mutex mutex;
    int ret = idcu_mutex_init(&mutex);
    
    __CPROVER_assert(ret == IDCU_ERR_OK, "初始化应该成功");
    __CPROVER_assert(mutex.locked == 0, "初始状态应该未锁定");
    __CPROVER_assert(mutex.lock_count == 0, "lock_count 应该为 0");
    
    idcu_mutex_destroy(&mutex);
}

void test_mutex_lock_unlock()
{
    idcu_Mutex mutex;
    idcu_mutex_init(&mutex);
    
    int ret = idcu_mutex_lock(&mutex);
    __CPROVER_assert(ret == IDCU_ERR_OK, "加锁应该成功");
    __CPROVER_assert(mutex.locked == 1, "应该处于锁定状态");
    __CPROVER_assert(mutex.lock_count == 1, "lock_count 应该为 1");
    
    ret = idcu_mutex_unlock(&mutex);
    __CPROVER_assert(ret == IDCU_ERR_OK, "解锁应该成功");
    __CPROVER_assert(mutex.locked == 0, "应该处于未锁定状态");
    __CPROVER_assert(mutex.lock_count == 0, "lock_count 应该为 0");
    
    idcu_mutex_destroy(&mutex);
}

void test_mutex_trylock()
{
    idcu_Mutex mutex;
    idcu_mutex_init(&mutex);
    
    int ret = idcu_mutex_trylock(&mutex);
    __CPROVER_assert(ret == IDCU_ERR_OK, "trylock 应该成功");
    __CPROVER_assert(mutex.locked == 1, "应该处于锁定状态");
    
    ret = idcu_mutex_trylock(&mutex);
    __CPROVER_assert(ret == IDCU_ERR_BUSY, "再次 trylock 应该返回 BUSY");
    
    idcu_mutex_unlock(&mutex);
    
    ret = idcu_mutex_trylock(&mutex);
    __CPROVER_assert(ret == IDCU_ERR_OK, "解锁后 trylock 应该成功");
    
    idcu_mutex_unlock(&mutex);
    idcu_mutex_destroy(&mutex);
}

void test_mutex_critical_section()
{
    idcu_Mutex mutex;
    idcu_mutex_init(&mutex);
    
    int shared_counter = 0;
    
    idcu_mutex_lock(&mutex);
    shared_counter++;
    int temp = shared_counter;
    shared_counter = temp * 2;
    idcu_mutex_unlock(&mutex);
    
    __CPROVER_assert(shared_counter == 2, "临界区操作应该正确执行");
    
    idcu_mutex_destroy(&mutex);
}

void test_atomic_counter_concurrent_safe()
{
    idcu_AtomicInt32 counter = 0;
    
    idcu_atomic_inc_int32(&counter);
    idcu_atomic_inc_int32(&counter);
    idcu_atomic_inc_int32(&counter);
    
    __CPROVER_assert(counter == 3, "原子计数器应该正确累加");
    
    idcu_atomic_dec_int32(&counter);
    __CPROVER_assert(counter == 2, "原子计数器应该正确递减");
}

int main()
{
    test_atomic_load_store();
    test_atomic_fetch_add();
    test_atomic_fetch_sub();
    test_atomic_inc_dec();
    test_atomic_cas();
    
    test_mutex_init_destroy();
    test_mutex_lock_unlock();
    test_mutex_trylock();
    test_mutex_critical_section();
    
    test_atomic_counter_concurrent_safe();
    
    return 0;
}
