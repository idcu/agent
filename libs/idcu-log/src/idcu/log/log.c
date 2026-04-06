#include "idcu/log/log.h"
#include "idcu/common/config.h"
#include "idcu/common/error_code.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>

static FILE* log_file = NULL;
static idcu_LogLevel log_level = IDCU_LOG_INFO;
static idcu_LogConfig log_config;
static uint64_t current_file_size = 0;
static time_t last_rotate_time = 0;
static idcu_LogOutput log_output = IDCU_LOG_OUTPUT_CONSOLE | IDCU_LOG_OUTPUT_FILE;

static const char* log_level_names[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static int check_rotate_needed(void) {
    if (log_config.rotate_policy == IDCU_LOG_ROTATE_NONE || !log_file) {
        return 0;
    }

    int need_rotate = 0;

    if (log_config.rotate_policy & IDCU_LOG_ROTATE_SIZE) {
        if (current_file_size >= log_config.max_file_size) {
            need_rotate = 1;
        }
    }

    if (log_config.rotate_policy & IDCU_LOG_ROTATE_TIME) {
        time_t now = time(NULL);
        if (difftime(now, last_rotate_time) >= log_config.rotate_interval) {
            need_rotate = 1;
        }
    }

    return need_rotate;
}

static int rotate_log_files(void) {
    if (!log_config.filename[0] || log_config.max_backup_files == 0) {
        return IDCU_ERR_SUCCESS;
    }

    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }

    char old_filename[512];
    char new_filename[512];

    for (int i = log_config.max_backup_files - 1; i > 0; i--) {
        snprintf(old_filename, sizeof(old_filename), "%s.%d", log_config.filename, i);
        snprintf(new_filename, sizeof(new_filename), "%s.%d", log_config.filename, i + 1);
        rename(old_filename, new_filename);
    }

    if (log_config.max_backup_files > 0) {
        snprintf(new_filename, sizeof(new_filename), "%s.1", log_config.filename);
        rename(log_config.filename, new_filename);
    }

    log_file = fopen(log_config.filename, "a");
    if (!log_file) {
        return IDCU_ERR_GENERAL;
    }
    setbuf(log_file, NULL);
    current_file_size = 0;
    last_rotate_time = time(NULL);

    return IDCU_ERR_SUCCESS;
}

void idcu_log_get_default_config(idcu_LogConfig* config) {
    memset(config, 0, sizeof(idcu_LogConfig));
    config->level = IDCU_LOG_INFO;
    config->output = IDCU_LOG_OUTPUT_CONSOLE | IDCU_LOG_OUTPUT_FILE;
    config->rotate_policy = IDCU_LOG_ROTATE_NONE;
    config->max_file_size = 10 * 1024 * 1024;
    config->rotate_interval = 86400;
    config->max_backup_files = 5;
}

int idcu_log_init(const char* filename, idcu_LogLevel level) {
    idcu_LogConfig config;
    idcu_log_get_default_config(&config);
    config.level = level;
    if (filename) {
        strncpy(config.filename, filename, sizeof(config.filename) - 1);
    }
    return idcu_log_init_with_config(&config);
}

int idcu_log_init_with_config(const idcu_LogConfig* config) {
    memcpy(&log_config, config, sizeof(idcu_LogConfig));
    log_level = config->level;
    log_output = config->output;
    current_file_size = 0;
    last_rotate_time = time(NULL);

    if (config->filename[0] && (log_output & IDCU_LOG_OUTPUT_FILE)) {
        log_file = fopen(config->filename, "a");
        if (!log_file) {
            return IDCU_ERR_GENERAL;
        }
        setbuf(log_file, NULL);
        
        struct stat st;
        if (stat(config->filename, &st) == 0) {
            current_file_size = st.st_size;
        }
    }

    return IDCU_ERR_SUCCESS;
}

void idcu_log_shutdown(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

void idcu_log_set_level(idcu_LogLevel level) {
    if (level >= IDCU_LOG_DEBUG && level <= IDCU_LOG_FATAL) {
        log_level = level;
    }
}

idcu_LogLevel idcu_log_get_level(void) {
    return log_level;
}

int idcu_log_set_file(const char* filename) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    
    if (filename) {
        strncpy(log_config.filename, filename, sizeof(log_config.filename) - 1);
        log_file = fopen(filename, "a");
        if (!log_file) {
            return IDCU_ERR_GENERAL;
        }
        setbuf(log_file, NULL);
        current_file_size = 0;
        last_rotate_time = time(NULL);
        
        struct stat st;
        if (stat(filename, &st) == 0) {
            current_file_size = st.st_size;
        }
    }
    
    return IDCU_ERR_SUCCESS;
}

void idcu_log_set_output(idcu_LogOutput output) {
    log_output = output;
}

int idcu_log_set_rotate_policy(idcu_LogRotatePolicy policy, uint64_t max_size, uint32_t interval, uint32_t max_backups) {
    log_config.rotate_policy = policy;
    log_config.max_file_size = max_size;
    log_config.rotate_interval = interval;
    log_config.max_backup_files = max_backups;
    return IDCU_ERR_SUCCESS;
}

int idcu_log_rotate(void) {
    return rotate_log_files();
}

void idcu_log_printf(idcu_LogLevel level, const char* file, int line, const char* fmt, ...) {
    if (level < log_level) return;
    
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm);

    const char* level_name = log_level_names[level];

    char header[1024];
    int header_len = snprintf(header, sizeof(header), "[%s] [%s] [%s:%d] ",
                              time_buf, level_name, file, line);

    char content[1024];
    va_list args;
    va_start(args, fmt);
    int content_len = vsnprintf(content, sizeof(content), fmt, args);
    va_end(args);

    char full_message[2048];
    int full_len = snprintf(full_message, sizeof(full_message), "%s%s\n", header, content);

    if (log_output & IDCU_LOG_OUTPUT_CONSOLE) {
        printf("%s", full_message);
    }

    if (log_output & IDCU_LOG_OUTPUT_FILE && log_file) {
        if (check_rotate_needed()) {
            rotate_log_files();
        }
        fprintf(log_file, "%s", full_message);
        current_file_size += full_len;
    }

    if (level == IDCU_LOG_FATAL) exit(1);
}
