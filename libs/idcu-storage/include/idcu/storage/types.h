#ifndef IDCU_STORAGE_TYPES_H
#define IDCU_STORAGE_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_STORAGE_PATH_MAX         1024
#define IDCU_STORAGE_KEY_MAX          256
#define IDCU_STORAGE_VALUE_MAX        4096
#define IDCU_STORAGE_MAX_ENTRIES      10000
#define IDCU_STORAGE_SQL_MAX          8192

typedef enum {
    IDCU_STORAGE_TYPE_KV = 0,
    IDCU_STORAGE_TYPE_SQLITE
} idcu_StorageType;

// KV存储条目
typedef struct {
    char    key[IDCU_STORAGE_KEY_MAX];
    uint8_t* value;
    size_t  value_size;
} idcu_KVEntry;

// KV存储
typedef struct idcu_KVStore {
    char           path[IDCU_STORAGE_PATH_MAX];
    idcu_KVEntry*  entries;
    uint32_t       entry_count;
    uint32_t       capacity;
    idcu_Mutex     lock;
    int            dirty;
    int            initialized;
} idcu_KVStore;

// SQLite数据库（简化版）
typedef struct idcu_SQLiteDB {
    void*          db_handle;
    char           path[IDCU_STORAGE_PATH_MAX];
    idcu_Mutex     lock;
    int            in_transaction;
    int            initialized;
} idcu_SQLiteDB;

// 事务
typedef struct idcu_Transaction {
    idcu_SQLiteDB* db;
    int            active;
} idcu_Transaction;

#ifdef __cplusplus
}
#endif

#endif
