#ifndef IDCU_SECURITY_AUDIT_LOG_H
#define IDCU_SECURITY_AUDIT_LOG_H

#include "idcu/common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_AUDIT_MAX_EVENT_NAME 64
#define IDCU_AUDIT_MAX_USER_NAME 64
#define IDCU_AUDIT_MAX_RESOURCE 256
#define IDCU_AUDIT_MAX_DETAILS 1024
#define IDCU_AUDIT_MAX_ENTRIES 10000

typedef enum {
    IDCU_AUDIT_LEVEL_INFO = 0,
    IDCU_AUDIT_LEVEL_WARNING,
    IDCU_AUDIT_LEVEL_ERROR,
    IDCU_AUDIT_LEVEL_CRITICAL
} idcu_AuditLevel;

typedef enum {
    IDCU_AUDIT_ACTION_LOGIN = 0,
    IDCU_AUDIT_ACTION_LOGOUT,
    IDCU_AUDIT_ACTION_ACCESS,
    IDCU_AUDIT_ACTION_MODIFY,
    IDCU_AUDIT_ACTION_DELETE,
    IDCU_AUDIT_ACTION_CREATE,
    IDCU_AUDIT_ACTION_EXECUTE,
    IDCU_AUDIT_ACTION_DENIED,
    IDCU_AUDIT_ACTION_CONFIG_CHANGE,
    IDCU_AUDIT_ACTION_MODULE_LOAD,
    IDCU_AUDIT_ACTION_MODULE_UNLOAD,
    IDCU_AUDIT_ACTION_CUSTOM
} idcu_AuditAction;

typedef struct {
    uint64_t timestamp;
    idcu_AuditLevel level;
    idcu_AuditAction action;
    char event_name[IDCU_AUDIT_MAX_EVENT_NAME];
    char user_name[IDCU_AUDIT_MAX_USER_NAME];
    char resource[IDCU_AUDIT_MAX_RESOURCE];
    char details[IDCU_AUDIT_MAX_DETAILS];
    int success;
    uint64_t session_id;
} idcu_AuditEntry;

typedef struct idcu_AuditLog idcu_AuditLog;

int idcu_audit_log_init(idcu_AuditLog** audit_log, const char* log_path);
void idcu_audit_log_destroy(idcu_AuditLog* audit_log);

int idcu_audit_log_log(idcu_AuditLog* audit_log, const idcu_AuditEntry* entry);
int idcu_audit_log_log_simple(idcu_AuditLog* audit_log, idcu_AuditLevel level, 
                              idcu_AuditAction action, const char* event_name,
                              const char* user_name, const char* resource,
                              const char* details, int success);

int idcu_audit_log_query(idcu_AuditLog* audit_log, uint64_t start_time, uint64_t end_time,
                         idcu_AuditEntry** entries, size_t* count);
void idcu_audit_log_free_entries(idcu_AuditEntry* entries, size_t count);

int idcu_audit_log_set_max_entries(idcu_AuditLog* audit_log, size_t max_entries);
int idcu_audit_log_flush(idcu_AuditLog* audit_log);
int idcu_audit_log_clear(idcu_AuditLog* audit_log);

const char* idcu_audit_level_to_string(idcu_AuditLevel level);
const char* idcu_audit_action_to_string(idcu_AuditAction action);

#endif // IDCU_SECURITY_AUDIT_LOG_H
