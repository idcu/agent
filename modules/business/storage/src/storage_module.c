#include "idcu/log/log.h"
#include "module_def.h"
#include "storage.h"
#include <stdio.h>
#include <string.h>

static int g_initialized = 0;

static int storage_module_init() {
    IDCU_LOG_INFO("[storage_module] Initializing storage module");

    int ret = idcu_storage_init();
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("[storage_module] Failed to initialize storage service: %d", ret);
        return -1;
    }

    g_initialized = 1;
    IDCU_LOG_INFO("[storage_module] Storage module initialized successfully");
    return 0;
}

static int storage_module_run() { return 0; }

static int storage_module_stop() {
    if (g_initialized) {
        IDCU_LOG_INFO("[storage_module] Stopping storage module");
        idcu_storage_cleanup();
        g_initialized = 0;
    }
    return 0;
}

int idcu_storage_module_open(const char *db_path, idcu_StorageDatabase **db) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[storage_module] Module not initialized");
        return -1;
    }
    return idcu_storage_open(db_path, db);
}

int idcu_storage_module_close(idcu_StorageDatabase *db) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[storage_module] Module not initialized");
        return -1;
    }
    return idcu_storage_close(db);
}

int idcu_storage_module_execute(idcu_StorageDatabase *db, const char *sql) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[storage_module] Module not initialized");
        return -1;
    }
    return idcu_storage_execute(db, sql);
}

int idcu_storage_module_begin_transaction(idcu_StorageDatabase *db) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[storage_module] Module not initialized");
        return -1;
    }
    return idcu_storage_begin_transaction(db);
}

int idcu_storage_module_commit_transaction(idcu_StorageDatabase *db) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[storage_module] Module not initialized");
        return -1;
    }
    return idcu_storage_commit_transaction(db);
}

int idcu_storage_module_rollback_transaction(idcu_StorageDatabase *db) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[storage_module] Module not initialized");
        return -1;
    }
    return idcu_storage_rollback_transaction(db);
}

IDCU_REGISTER_MODULE(storage_module, IDCU_MODULE_VERSION(1, 0, 0), storage_module_init,
                     storage_module_run, storage_module_stop);
