#ifndef IDCU_STORAGE_STORAGE_H
#define IDCU_STORAGE_STORAGE_H

#include "idcu/common/error_code.h"

#include <stddef.h>
#include <stdint.h>

#define IDCU_STORAGE_DB_PATH_MAX 512
#define IDCU_STORAGE_SQL_MAX     4096
#define IDCU_STORAGE_VALUE_MAX   4096

typedef struct idcu_StorageDatabase  idcu_StorageDatabase;
typedef struct idcu_StorageStatement idcu_StorageStatement;
typedef struct idcu_StorageResult    idcu_StorageResult;

typedef enum
{
    IDCU_STORAGE_TYPE_NULL = 0,
    IDCU_STORAGE_TYPE_INTEGER,
    IDCU_STORAGE_TYPE_FLOAT,
    IDCU_STORAGE_TYPE_TEXT,
    IDCU_STORAGE_TYPE_BLOB
} idcu_StorageType;

typedef struct
{
    idcu_StorageType type;
    union
    {
        int64_t int_value;
        double  float_value;
        char*   text_value;
        void*   blob_value;
    } value;
    size_t blob_size;
} idcu_StorageValue;

int  idcu_storage_init(void);
void idcu_storage_cleanup(void);

int idcu_storage_open(const char* db_path, idcu_StorageDatabase** db);
int idcu_storage_close(idcu_StorageDatabase* db);

int idcu_storage_execute(idcu_StorageDatabase* db, const char* sql);
int idcu_storage_execute_format(idcu_StorageDatabase* db, const char* format, ...);

int idcu_storage_prepare(idcu_StorageDatabase* db, const char* sql, idcu_StorageStatement** stmt);
int idcu_storage_bind_int(idcu_StorageStatement* stmt, int index, int64_t value);
int idcu_storage_bind_float(idcu_StorageStatement* stmt, int index, double value);
int idcu_storage_bind_text(idcu_StorageStatement* stmt, int index, const char* value);
int idcu_storage_bind_blob(idcu_StorageStatement* stmt, int index, const void* value, size_t size);
int idcu_storage_bind_null(idcu_StorageStatement* stmt, int index);

int              idcu_storage_step(idcu_StorageStatement* stmt, idcu_StorageResult** result);
int              idcu_storage_column_count(idcu_StorageResult* result);
const char*      idcu_storage_column_name(idcu_StorageResult* result, int index);
idcu_StorageType idcu_storage_column_type(idcu_StorageResult* result, int index);
int64_t          idcu_storage_column_int(idcu_StorageResult* result, int index);
double           idcu_storage_column_float(idcu_StorageResult* result, int index);
const char*      idcu_storage_column_text(idcu_StorageResult* result, int index);
const void*      idcu_storage_column_blob(idcu_StorageResult* result, int index, size_t* size);

int idcu_storage_result_destroy(idcu_StorageResult* result);
int idcu_storage_finalize(idcu_StorageStatement* stmt);

int64_t idcu_storage_last_insert_rowid(idcu_StorageDatabase* db);
int     idcu_storage_changes(idcu_StorageDatabase* db);
int     idcu_storage_begin_transaction(idcu_StorageDatabase* db);
int     idcu_storage_commit_transaction(idcu_StorageDatabase* db);
int     idcu_storage_rollback_transaction(idcu_StorageDatabase* db);

#endif  // IDCU_STORAGE_STORAGE_H
