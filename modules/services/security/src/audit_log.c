#include "audit_log.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

struct idcu_AuditLog {
    idcu_AuditEntry *entries;
    size_t max_entries;
    size_t current_count;
    size_t head;
    size_t tail;
    char log_path[512];
    FILE *log_file;
    int initialized;
};

static uint64_t get_current_timestamp_ms(void) {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (uli.QuadPart - 116444736000000000ULL) / 10000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}

static const char *level_strings[] = {"INFO", "WARNING", "ERROR", "CRITICAL"};

static const char *action_strings[] = {"LOGIN",         "LOGOUT",      "ACCESS",        "MODIFY",
                                       "DELETE",        "CREATE",      "EXECUTE",       "DENIED",
                                       "CONFIG_CHANGE", "MODULE_LOAD", "MODULE_UNLOAD", "CUSTOM"};

const char *idcu_audit_level_to_string(idcu_AuditLevel level) {
    if (level < 0 || level >= sizeof(level_strings) / sizeof(level_strings[0])) {
        return "UNKNOWN";
    }
    return level_strings[level];
}

const char *idcu_audit_action_to_string(idcu_AuditAction action) {
    if (action < 0 || action >= sizeof(action_strings) / sizeof(action_strings[0])) {
        return "UNKNOWN";
    }
    return action_strings[action];
}

int idcu_audit_log_init(idcu_AuditLog **audit_log, const char *log_path) {
    if (!audit_log) {
        return IDCU_ERR_INVALID_PARAM;
    }

    *audit_log = (idcu_AuditLog *)malloc(sizeof(idcu_AuditLog));
    if (!*audit_log) {
        return IDCU_ERR_NO_MEMORY;
    }

    memset(*audit_log, 0, sizeof(idcu_AuditLog));

    (*audit_log)->max_entries = IDCU_AUDIT_MAX_ENTRIES;
    (*audit_log)->entries =
        (idcu_AuditEntry *)calloc((*audit_log)->max_entries, sizeof(idcu_AuditEntry));
    if (!(*audit_log)->entries) {
        free(*audit_log);
        *audit_log = NULL;
        return IDCU_ERR_NO_MEMORY;
    }

    if (log_path) {
        strncpy((*audit_log)->log_path, log_path, sizeof((*audit_log)->log_path) - 1);
        (*audit_log)->log_path[sizeof((*audit_log)->log_path) - 1] = '\0';

        (*audit_log)->log_file = fopen(log_path, "a");
        if (!(*audit_log)->log_file) {
            IDCU_LOG_WARN("[audit_log] Failed to open audit log file: %s", log_path);
        }
    }

    (*audit_log)->current_count = 0;
    (*audit_log)->head = 0;
    (*audit_log)->tail = 0;
    (*audit_log)->initialized = 1;

    IDCU_LOG_INFO("[audit_log] Initialized with max entries: %zu", (*audit_log)->max_entries);
    return IDCU_ERR_OK;
}

void idcu_audit_log_destroy(idcu_AuditLog *audit_log) {
    if (!audit_log) {
        return;
    }

    if (audit_log->log_file) {
        fflush(audit_log->log_file);
        fclose(audit_log->log_file);
        audit_log->log_file = NULL;
    }

    if (audit_log->entries) {
        free(audit_log->entries);
        audit_log->entries = NULL;
    }

    free(audit_log);
    IDCU_LOG_INFO("[audit_log] Destroyed");
}

static void write_entry_to_file(idcu_AuditLog *audit_log, const idcu_AuditEntry *entry) {
    if (!audit_log || !audit_log->log_file || !entry) {
        return;
    }

    time_t t = (time_t)(entry->timestamp / 1000);
    struct tm *tm_info = localtime(&t);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(audit_log->log_file,
            "[%s.%03u] [%s] [%s] Event: %s, User: %s, Resource: %s, Success: %d, Session: %llu, "
            "Details: %s\n",
            time_buf, (unsigned int)(entry->timestamp % 1000),
            idcu_audit_level_to_string(entry->level), idcu_audit_action_to_string(entry->action),
            entry->event_name, entry->user_name, entry->resource, entry->success,
            (unsigned long long)entry->session_id, entry->details);

    fflush(audit_log->log_file);
}

int idcu_audit_log_log(idcu_AuditLog *audit_log, const idcu_AuditEntry *entry) {
    if (!audit_log || !audit_log->initialized || !entry) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_AuditEntry *new_entry = &audit_log->entries[audit_log->tail];
    memcpy(new_entry, entry, sizeof(idcu_AuditEntry));
    new_entry->timestamp = get_current_timestamp_ms();

    audit_log->tail = (audit_log->tail + 1) % audit_log->max_entries;

    if (audit_log->current_count >= audit_log->max_entries) {
        audit_log->head = (audit_log->head + 1) % audit_log->max_entries;
    } else {
        audit_log->current_count++;
    }

    write_entry_to_file(audit_log, new_entry);

    IDCU_LOG_DEBUG("[audit_log] Logged entry: %s", entry->event_name);
    return IDCU_ERR_OK;
}

