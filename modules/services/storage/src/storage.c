#include "storage.h"
#include "idcu/log/log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IDCU_STORAGE_MAX_DBS 16
#define IDCU_STORAGE_MAX_STMTS 64
#define IDCU_STORAGE_MAX_RESULTS 32

struct idcu_StorageDatabase {
    char db_path[IDCU_STORAGE_DB_PATH_MAX];
    int in_use;
    int transaction_active;
};

struct idcu_StorageStatement {
    char sql[IDCU_STORAGE_SQL_MAX];
    int in_use;
    idcu_StorageDatabase *db;
};

struct idcu_StorageResult {
    int column_count;
    char **column_names;
    idcu_StorageValue *values;
    int in_use;
};

static idcu_StorageDatabase g_databases[IDCU_STORAGE_MAX_DBS];
static idcu_StorageStatement g_statements[IDCU_STORAGE_MAX_STMTS];
static idcu_StorageResult g_results[IDCU_STORAGE_MAX_RESULTS];
static int g_initialized = 0;

int idcu_storage_init(void) {
    if (g_initialized)
        return IDCU_ERR_OK;

    memset(g_databases, 0, sizeof(g_databases));
    memset(g_statements, 0, sizeof(g_statements));
    memset(g_results, 0, sizeof(g_results));
    g_initialized = 1;

    IDCU_LOG_INFO("[storage] Storage service initialized");
    return IDCU_ERR_OK;
}

void idcu_storage_cleanup(void) {
    if (!g_initialized)
        return;

    for (int i = 0; i < IDCU_STORAGE_MAX_DBS; i++) {
        if (g_databases[i].in_use) {
            idcu_storage_close(&g_databases[i]);
        }
    }

    g_initialized = 0;
    IDCU_LOG_INFO("[storage] Storage service cleaned up");
}

int idcu_storage_open(const char *db_path, idcu_StorageDatabase **db) {
    if (!g_initialized)
        idcu_storage_init();
    if (!db_path || !db)
        return IDCU_ERR_INVALID_PARAM;

    for (int i = 0; i < IDCU_STORAGE_MAX_DBS; i++) {
        if (!g_databases[i].in_use) {
            strncpy(g_databases[i].db_path, db_path, IDCU_STORAGE_DB_PATH_MAX - 1);
            g_databases[i].db_path[IDCU_STORAGE_DB_PATH_MAX - 1] = '\0';
            g_databases[i].in_use = 1;
            g_databases[i].transaction_active = 0;
            *db = &g_databases[i];

            IDCU_LOG_INFO("[storage] Opened database: %s", db_path);
            return IDCU_ERR_OK;
        }
    }

    IDCU_LOG_ERROR("[storage] No available database slots");
    return IDCU_ERR_NO_MEMORY;
}

int idcu_storage_close(idcu_StorageDatabase *db) {
    if (!db || !db->in_use)
        return IDCU_ERR_INVALID_PARAM;

    IDCU_LOG_INFO("[storage] Closed database: %s", db->db_path);
    memset(db, 0, sizeof(idcu_StorageDatabase));
    return IDCU_ERR_OK;
}

int idcu_storage_execute(idcu_StorageDatabase *db, const char *sql) {
    if (!db || !db->in_use || !sql)
        return IDCU_ERR_INVALID_PARAM;

    IDCU_LOG_DEBUG("[storage] Executing SQL: %s", sql);
    return IDCU_ERR_OK;
}

int idcu_storage_execute_format(idcu_StorageDatabase *db, const char *format, ...) {
    if (!db || !db->in_use || !format)
        return IDCU_ERR_INVALID_PARAM;

    char sql[IDCU_STORAGE_SQL_MAX];
    va_list args;
    va_start(args, format);
    vsnprintf(sql, sizeof(sql), format, args);
    va_end(args);

    return idcu_storage_execute(db, sql);
}

int idcu_storage_prepare(idcu_StorageDatabase *db, const char *sql, idcu_StorageStatement **stmt) {
    if (!db || !db->in_use || !sql || !stmt)
        return IDCU_ERR_INVALID_PARAM;

    for (int i = 0; i < IDCU_STORAGE_MAX_STMTS; i++) {
        if (!g_statements[i].in_use) {
            strncpy(g_statements[i].sql, sql, IDCU_STORAGE_SQL_MAX - 1);
            g_statements[i].sql[IDCU_STORAGE_SQL_MAX - 1] = '\0';
            g_statements[i].in_use = 1;
            g_statements[i].db = db;
            *stmt = &g_statements[i];

            IDCU_LOG_DEBUG("[storage] Prepared statement: %s", sql);
            return IDCU_ERR_OK;
        }
    }

    IDCU_LOG_ERROR("[storage] No available statement slots");
    return IDCU_ERR_NO_MEMORY;
}

int idcu_storage_bind_int(idcu_StorageStatement *stmt, int index, int64_t value) {
    if (!stmt || !stmt->in_use)
        return IDCU_ERR_INVALID_PARAM;
    IDCU_LOG_DEBUG("[storage] Bound int at %d: %lld", index, (long long)value);
    return IDCU_ERR_OK;
}

int idcu_storage_bind_float(idcu_StorageStatement *stmt, int index, double value) {
    if (!stmt || !stmt->in_use)
        return IDCU_ERR_INVALID_PARAM;
    IDCU_LOG_DEBUG("[storage] Bound float at %d: %f", index, value);
    return IDCU_ERR_OK;
}

int idcu_storage_bind_text(idcu_StorageStatement *stmt, int index, const char *value) {
    if (!stmt || !stmt->in_use)
        return IDCU_ERR_INVALID_PARAM;
    IDCU_LOG_DEBUG("[storage] Bound text at %d: %s", index, value);
    return IDCU_ERR_OK;
}

