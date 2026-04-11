# idcu-storage API Documentation

Key-value storage library with multiple backend support.

## KV Store

```c
typedef struct idcu_KVStore idcu_KVStore;
```

### KV Store Functions

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

## SQLite Database

```c
typedef struct idcu_SQLiteDB idcu_SQLiteDB;
```

### SQLite Functions

```c
int idcu_sqlite_init(idcu_SQLiteDB* db, const char* path);
void idcu_sqlite_destroy(idcu_SQLiteDB* db);

int idcu_sqlite_execute(idcu_SQLiteDB* db, const char* sql);

int idcu_sqlite_begin_transaction(idcu_SQLiteDB* db);
int idcu_sqlite_commit(idcu_SQLiteDB* db);
int idcu_sqlite_rollback(idcu_SQLiteDB* db);
```

## Constants

```c
#define IDCU_STORAGE_PATH_MAX 1024
#define IDCU_STORAGE_KEY_MAX 256
#define IDCU_STORAGE_VALUE_MAX (64 * 1024)
#define IDCU_STORAGE_MAX_ENTRIES 65536
```

## Example - KV Store

```c
#include <idcu/storage/storage.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    idcu_KVStore store;
    idcu_kvstore_init(&store, "mydb.json");
    
    // Put values
    idcu_kvstore_put_string(&store, "name", "John Doe");
    idcu_kvstore_put_int(&store, "age", 30);
    idcu_kvstore_put_double(&store, "score", 95.5);
    
    printf("Entry count: %u\n", idcu_kvstore_count(&store));
    
    // Get values
    char name[256];
    if (idcu_kvstore_get_string(&store, "name", name, sizeof(name)) == IDCU_ERR_OK) {
        printf("Name: %s\n", name);
    }
    
    int64_t age;
    if (idcu_kvstore_get_int(&store, "age", &age) == IDCU_ERR_OK) {
        printf("Age: %" PRId64 "\n", age);
    }
    
    // Remove a value
    idcu_kvstore_remove(&store, "score");
    
    // Sync to disk
    idcu_kvstore_sync(&store);
    
    idcu_kvstore_destroy(&store);
    return 0;
}
```

## Example - Binary Data

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
    
    // Store binary data
    MyData data = {
        .id = 123,
        .name = "Test Data",
        .value = 45.67
    };
    
    idcu_kvstore_put(&store, "mydata", &data, sizeof(data));
    
    // Retrieve binary data
    MyData retrieved;
    size_t actual_size;
    if (idcu_kvstore_get(&store, "mydata", &retrieved, sizeof(retrieved), &actual_size) == IDCU_ERR_OK) {
        printf("ID: %d\n", retrieved.id);
        printf("Name: %s\n", retrieved.name);
        printf("Value: %f\n", retrieved.value);
    }
    
    idcu_kvstore_destroy(&store);
    return 0;
}
```