int idcu_audit_log_log_simple(idcu_AuditLog *audit_log, idcu_AuditLevel level,
                              idcu_AuditAction action, const char *event_name,
                              const char *user_name, const char *resource, const char *details,
                              int success) {
    if (!audit_log || !audit_log->initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_AuditEntry entry;
    memset(&entry, 0, sizeof(entry));

    entry.level = level;
    entry.action = action;
    entry.success = success;
    entry.session_id = 0;

    if (event_name) {
        strncpy(entry.event_name, event_name, IDCU_AUDIT_MAX_EVENT_NAME - 1);
        entry.event_name[IDCU_AUDIT_MAX_EVENT_NAME - 1] = '\0';
    }

    if (user_name) {
        strncpy(entry.user_name, user_name, IDCU_AUDIT_MAX_USER_NAME - 1);
        entry.user_name[IDCU_AUDIT_MAX_USER_NAME - 1] = '\0';
    }

    if (resource) {
        strncpy(entry.resource, resource, IDCU_AUDIT_MAX_RESOURCE - 1);
        entry.resource[IDCU_AUDIT_MAX_RESOURCE - 1] = '\0';
    }

    if (details) {
        strncpy(entry.details, details, IDCU_AUDIT_MAX_DETAILS - 1);
        entry.details[IDCU_AUDIT_MAX_DETAILS - 1] = '\0';
    }

    return idcu_audit_log_log(audit_log, &entry);
}

int idcu_audit_log_query(idcu_AuditLog *audit_log, uint64_t start_time, uint64_t end_time,
                         idcu_AuditEntry **entries, size_t *count) {
    if (!audit_log || !audit_log->initialized || !entries || !count) {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t result_count = 0;
    size_t idx = audit_log->head;

    for (size_t i = 0; i < audit_log->current_count; i++) {
        idcu_AuditEntry *entry = &audit_log->entries[idx];
        if (entry->timestamp >= start_time && entry->timestamp <= end_time) {
            result_count++;
        }
        idx = (idx + 1) % audit_log->max_entries;
    }

    if (result_count == 0) {
        *entries = NULL;
        *count = 0;
        return IDCU_ERR_OK;
    }

    *entries = (idcu_AuditEntry *)malloc(result_count * sizeof(idcu_AuditEntry));
    if (!*entries) {
        return IDCU_ERR_NO_MEMORY;
    }

    idx = audit_log->head;
    size_t copy_idx = 0;
    for (size_t i = 0; i < audit_log->current_count; i++) {
        idcu_AuditEntry *entry = &audit_log->entries[idx];
        if (entry->timestamp >= start_time && entry->timestamp <= end_time) {
            memcpy(&(*entries)[copy_idx], entry, sizeof(idcu_AuditEntry));
            copy_idx++;
        }
        idx = (idx + 1) % audit_log->max_entries;
    }

    *count = result_count;
    IDCU_LOG_DEBUG("[audit_log] Queried %zu entries", result_count);
    return IDCU_ERR_OK;
}

void idcu_audit_log_free_entries(idcu_AuditEntry *entries, size_t count) {
    if (entries) {
        free(entries);
    }
}

int idcu_audit_log_set_max_entries(idcu_AuditLog *audit_log, size_t max_entries) {
    if (!audit_log || !audit_log->initialized || max_entries == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_AuditEntry *new_entries = (idcu_AuditEntry *)calloc(max_entries, sizeof(idcu_AuditEntry));
    if (!new_entries) {
        return IDCU_ERR_NO_MEMORY;
    }

    size_t copy_count =
        (audit_log->current_count < max_entries) ? audit_log->current_count : max_entries;
    size_t idx = audit_log->head;
    for (size_t i = 0; i < copy_count; i++) {
        memcpy(&new_entries[i], &audit_log->entries[idx], sizeof(idcu_AuditEntry));
        idx = (idx + 1) % audit_log->max_entries;
    }

    free(audit_log->entries);
    audit_log->entries = new_entries;
    audit_log->max_entries = max_entries;
    audit_log->current_count = copy_count;
    audit_log->head = 0;
    audit_log->tail = copy_count % max_entries;

    IDCU_LOG_INFO("[audit_log] Set max entries to: %zu", max_entries);
    return IDCU_ERR_OK;
}

int idcu_audit_log_flush(idcu_AuditLog *audit_log) {
    if (!audit_log || !audit_log->initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (audit_log->log_file) {
        fflush(audit_log->log_file);
    }

    IDCU_LOG_DEBUG("[audit_log] Flushed");
    return IDCU_ERR_OK;
}

int idcu_audit_log_clear(idcu_AuditLog *audit_log) {
    if (!audit_log || !audit_log->initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(audit_log->entries, 0, audit_log->max_entries * sizeof(idcu_AuditEntry));
    audit_log->current_count = 0;
    audit_log->head = 0;
    audit_log->tail = 0;

    IDCU_LOG_INFO("[audit_log] Cleared");
    return IDCU_ERR_OK;
}
