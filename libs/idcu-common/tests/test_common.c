#include <idcu/testframework/testframework.h>
#include <idcu/common/error_code.h>
#include <idcu/common/vector.h>
#include <idcu/common/hash_map.h>
#include <idcu/common/lock.h>
#include <idcu/common/linked_list.h>
#include <idcu/common/option.h>
#include <idcu/common/deadlock_detector.h>
#include <idcu/common/security.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#define THREAD_HANDLE HANDLE
#define THREAD_RET DWORD WINAPI
#define THREAD_FUNC_ARG LPVOID
#else
#include <pthread.h>
#include <unistd.h>
#define THREAD_HANDLE pthread_t
#define THREAD_RET void*
#define THREAD_FUNC_ARG void*
#endif

static int g_counter = 0;
static idcu_Mutex g_test_mutex;
static volatile int g_thread_done = 0;

THREAD_RET thread_increment_counter(THREAD_FUNC_ARG arg)
{
    (void)arg;
    for (int i = 0; i < 10000; i++) {
        idcu_mutex_lock(&g_test_mutex);
        g_counter++;
        idcu_mutex_unlock(&g_test_mutex);
    }
    g_thread_done = 1;
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

IDCU_TEST_CASE(mutex, concurrent_access) {
    g_counter = 0;
    g_thread_done = 0;
    int ret = idcu_mutex_init(&g_test_mutex);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
#ifdef _WIN32
    HANDLE thread = CreateThread(NULL, 0, thread_increment_counter, NULL, 0, NULL);
    IDCU_TEST_ASSERT(thread != NULL);
    
    for (int i = 0; i < 10000; i++) {
        idcu_mutex_lock(&g_test_mutex);
        g_counter++;
        idcu_mutex_unlock(&g_test_mutex);
    }
    
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
#else
    pthread_t thread;
    pthread_create(&thread, NULL, thread_increment_counter, NULL);
    
    for (int i = 0; i < 10000; i++) {
        idcu_mutex_lock(&g_test_mutex);
        g_counter++;
        idcu_mutex_unlock(&g_test_mutex);
    }
    
    pthread_join(thread, NULL);
#endif
    
    IDCU_TEST_ASSERT_EQUAL(20000, g_counter);
    
    idcu_mutex_destroy(&g_test_mutex);
}

IDCU_TEST_CASE(mutex, timedlock) {
    idcu_Mutex mutex;
    int ret = idcu_mutex_init(&mutex);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    ret = idcu_mutex_lock(&mutex);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    ret = idcu_mutex_timedlock(&mutex, 100);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_TIMEOUT, ret);
    
    idcu_mutex_unlock(&mutex);
    
    ret = idcu_mutex_timedlock(&mutex, 100);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_mutex_unlock(&mutex);
    idcu_mutex_destroy(&mutex);
}

#ifdef IDCU_DEBUG
IDCU_TEST_CASE(mutex, hold_time_stats) {
    idcu_Mutex mutex;
    int ret = idcu_mutex_init(&mutex);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    ret = idcu_mutex_lock(&mutex);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
#ifdef _WIN32
    Sleep(10);
#else
    usleep(10000);
#endif
    
    ret = idcu_mutex_unlock(&mutex);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    uint64_t total_hold = idcu_mutex_get_total_hold_time(&mutex);
    uint64_t max_hold = idcu_mutex_get_max_hold_time(&mutex);
    
    IDCU_TEST_ASSERT(total_hold >= 10);
    IDCU_TEST_ASSERT(max_hold >= 10);
    
    idcu_mutex_destroy(&mutex);
}
#endif

IDCU_TEST_CASE(deadlock_detector, init_destroy) {
    idcu_DeadlockDetector* detector = NULL;
    int ret = idcu_deadlock_detector_init(&detector);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT(detector != NULL);
    
    idcu_deadlock_detector_destroy(detector);
}

IDCU_TEST_CASE(deadlock_detector, register_mutex) {
    idcu_DeadlockDetector* detector = NULL;
    int ret = idcu_deadlock_detector_init(&detector);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_Mutex mutex1, mutex2;
    idcu_mutex_init(&mutex1);
    idcu_mutex_init(&mutex2);
    
    ret = idcu_deadlock_detector_register_mutex(detector, &mutex1, "mutex1");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    ret = idcu_deadlock_detector_register_mutex(detector, &mutex2, "mutex2");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_deadlock_detector_unregister_mutex(detector, &mutex1);
    idcu_deadlock_detector_unregister_mutex(detector, &mutex2);
    
    idcu_mutex_destroy(&mutex1);
    idcu_mutex_destroy(&mutex2);
    idcu_deadlock_detector_destroy(detector);
}