int idcu_storage_bind_blob(idcu_StorageStatement *stmt, int index, const void *value, size_t size) {
    if (!stmt || !stmt->in_use)
        return IDCU_ERR_INVALID_PARAM;
    IDCU_LOG_DEBUG("[storage] Bound blob at %d: %zu bytes", index, size);
    return IDCU_ERR_OK;
}

int idcu_storage_bind_null(idcu_StorageStatement *stmt, int index) {
    if (!stmt || !stmt->in_use)
        return IDCU_ERR_INVALID_PARAM;
    IDCU_LOG_DEBUG("[storage] Bound null at %d", index);
    return IDCU_ERR_OK;
}

int idcu_storage_step(idcu_StorageStatement *stmt, idcu_StorageResult **result) {
    if (!stmt || !stmt->in_use || !result)
        return IDCU_ERR_INVALID_PARAM;

    for (int i = 0; i < IDCU_STORAGE_MAX_RESULTS; i++) {
        if (!g_results[i].in_use) {
            g_results[i].column_count = 0;
            g_results[i].column_names = NULL;
            g_results[i].values = NULL;
            g_results[i].in_use = 1;
            *result = &g_results[i];

            IDCU_LOG_DEBUG("[storage] Stepped statement");
            return IDCU_ERR_OK;
        }
    }

    IDCU_LOG_ERROR("[storage] No available result slots");
    return IDCU_ERR_NO_MEMORY;
}

int idcu_storage_column_count(idcu_StorageResult *result) {
    if (!result || !result->in_use)
        return 0;
    return result->column_count;
}

const char *idcu_storage_column_name(idcu_StorageResult *result, int index) {
    if (!result || !result->in_use || index < 0 || index >= result->column_count)
        return NULL;
    return result->column_names ? result->column_names[index] : NULL;
}

idcu_StorageType idcu_storage_column_type(idcu_StorageResult *result, int index) {
    if (!result || !result->in_use || index < 0 || index >= result->column_count)
        return IDCU_STORAGE_TYPE_NULL;
    return result->values ? result->values[index].type : IDCU_STORAGE_TYPE_NULL;
}

int64_t idcu_storage_column_int(idcu_StorageResult *result, int index) {
    if (!result || !result->in_use || index < 0 || index >= result->column_count)
        return 0;
    return result->values ? result->values[index].value.int_value : 0;
}

double idcu_storage_column_float(idcu_StorageResult *result, int index) {
    if (!result || !result->in_use || index < 0 || index >= result->column_count)
        return 0.0;
    return result->values ? result->values[index].value.float_value : 0.0;
}

const char *idcu_storage_column_text(idcu_StorageResult *result, int index) {
    if (!result || !result->in_use || index < 0 || index >= result->column_count)
        return NULL;
    return result->values ? result->values[index].value.text_value : NULL;
}

const void *idcu_storage_column_blob(idcu_StorageResult *result, int index, size_t *size) {
    if (!result || !result->in_use || index < 0 || index >= result->column_count) {
        if (size)
            *size = 0;
        return NULL;
    }
    if (size)
        *size = result->values ? result->values[index].blob_size : 0;
    return result->values ? result->values[index].value.blob_value : NULL;
}

int idcu_storage_result_destroy(idcu_StorageResult *result) {
    if (!result || !result->in_use)
        return IDCU_ERR_INVALID_PARAM;

    if (result->column_names) {
        free(result->column_names);
        result->column_names = NULL;
    }
    if (result->values) {
        free(result->values);
        result->values = NULL;
    }

    memset(result, 0, sizeof(idcu_StorageResult));
    return IDCU_ERR_OK;
}

int idcu_storage_finalize(idcu_StorageStatement *stmt) {
    if (!stmt || !stmt->in_use)
        return IDCU_ERR_INVALID_PARAM;
    memset(stmt, 0, sizeof(idcu_StorageStatement));
    return IDCU_ERR_OK;
}

int64_t idcu_storage_last_insert_rowid(idcu_StorageDatabase *db) {
    if (!db || !db->in_use)
        return 0;
    return 0;
}

int idcu_storage_changes(idcu_StorageDatabase *db) {
    if (!db || !db->in_use)
        return 0;
    return 0;
}

int idcu_storage_begin_transaction(idcu_StorageDatabase *db) {
    if (!db || !db->in_use)
        return IDCU_ERR_INVALID_PARAM;
    if (db->transaction_active) {
        IDCU_LOG_WARN("[storage] Transaction already active");
        return IDCU_ERR_OK;
    }
    db->transaction_active = 1;
    IDCU_LOG_INFO("[storage] Transaction began");
    return IDCU_ERR_OK;
}

int idcu_storage_commit_transaction(idcu_StorageDatabase *db) {
    if (!db || !db->in_use)
        return IDCU_ERR_INVALID_PARAM;
    if (!db->transaction_active) {
        IDCU_LOG_WARN("[storage] No active transaction to commit");
        return IDCU_ERR_OK;
    }
    db->transaction_active = 0;
    IDCU_LOG_INFO("[storage] Transaction committed");
    return IDCU_ERR_OK;
}

int idcu_storage_rollback_transaction(idcu_StorageDatabase *db) {
    if (!db || !db->in_use)
        return IDCU_ERR_INVALID_PARAM;
    if (!db->transaction_active) {
        IDCU_LOG_WARN("[storage] No active transaction to rollback");
        return IDCU_ERR_OK;
    }
    db->transaction_active = 0;
    IDCU_LOG_INFO("[storage] Transaction rolled back");
    return IDCU_ERR_OK;
}
