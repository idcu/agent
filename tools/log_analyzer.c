#include "log_analyzer.h"
#include <idcu/common/vector.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static idcu_Vector log_entries;
static char* log_buffer = NULL;
static int initialized = 0;

static idcu_LogLevel parse_level(const char* str) {
    if (strcmp(str, "TRACE") == 0 || strcmp(str, "trace") == 0) return IDCU_LOG_LEVEL_TRACE;
    if (strcmp(str, "DEBUG") == 0 || strcmp(str, "debug") == 0) return IDCU_LOG_LEVEL_DEBUG;
    if (strcmp(str, "INFO") == 0 || strcmp(str, "info") == 0) return IDCU_LOG_LEVEL_INFO;
    if (strcmp(str, "WARN") == 0 || strcmp(str, "warn") == 0 || 
        strcmp(str, "WARNING") == 0 || strcmp(str, "warning") == 0) return IDCU_LOG_LEVEL_WARN;
    if (strcmp(str, "ERROR") == 0 || strcmp(str, "error") == 0) return IDCU_LOG_LEVEL_ERROR;
    if (strcmp(str, "FATAL") == 0 || strcmp(str, "fatal") == 0) return IDCU_LOG_LEVEL_FATAL;
    return IDCU_LOG_LEVEL_INFO;
}