IDCU_TEST_CASE(error_code, to_str) {
    const char* str = idcu_err_to_str(IDCU_ERR_OK);
    IDCU_TEST_ASSERT_STRING_EQUAL("Success", str);
    
    str = idcu_err_to_str(IDCU_ERR_INVALID_PARAM);
    IDCU_TEST_ASSERT_STRING_EQUAL("Invalid parameter", str);
    
    str = idcu_err_to_str(IDCU_ERR_NO_MEMORY);
    IDCU_TEST_ASSERT_STRING_EQUAL("Out of memory", str);
}

IDCU_TEST_CASE(error_code, last_error) {
    idcu_err_set_last_error(IDCU_ERR_GENERAL, "test error", __FILE__, __LINE__);
    const idcu_ErrorInfo* info = idcu_err_get_last_error();
    IDCU_TEST_ASSERT(info != NULL);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_GENERAL, info->error_code);
    
    idcu_err_clear_last_error();
    info = idcu_err_get_last_error();
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, info->error_code);
}

IDCU_TEST_CASE(error_code, is_ok) {
    IDCU_TEST_ASSERT(idcu_err_is_ok(IDCU_ERR_OK));
    IDCU_TEST_ASSERT(!idcu_err_is_ok(IDCU_ERR_GENERAL));
}

IDCU_TEST_CASE(error_code, error_chain) {
    idcu_err_set_last_error(IDCU_ERR_INVALID_PARAM, "invalid parameter value", __FILE__, __LINE__);
    idcu_ErrorInfo* cause = idcu_err_clone_error(idcu_err_get_last_error());
    
    idcu_err_set_last_error_with_cause(IDCU_ERR_CONFIG_LOAD, "failed to load config", 
                                        __FILE__, __LINE__, cause);
    
    const idcu_ErrorInfo* info = idcu_err_get_last_error();
    IDCU_TEST_ASSERT(info != NULL);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_CONFIG_LOAD, info->error_code);
    IDCU_TEST_ASSERT(idcu_err_has_cause(info));
    
    const idcu_ErrorInfo* cause_info = idcu_err_get_cause(info);
    IDCU_TEST_ASSERT(cause_info != NULL);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_PARAM, cause_info->error_code);
    
    idcu_err_free_error(cause);
    idcu_err_clear_last_error();
}

IDCU_TEST_CASE(error_code, format_error) {
    idcu_err_set_last_error(IDCU_ERR_GENERAL, "test error", __FILE__, __LINE__);
    
    char buffer[512];
    int len = idcu_err_format_error(idcu_err_get_last_error(), buffer, sizeof(buffer));
    
    IDCU_TEST_ASSERT(len > 0);
    IDCU_TEST_ASSERT(strstr(buffer, "General error") != NULL);
    IDCU_TEST_ASSERT(strstr(buffer, "test error") != NULL);
    
    idcu_err_clear_last_error();
}

IDCU_TEST_CASE(error_code, format_error_chain) {
    idcu_err_set_last_error(IDCU_ERR_INVALID_PARAM, "invalid parameter value", __FILE__, __LINE__);
    idcu_ErrorInfo* cause = idcu_err_clone_error(idcu_err_get_last_error());
    
    idcu_err_set_last_error_with_cause(IDCU_ERR_CONFIG_LOAD, "failed to load config", 
                                        __FILE__, __LINE__, cause);
    
    char buffer[1024];
    int len = idcu_err_format_error_chain(idcu_err_get_last_error(), buffer, sizeof(buffer));
    
    IDCU_TEST_ASSERT(len > 0);
    IDCU_TEST_ASSERT(strstr(buffer, "Config load failed") != NULL);
    IDCU_TEST_ASSERT(strstr(buffer, "Invalid parameter") != NULL);
    
    idcu_err_free_error(cause);
    idcu_err_clear_last_error();
}

IDCU_TEST_CASE(error_code, clone_and_free) {
    idcu_err_set_last_error(IDCU_ERR_NO_MEMORY, "out of memory", __FILE__, __LINE__);
    
    const idcu_ErrorInfo* original = idcu_err_get_last_error();
    idcu_ErrorInfo* cloned = idcu_err_clone_error(original);
    
    IDCU_TEST_ASSERT(cloned != NULL);
    IDCU_TEST_ASSERT_EQUAL(original->error_code, cloned->error_code);
    IDCU_TEST_ASSERT_STRING_EQUAL(original->context, cloned->context);
    IDCU_TEST_ASSERT_EQUAL(original->line, cloned->line);
    
    idcu_err_free_error(cloned);
    idcu_err_clear_last_error();
}

