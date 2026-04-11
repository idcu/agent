#include <idcu/testframework/testframework.h>
#include <idcu/common/error_code.h>
#include <idcu/common/vector.h>
#include <idcu/common/hash_map.h>
#include <idcu/common/lock.h>
#include <idcu/common/linked_list.h>
#include <idcu/common/option.h>
#include <stdio.h>
#include <string.h>

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

int main(void) {
    return idcu_test_run_all();
}
