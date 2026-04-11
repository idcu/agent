#ifndef IDCU_TOOLS_LOG_ANALYZER_H
#define IDCU_TOOLS_LOG_ANALYZER_H

#include <idcu/common/config.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_LOG_LEVEL_TRACE,
    IDCU_LOG_LEVEL_DEBUG,
    IDCU_LOG_LEVEL_INFO,
    IDCU_LOG_LEVEL_WARN,
    IDCU_LOG_LEVEL_ERROR,
    IDCU_LOG_LEVEL_FATAL
} idcu_LogLevel;

typedef struct {
    const char* file;
    int line;
    const char* function;
    idcu_LogLevel level;
    uint64_t timestamp;
    const char* message;
    size_t message_len;
} idcu_LogEntry;

typedef struct {
    size_t total_entries;
    size_t trace_count;
    size_t debug_count;
    size_t info_count;
    size_t warn_count;
    size_t error_count;
    size_t fatal_count;
    idcu_LogLevel min_level;
    idcu_LogLevel max_level;
} idcu_LogStats;

int idcu_log_analyzer_init(const char* log_file);
void idcu_log_analyzer_destroy(void);

size_t idcu_log_analyzer_get_entry_count(void);
int idcu_log_analyzer_get_entry(size_t index, idcu_LogEntry* entry);

void idcu_log_analyzer_get_stats(idcu_LogStats* stats);
void idcu_log_analyzer_print_stats(void);

size_t idcu_log_analyzer_filter_by_level(idcu_LogLevel level, idcu_LogEntry** entries, size_t max_entries);
size_t idcu_log_analyzer_search(const char* keyword, idcu_LogEntry** entries, size_t max_entries);
size_t idcu_log_analyzer_get_errors(idcu_LogEntry** entries, size_t max_entries);

#ifdef __cplusplus
}
#endif

#endif