static const char* level_to_string(idcu_LogLevel level) {
    switch (level) {
        case IDCU_LOG_LEVEL_TRACE: return "TRACE";
        case IDCU_LOG_LEVEL_DEBUG: return "DEBUG";
        case IDCU_LOG_LEVEL_INFO: return "INFO";
        case IDCU_LOG_LEVEL_WARN: return "WARN";
        case IDCU_LOG_LEVEL_ERROR: return "ERROR";
        case IDCU_LOG_LEVEL_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

int idcu_log_analyzer_init(const char* log_file) {
    if (!log_file) {
        return -1;
    }
    
    if (initialized) {
        idcu_log_analyzer_destroy();
    }
    
    FILE* fp = fopen(log_file, "r");
    if (!fp) {
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    log_buffer = (char*)malloc(file_size + 1);
    if (!log_buffer) {
        fclose(fp);
        return -1;
    }
    
    size_t read = fread(log_buffer, 1, file_size, fp);
    log_buffer[read] = '\0';
    fclose(fp);
    
    idcu_vector_init(&log_entries, sizeof(idcu_LogEntry));
    
    char* line = log_buffer;
    while (line && *line) {
        char* next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';
        }
        
        if (*line) {
            idcu_LogEntry entry;
            memset(&entry, 0, sizeof(entry));
            entry.message = line;
            entry.message_len = strlen(line);
            entry.level = IDCU_LOG_LEVEL_INFO;
            
            char* level_str = NULL;
            if (strstr(line, "[TRACE]") || strstr(line, "TRACE:")) {
                entry.level = IDCU_LOG_LEVEL_TRACE;
            } else if (strstr(line, "[DEBUG]") || strstr(line, "DEBUG:")) {
                entry.level = IDCU_LOG_LEVEL_DEBUG;
            } else if (strstr(line, "[INFO]") || strstr(line, "INFO:")) {
                entry.level = IDCU_LOG_LEVEL_INFO;
            } else if (strstr(line, "[WARN]") || strstr(line, "WARN:") ||
                       strstr(line, "[WARNING]") || strstr(line, "WARNING:")) {
                entry.level = IDCU_LOG_LEVEL_WARN;
            } else if (strstr(line, "[ERROR]") || strstr(line, "ERROR:")) {
                entry.level = IDCU_LOG_LEVEL_ERROR;
            } else if (strstr(line, "[FATAL]") || strstr(line, "FATAL:")) {
                entry.level = IDCU_LOG_LEVEL_FATAL;
            }
            
            idcu_vector_push(&log_entries, &entry);
        }
        
        line = next_line ? next_line + 1 : NULL;
    }
    
    initialized = 1;
    return 0;
}

void idcu_log_analyzer_destroy(void) {
    if (!initialized) {
        return;
    }
    
    if (log_buffer) {
        free(log_buffer);
        log_buffer = NULL;
    }
    
    idcu_vector_destroy(&log_entries);
    initialized = 0;
}

size_t idcu_log_analyzer_get_entry_count(void) {
    if (!initialized) {
        return 0;
    }
    return log_entries.size;
}

int idcu_log_analyzer_get_entry(size_t index, idcu_LogEntry* entry) {
    if (!initialized || !entry || index >= log_entries.size) {
        return -1;
    }
    
    idcu_LogEntry* src = (idcu_LogEntry*)idcu_vector_get(&log_entries, index);
    *entry = *src;
    return 0;
}

void idcu_log_analyzer_get_stats(idcu_LogStats* stats) {
    if (!initialized || !stats) {
        return;
    }
    
    memset(stats, 0, sizeof(*stats));
    stats->total_entries = log_entries.size;
    stats->min_level = IDCU_LOG_LEVEL_FATAL;
    stats->max_level = IDCU_LOG_LEVEL_TRACE;
    
    for (size_t i = 0; i < log_entries.size; i++) {
        idcu_LogEntry* entry = (idcu_LogEntry*)idcu_vector_get(&log_entries, i);
        
        switch (entry->level) {
            case IDCU_LOG_LEVEL_TRACE: stats->trace_count++; break;
            case IDCU_LOG_LEVEL_DEBUG: stats->debug_count++; break;
            case IDCU_LOG_LEVEL_INFO: stats->info_count++; break;
            case IDCU_LOG_LEVEL_WARN: stats->warn_count++; break;
            case IDCU_LOG_LEVEL_ERROR: stats->error_count++; break;
            case IDCU_LOG_LEVEL_FATAL: stats->fatal_count++; break;
        }
        
        if (entry->level < stats->min_level) {
            stats->min_level = entry->level;
        }
        if (entry->level > stats->max_level) {
            stats->max_level = entry->level;
        }
    }
}

void idcu_log_analyzer_print_stats(void) {
    idcu_LogStats stats;
    idcu_log_analyzer_get_stats(&stats);
    
    printf("=== Log Analyzer Statistics ===\n");
    printf("Total entries:     %zu\n", stats.total_entries);
    printf("TRACE level:       %zu\n", stats.trace_count);
    printf("DEBUG level:       %zu\n", stats.debug_count);
    printf("INFO level:        %zu\n", stats.info_count);
    printf("WARN level:        %zu\n", stats.warn_count);
    printf("ERROR level:       %zu\n", stats.error_count);
    printf("FATAL level:       %zu\n", stats.fatal_count);
    printf("===============================\n");
}

size_t idcu_log_analyzer_filter_by_level(idcu_LogLevel level, idcu_LogEntry** entries, size_t max_entries) {
    if (!initialized || !entries) {
        return 0;
    }
    
    size_t count = 0;
    for (size_t i = 0; i < log_entries.size && count < max_entries; i++) {
        idcu_LogEntry* entry = (idcu_LogEntry*)idcu_vector_get(&log_entries, i);
        if (entry->level == level) {
            entries[count++] = entry;
        }
    }
    return count;
}

size_t idcu_log_analyzer_search(const char* keyword, idcu_LogEntry** entries, size_t max_entries) {
    if (!initialized || !entries || !keyword) {
        return 0;
    }
    
    size_t count = 0;
    for (size_t i = 0; i < log_entries.size && count < max_entries; i++) {
        idcu_LogEntry* entry = (idcu_LogEntry*)idcu_vector_get(&log_entries, i);
        if (strstr(entry->message, keyword)) {
            entries[count++] = entry;
        }
    }
    return count;
}

size_t idcu_log_analyzer_get_errors(idcu_LogEntry** entries, size_t max_entries) {
    if (!initialized || !entries) {
        return 0;
    }
    
    size_t count = 0;
    for (size_t i = 0; i < log_entries.size && count < max_entries; i++) {
        idcu_LogEntry* entry = (idcu_LogEntry*)idcu_vector_get(&log_entries, i);
        if (entry->level == IDCU_LOG_LEVEL_ERROR || entry->level == IDCU_LOG_LEVEL_FATAL) {
            entries[count++] = entry;
        }
    }
    return count;
}