IDCU_TEST_CASE(vector, init_destroy) {
    idcu_Vector vec;
    int ret = idcu_vector_init(&vec, sizeof(int), 4);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_EQUAL(0, idcu_vector_size(&vec));
    IDCU_TEST_ASSERT(idcu_vector_empty(&vec));
    
    idcu_vector_destroy(&vec);
}

IDCU_TEST_CASE(vector, push_back) {
    idcu_Vector vec;
    int ret = idcu_vector_init(&vec, sizeof(int), 4);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    int val = 42;
    ret = idcu_vector_push_back(&vec, &val);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_EQUAL(1, idcu_vector_size(&vec));
    
    int* get_val = (int*)idcu_vector_get(&vec, 0);
    IDCU_TEST_ASSERT(get_val != NULL);
    IDCU_TEST_ASSERT_EQUAL(42, *get_val);
    
    idcu_vector_destroy(&vec);
}

IDCU_TEST_CASE(vector, pop_back) {
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 4);
    
    int val1 = 10, val2 = 20;
    idcu_vector_push_back(&vec, &val1);
    idcu_vector_push_back(&vec, &val2);
    
    int popped;
    int ret = idcu_vector_pop_back(&vec, &popped);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_EQUAL(20, popped);
    IDCU_TEST_ASSERT_EQUAL(1, idcu_vector_size(&vec));
    
    idcu_vector_destroy(&vec);
}

IDCU_TEST_CASE(vector, insert_remove) {
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 4);
    
    int val1 = 10, val2 = 20, val3 = 30;
    idcu_vector_push_back(&vec, &val1);
    idcu_vector_push_back(&vec, &val3);
    
    int ret = idcu_vector_insert(&vec, 1, &val2);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_EQUAL(3, idcu_vector_size(&vec));
    
    int* get_val = (int*)idcu_vector_get(&vec, 1);
    IDCU_TEST_ASSERT_EQUAL(20, *get_val);
    
    ret = idcu_vector_remove(&vec, 1);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_EQUAL(2, idcu_vector_size(&vec));
    
    get_val = (int*)idcu_vector_get(&vec, 1);
    IDCU_TEST_ASSERT_EQUAL(30, *get_val);
    
    idcu_vector_destroy(&vec);
}

IDCU_TEST_CASE(vector, set) {
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 4);
    
    int val1 = 10, val2 = 20;
    idcu_vector_push_back(&vec, &val1);
    
    int ret = idcu_vector_set(&vec, 0, &val2);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    int* get_val = (int*)idcu_vector_get(&vec, 0);
    IDCU_TEST_ASSERT_EQUAL(20, *get_val);
    
    idcu_vector_destroy(&vec);
}

IDCU_TEST_CASE(vector, clear) {
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 4);
    
    int val = 42;
    idcu_vector_push_back(&vec, &val);
    idcu_vector_push_back(&vec, &val);
    
    idcu_vector_clear(&vec);
    IDCU_TEST_ASSERT(idcu_vector_empty(&vec));
    IDCU_TEST_ASSERT_EQUAL(0, idcu_vector_size(&vec));
    
    idcu_vector_destroy(&vec);
}

IDCU_TEST_CASE(hash_map, init_destroy) {
    idcu_HashMap map;
    int ret = idcu_hash_map_init(&map, 8, sizeof(int));
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_EQUAL(0, idcu_hash_map_size(&map));
    IDCU_TEST_ASSERT(idcu_hash_map_empty(&map));
    
    idcu_hash_map_destroy(&map);
}

IDCU_TEST_CASE(hash_map, put_get) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, 8, sizeof(int));
    
    int val = 42;
    int ret = idcu_hash_map_put(&map, "key1", &val);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_EQUAL(1, idcu_hash_map_size(&map));
    
    IDCU_TEST_ASSERT(idcu_hash_map_contains(&map, "key1"));
    
    int get_val;
    ret = idcu_hash_map_get(&map, "key1", &get_val);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_EQUAL(42, get_val);
    
    idcu_hash_map_destroy(&map);
}

IDCU_TEST_CASE(hash_map, remove) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, 8, sizeof(int));
    
    int val = 42;
    idcu_hash_map_put(&map, "key1", &val);
    
    int ret = idcu_hash_map_remove(&map, "key1");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT(idcu_hash_map_empty(&map));
    IDCU_TEST_ASSERT(!idcu_hash_map_contains(&map, "key1"));
    
    idcu_hash_map_destroy(&map);
}

