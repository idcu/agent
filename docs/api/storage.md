# idcu-storage API 文档

支持多种后端的键值存储库。

## KV 存储

```c
typedef struct idcu_KVStore idcu_KVStore;
```

### KV 存储函数

```c
int idcu_kvstore_init(idcu_KVStore* store, const char* path);
void idcu_kvstore_destroy(idcu_KVStore* store);

int idcu_kvstore_put(idcu_KVStore* store, const char* key, const void* value, size_t value_size);
int idcu_kvstore_put_string(idcu_KVStore* store, const char* key, const char* value);
int idcu_kvstore_put_int(idcu_KVStore* store, const char* key, int64_t value);
int idcu_kvstore_put_double(idcu_KVStore* store, const char* key, double value);

int idcu_kvstore_get(idcu_KVStore* store, const char* key, void* buffer, size_t buffer_size, size_t* value_size);
int idcu_kvstore_get_string(idcu_KVStore* store, const char* key, char* buffer, size_t buffer_size);
int idcu_kvstore_get_int(idcu_KVStore* store, const char* key, int64_t* out_value);
int idcu_kvstore_get_double(idcu_KVStore* store, const char* key, double* out_value);

int idcu_kvstore_remove(idcu_KVStore* store, const char* key);
int idcu_kvstore_clear(idcu_KVStore* store);

int idcu_kvstore_sync(idcu_KVStore* store);
uint32_t idcu_kvstore_count(idcu_KVStore* store);
```

## SQLite 数据库

```c
typedef struct idcu_SQLiteDB idcu_SQLiteDB;
```

### SQLite 函数

```c
int idcu_sqlite_init(idcu_SQLiteDB* db, const char* path);
void idcu_sqlite_destroy(idcu_SQLiteDB* db);

int idcu_sqlite_execute(idcu_SQLiteDB* db, const char* sql);

int idcu_sqlite_begin_transaction(idcu_SQLiteDB* db);
int idcu_sqlite_commit(idcu_SQLiteDB* db);
int idcu_sqlite_rollback(idcu_SQLiteDB* db);
```

## 常量

```c
#define IDCU_STORAGE_PATH_MAX 1024
#define IDCU_STORAGE_KEY_MAX 256
#define IDCU_STORAGE_VALUE_MAX (64 * 1024)
#define IDCU_STORAGE_MAX_ENTRIES 65536
```

## 示例 - KV 存储

```c
#include <idcu/storage/storage.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    idcu_KVStore store;
    idcu_kvstore_init(&store, "mydb.json");
    
    // 放入值
    idcu_kvstore_put_string(&store, "name", "John Doe");
    idcu_kvstore_put_int(&store, "age", 30);
    idcu_kvstore_put_double(&store, "score", 95.5);
    
    printf("条目数: %u\n", idcu_kvstore_count(&store));
    
    // 获取值
    char name[256];
    if (idcu_kvstore_get_string(&store, "name", name, sizeof(name)) == IDCU_ERR_OK) {
        printf("名称: %s\n", name);
    }
    
    int64_t age;
    if (idcu_kvstore_get_int(&store, "age", &age) == IDCU_ERR_OK) {
        printf("年龄: %" PRId64 "\n", age);
    }
    
    // 移除一个值
    idcu_kvstore_remove(&store, "score");
    
    // 同步到磁盘
    idcu_kvstore_sync(&store);
    
    idcu_kvstore_destroy(&store);
    return 0;
}
```

## 示例 - 二进制数据

```c
#include <idcu/storage/storage.h>
#include <stdio.h>

typedef struct {
    int id;
    char name[64];
    double value;
} MyData;

int main(void) {
    idcu_KVStore store;
    idcu_kvstore_init(&store, "binary_db.json");
    
    // 存储二进制数据
    MyData data = {
        .id = 123,
        .name = "Test Data",
        .value = 45.67
    };
    
    idcu_kvstore_put(&store, "mydata", &data, sizeof(data));
    
    // 检索二进制数据
    MyData retrieved;
    size_t actual_size;
    if (idcu_kvstore_get(&store, "mydata", &retrieved, sizeof(retrieved), &actual_size) == IDCU_ERR_OK) {
        printf("ID: %d\n", retrieved.id);
        printf("名称: %s\n", retrieved.name);
        printf("值: %f\n", retrieved.value);
    }
    
    idcu_kvstore_destroy(&store);
    return 0;
}
```
