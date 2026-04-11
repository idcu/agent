#ifndef IDCU_STORAGE_STORAGE_H
#define IDCU_STORAGE_STORAGE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/storage/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ========== KV存储API ==========
int  idcu_kvstore_init(idcu_KVStore* store, const char* path);
void idcu_kvstore_destroy(idcu_KVStore* store);
int  idcu_kvstore_put(idcu_KVStore* store, const char* key, const void* value, size_t value_size);
int  idcu_kvstore_put_string(idcu_KVStore* store, const char* key, const char* value);
int  idcu_kvstore_put_int(idcu_KVStore* store, const char* key, int64_t value);
int  idcu_kvstore_put_double(idcu_KVStore* store, const char* key, double value);
int  idcu_kvstore_get(idcu_KVStore* store, const char* key, void* buffer, size_t buffer_size, size_t* value_size);
int  idcu_kvstore_get_string(idcu_KVStore* store, const char* key, char* buffer, size_t buffer_size);
int  idcu_kvstore_get_int(idcu_KVStore* store, const char* key, int64_t* out_value);
int  idcu_kvstore_get_double(idcu_KVStore* store, const char* key, double* out_value);
int  idcu_kvstore_remove(idcu_KVStore* store, const char* key);
int  idcu_kvstore_sync(idcu_KVStore* store);
int  idcu_kvstore_clear(idcu_KVStore* store);
uint32_t idcu_kvstore_count(idcu_KVStore* store);

// ========== Cache API ==========
void idcu_kvstore_set_cache_config(idcu_KVStore* store, const idcu_KVCacheConfig* config);
void idcu_kvstore_enable_cache(idcu_KVStore* store, int enabled);
void idcu_kvstore_clear_cache(idcu_KVStore* store);
size_t idcu_kvstore_get_cache_hit_count(idcu_KVStore* store);
size_t idcu_kvstore_get_cache_miss_count(idcu_KVStore* store);

// ========== Compression API ==========
void idcu_kvstore_set_compression(idcu_KVStore* store, int enabled, idcu_StorageCompressionLevel level);

// ========== SQLite数据库API（简化版）==========
int  idcu_sqlite_init(idcu_SQLiteDB* db, const char* path);
void idcu_sqlite_destroy(idcu_SQLiteDB* db);
int  idcu_sqlite_execute(idcu_SQLiteDB* db, const char* sql);
int  idcu_sqlite_begin_transaction(idcu_SQLiteDB* db);
int  idcu_sqlite_commit(idcu_SQLiteDB* db);
int  idcu_sqlite_rollback(idcu_SQLiteDB* db);

#ifdef __cplusplus
}
#endif

#endif