IDCU_TEST_CASE(hash_map, clear) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, 8, sizeof(int));
    
    int val = 42;
    idcu_hash_map_put(&map, "key1", &val);
    idcu_hash_map_put(&map, "key2", &val);
    
    idcu_hash_map_clear(&map);
    IDCU_TEST_ASSERT(idcu_hash_map_empty(&map));
    
    idcu_hash_map_destroy(&map);
}

IDCU_TEST_CASE(hash_map, multiple) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, 8, sizeof(int));
    
    int val1 = 10, val2 = 20, val3 = 30;
    idcu_hash_map_put(&map, "key1", &val1);
    idcu_hash_map_put(&map, "key2", &val2);
    idcu_hash_map_put(&map, "key3", &val3);
    
    IDCU_TEST_ASSERT_EQUAL(3, idcu_hash_map_size(&map));
    
    int get_val;
    idcu_hash_map_get(&map, "key2", &get_val);
    IDCU_TEST_ASSERT_EQUAL(20, get_val);
    
    idcu_hash_map_destroy(&map);
}

IDCU_TEST_CASE(hash_map, iterator) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, 8, sizeof(int));
    
    int val1 = 10, val2 = 20;
    idcu_hash_map_put(&map, "key1", &val1);
    idcu_hash_map_put(&map, "key2", &val2);
    
    int count = 0;
    idcu_HashMapIterator iter;
    const char* key;
    int* value;
    
    IDCU_HASH_MAP_FOR_EACH(&map, key, value, iter) {
        count++;
    }
    
    IDCU_TEST_ASSERT_EQUAL(2, count);
    
    idcu_hash_map_destroy(&map);
}

IDCU_TEST_CASE(mutex, init_destroy) {
    idcu_Mutex mutex;
    int ret = idcu_mutex_init(&mutex);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_mutex_destroy(&mutex);
}

IDCU_TEST_CASE(mutex, lock_unlock) {
    idcu_Mutex mutex;
    idcu_mutex_init(&mutex);
    
    int ret = idcu_mutex_lock(&mutex);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    ret = idcu_mutex_unlock(&mutex);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_mutex_destroy(&mutex);
}

IDCU_TEST_CASE(mutex, trylock) {
    idcu_Mutex mutex;
    idcu_mutex_init(&mutex);
    
    int ret = idcu_mutex_trylock(&mutex);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_mutex_unlock(&mutex);
    idcu_mutex_destroy(&mutex);
}

IDCU_TEST_CASE(option, basic) {
    idcu_Option opt;
    int val = 42;
    
    idcu_option_some(&opt, &val, sizeof(int));
    IDCU_TEST_ASSERT(idcu_option_is_some(&opt));
    
    int* get_val = (int*)idcu_option_value(&opt);
    IDCU_TEST_ASSERT(get_val != NULL);
    IDCU_TEST_ASSERT_EQUAL(42, *get_val);
    
    idcu_option_none(&opt);
    IDCU_TEST_ASSERT(idcu_option_is_none(&opt));
}

IDCU_TEST_CASE(security, secure_zero) {
    uint8_t buffer[16];
    memset(buffer, 0xFF, sizeof(buffer));
    
    idcu_secure_zero(buffer, sizeof(buffer));
    
    for (size_t i = 0; i < sizeof(buffer); i++) {
        IDCU_TEST_ASSERT_EQUAL(0, buffer[i]);
    }
}

IDCU_TEST_CASE(security, memcmp_constant) {
    uint8_t a[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t b[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t c[8] = {1, 2, 3, 4, 5, 6, 7, 9};
    
    IDCU_TEST_ASSERT_EQUAL(0, idcu_memcmp_constant(a, b, 8));
    IDCU_TEST_ASSERT(idcu_memcmp_constant(a, c, 8) != 0);
    IDCU_TEST_ASSERT_EQUAL(-1, idcu_memcmp_constant(NULL, b, 8));
}

IDCU_TEST_CASE(security, safe_add_uint64) {
    uint64_t result;
    
    IDCU_TEST_ASSERT(idcu_safe_add_uint64(100, 200, &result));
    IDCU_TEST_ASSERT_EQUAL(300, result);
    
    IDCU_TEST_ASSERT(!idcu_safe_add_uint64(UINT64_MAX, 1, &result));
}

IDCU_TEST_CASE(security, safe_sub_uint64) {
    uint64_t result;
    
    IDCU_TEST_ASSERT(idcu_safe_sub_uint64(200, 100, &result));
    IDCU_TEST_ASSERT_EQUAL(100, result);
    
    IDCU_TEST_ASSERT(!idcu_safe_sub_uint64(100, 200, &result));
}

IDCU_TEST_CASE(security, safe_mul_uint64) {
    uint64_t result;
    
    IDCU_TEST_ASSERT(idcu_safe_mul_uint64(100, 200, &result));
    IDCU_TEST_ASSERT_EQUAL(20000, result);
    
    IDCU_TEST_ASSERT(!idcu_safe_mul_uint64(UINT64_MAX, 2, &result));
}

IDCU_TEST_CASE(security, safe_add_size_t) {
    size_t result;
    
    IDCU_TEST_ASSERT(idcu_safe_add_size_t(100, 200, &result));
    IDCU_TEST_ASSERT_EQUAL(300, result);
    
    IDCU_TEST_ASSERT(!idcu_safe_add_size_t(SIZE_MAX, 1, &result));
}

IDCU_TEST_CASE(security, safe_add_int64) {
    int64_t result;
    
    IDCU_TEST_ASSERT(idcu_safe_add_int64(100, 200, &result));
    IDCU_TEST_ASSERT_EQUAL(300, result);
    
    IDCU_TEST_ASSERT(!idcu_safe_add_int64(INT64_MAX, 1, &result));
    IDCU_TEST_ASSERT(!idcu_safe_add_int64(INT64_MIN, -1, &result));
}

IDCU_TEST_CASE(security, safe_memcpy) {
    uint8_t src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t dst[16];
    
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, idcu_safe_memcpy(dst, sizeof(dst), src, sizeof(src)));
    IDCU_TEST_ASSERT(memcmp(dst, src, sizeof(src)) == 0);
    
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_BUFFER_TOO_SMALL, idcu_safe_memcpy(dst, 4, src, 8));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_PARAM, idcu_safe_memcpy(NULL, sizeof(dst), src, 8));
}

IDCU_TEST_CASE(security, safe_strcpy) {
    char dst[16];
    
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, idcu_safe_strcpy(dst, sizeof(dst), "hello"));
    IDCU_TEST_ASSERT_STRING_EQUAL("hello", dst);
    
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_BUFFER_TOO_SMALL, idcu_safe_strcpy(dst, 4, "hello world"));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_PARAM, idcu_safe_strcpy(NULL, sizeof(dst), "hello"));
}

IDCU_TEST_CASE(security, safe_strcat) {
    char dst[32] = "hello";
    
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, idcu_safe_strcat(dst, sizeof(dst), " world"));
    IDCU_TEST_ASSERT_STRING_EQUAL("hello world", dst);
}

IDCU_TEST_CASE(security, safe_strlen) {
    IDCU_TEST_ASSERT_EQUAL(5, idcu_safe_strlen("hello", 100));
    IDCU_TEST_ASSERT_EQUAL(5, idcu_safe_strlen("hello", 5));
    IDCU_TEST_ASSERT_EQUAL(0, idcu_safe_strlen(NULL, 100));
}

IDCU_TEST_CASE(security, validate_input) {
    uint8_t buffer[16];
    
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, idcu_validate_input(buffer, 8, 16));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_PARAM, idcu_validate_input(NULL, 8, 16));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OUT_OF_RANGE, idcu_validate_input(buffer, 32, 16));
}

IDCU_TEST_CASE(security, check_macros) {
    int value = 42;
    int* ptr = &value;
    
    IDCU_TEST_ASSERT(!IDCU_CHECK_NULL(ptr));
    IDCU_TEST_ASSERT(IDCU_CHECK_NULL(NULL));
    
    IDCU_TEST_ASSERT(IDCU_CHECK_RANGE(42, 0, 100));
    IDCU_TEST_ASSERT(!IDCU_CHECK_RANGE(150, 0, 100));
    
    IDCU_TEST_ASSERT(IDCU_CHECK_SIZE(50, 100));
    IDCU_TEST_ASSERT(!IDCU_CHECK_SIZE(150, 100));
    
    IDCU_TEST_ASSERT(IDCU_CHECK_PTR_AND_SIZE(ptr, 50, 100));
    IDCU_TEST_ASSERT(!IDCU_CHECK_PTR_AND_SIZE(NULL, 50, 100));
}

int main(void) {
    return idcu_test_run_all();
}
