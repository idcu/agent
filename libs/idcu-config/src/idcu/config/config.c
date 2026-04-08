#include "idcu/config/config.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
typedef unsigned __stdcall ThreadFuncRet;
#define THREAD_FUNC_PREFIX unsigned __stdcall
#else
#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
typedef void *ThreadFuncRet;
#define THREAD_FUNC_PREFIX void *
#endif

#define IDCU_CONFIG_MAX_CALLBACKS 32

typedef struct {
    idcu_ConfigChangeCallback callback;
    void *user_data;
} idcu_ConfigCallbackEntry;

static idcu_ConfigManager g_config_mgr;
static idcu_ConfigCallbackEntry g_callbacks[IDCU_CONFIG_MAX_CALLBACKS];
static int g_callback_count = 0;
static char g_config_file_path[IDCU_CONFIG_PATH_MAX];
static int g_initialized = 0;

static volatile int g_watch_running = 0;
static volatile int g_watch_stop_requested = 0;
static uint64_t g_last_modified_time = 0;
static uint32_t g_watch_interval_ms = 5000;

#ifdef _WIN32
static HANDLE g_watch_thread = NULL;
#else
static pthread_t g_watch_thread;
#endif

static idcu_ConfigSection *find_section(const char *name) {
    if (!name) {
        return NULL;
    }
    for (uint32_t i = 0; i < g_config_mgr.section_count; i++) {
        if (strcmp(g_config_mgr.sections[i].name, name) == 0) {
            return &g_config_mgr.sections[i];
        }
    }
    return NULL;
}

static idcu_ConfigEntry *find_entry(idcu_ConfigSection *section, const char *key) {
    if (!section || !key) {
        return NULL;
    }
    for (uint32_t i = 0; i < section->entry_count; i++) {
        if (strcmp(section->entries[i].key, key) == 0) {
            return &section->entries[i];
        }
    }
    return NULL;
}

static idcu_ConfigSection *add_section(const char *name) {
    if (!name || g_config_mgr.section_count >= IDCU_CONFIG_MAX_SECTIONS) {
        return NULL;
    }
    idcu_ConfigSection *section = &g_config_mgr.sections[g_config_mgr.section_count];
    strncpy(section->name, name, IDCU_CONFIG_SECTION_MAX - 1);
    section->name[IDCU_CONFIG_SECTION_MAX - 1] = '\0';
    section->entry_count = 0;
    g_config_mgr.section_count++;
    return section;
}

static int add_entry(idcu_ConfigSection *section, const char *key, const char *value) {
    if (!section || !key || section->entry_count >= IDCU_CONFIG_MAX_KEYS_PER_SECTION) {
        return IDCU_ERR_QUEUE_FULL;
    }
    idcu_ConfigEntry *entry = &section->entries[section->entry_count];
    strncpy(entry->key, key, IDCU_CONFIG_KEY_MAX - 1);
    entry->key[IDCU_CONFIG_KEY_MAX - 1] = '\0';
    if (value) {
        strncpy(entry->value, value, IDCU_CONFIG_VALUE_MAX - 1);
        entry->value[IDCU_CONFIG_VALUE_MAX - 1] = '\0';
    } else {
        entry->value[0] = '\0';
    }
    section->entry_count++;
    return IDCU_ERR_SUCCESS;
}

static char *trim_whitespace(char *str) {
    if (!str) {
        return NULL;
    }
    char *end;
    while (isspace((unsigned char)*str)) {
        str++;
    }
    if (*str == 0) {
        return str;
    }
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    end[1] = '\0';
    return str;
}

static const char *get_env_var(const char *name) {
    if (!name || !g_config_mgr.env_var_enabled) {
        return NULL;
    }
#ifdef _WIN32
    static char env_buf[IDCU_CONFIG_VALUE_MAX];
    DWORD ret = GetEnvironmentVariableA(name, env_buf, sizeof(env_buf));
    if (ret > 0 && ret < sizeof(env_buf)) {
        return env_buf;
    }
    return NULL;
#else
    return getenv(name);
#endif
}

static int expand_env_vars(char *value, size_t max_len) {
    if (!value || !g_config_mgr.env_var_enabled) {
        return 0;
    }
    char temp[IDCU_CONFIG_VALUE_MAX];
    char *src = value;
    char *dst = temp;
    size_t dst_remaining = sizeof(temp) - 1;

    while (*src && dst_remaining > 0) {
        if (*src == '$' && *(src + 1) == '{') {
            char *end_brace = strchr(src + 2, '}');
            if (end_brace) {
                char var_name[IDCU_CONFIG_KEY_MAX];
                size_t var_len = end_brace - (src + 2);
                if (var_len < sizeof(var_name)) {
                    strncpy(var_name, src + 2, var_len);
                    var_name[var_len] = '\0';
                    const char *env_val = get_env_var(var_name);
                    if (env_val) {
                        size_t env_len = strlen(env_val);
                        if (env_len < dst_remaining) {
                            strncpy(dst, env_val, env_len);
                            dst += env_len;
                            dst_remaining -= env_len;
                        }
                    }
                    src = end_brace + 1;
                    continue;
                }
            }
        }
        *dst++ = *src++;
        dst_remaining--;
    }
    *dst = '\0';
    strncpy(value, temp, max_len - 1);
    value[max_len - 1] = '\0';
    return IDCU_ERR_SUCCESS;
}

static int parse_file(const char *file_path) {
    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        return IDCU_ERR_CONFIG_LOAD;
    }
    char line[512];
    idcu_ConfigSection *current_section = NULL;
    while (fgets(line, sizeof(line), fp)) {
        char *trimmed = trim_whitespace(line);
        if (*trimmed == '\0' || *trimmed == '#' || *trimmed == ';') {
            continue;
        }
        if (*trimmed == '[') {
            char *end_bracket = strchr(trimmed, ']');
            if (end_bracket) {
                *end_bracket = '\0';
                char *section_name = trim_whitespace(trimmed + 1);
                current_section = find_section(section_name);
                if (!current_section) {
                    current_section = add_section(section_name);
                }
            }
            continue;
        }
        char *equals = strchr(trimmed, '=');
        if (equals && current_section) {
            *equals = '\0';
            char *key = trim_whitespace(trimmed);
            char *value = trim_whitespace(equals + 1);
            expand_env_vars(value, IDCU_CONFIG_VALUE_MAX);
            idcu_ConfigEntry *existing = find_entry(current_section, key);
            if (existing) {
                strncpy(existing->value, value, IDCU_CONFIG_VALUE_MAX - 1);
                existing->value[IDCU_CONFIG_VALUE_MAX - 1] = '\0';
            } else {
                add_entry(current_section, key, value);
            }
        }
    }
    fclose(fp);
    return IDCU_ERR_SUCCESS;
}

int idcu_config_init(const char *file_path) {
    if (g_initialized) {
        idcu_config_shutdown();
    }
    memset(&g_config_mgr, 0, sizeof(idcu_ConfigManager));
    memset(g_callbacks, 0, sizeof(g_callbacks));
    g_callback_count = 0;
    g_config_file_path[0] = '\0';
    g_watch_running = 0;
    g_watch_stop_requested = 0;
    g_last_modified_time = 0;
    g_watch_interval_ms = 5000;
    int ret = idcu_mutex_init(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    g_config_mgr.env_var_enabled = 1;
    g_config_mgr.validation_enabled = 1;
    g_initialized = 1;
    if (file_path) {
        strncpy(g_config_file_path, file_path, IDCU_CONFIG_PATH_MAX - 1);
        g_config_file_path[IDCU_CONFIG_PATH_MAX - 1] = '\0';
        ret = parse_file(file_path);
        if (ret == IDCU_ERR_SUCCESS) {
            g_config_mgr.loaded = 1;
        }
    }
    return IDCU_ERR_SUCCESS;
}

void idcu_config_shutdown(void) {
    if (!g_initialized) {
        return;
    }
    idcu_config_watch_stop();
    idcu_mutex_destroy(&g_config_mgr.lock);
    memset(&g_config_mgr, 0, sizeof(idcu_ConfigManager));
    g_initialized = 0;
}

int idcu_config_is_loaded(void) { return g_initialized && g_config_mgr.loaded; }

typedef struct {
    char section[IDCU_CONFIG_SECTION_MAX];
    char key[IDCU_CONFIG_KEY_MAX];
    char old_value[IDCU_CONFIG_VALUE_MAX];
    char new_value[IDCU_CONFIG_VALUE_MAX];
} idcu_ConfigChange;

#define IDCU_CONFIG_MAX_CHANGES 256

static idcu_ConfigChange g_changes[IDCU_CONFIG_MAX_CHANGES];
static int g_change_count = 0;

static void record_change(const char *section, const char *key, const char *old_value,
                          const char *new_value) {
    if (g_change_count >= IDCU_CONFIG_MAX_CHANGES)
        return;

    idcu_ConfigChange *change = &g_changes[g_change_count];
    strncpy(change->section, section, IDCU_CONFIG_SECTION_MAX - 1);
    change->section[IDCU_CONFIG_SECTION_MAX - 1] = '\0';
    strncpy(change->key, key, IDCU_CONFIG_KEY_MAX - 1);
    change->key[IDCU_CONFIG_KEY_MAX - 1] = '\0';
    if (old_value) {
        strncpy(change->old_value, old_value, IDCU_CONFIG_VALUE_MAX - 1);
        change->old_value[IDCU_CONFIG_VALUE_MAX - 1] = '\0';
    } else {
        change->old_value[0] = '\0';
    }
    if (new_value) {
        strncpy(change->new_value, new_value, IDCU_CONFIG_VALUE_MAX - 1);
        change->new_value[IDCU_CONFIG_VALUE_MAX - 1] = '\0';
    } else {
        change->new_value[0] = '\0';
    }
    g_change_count++;
}

int idcu_config_reload(void) {
    if (!g_initialized || !g_config_file_path[0]) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    idcu_ConfigManager old_config = g_config_mgr;
    g_change_count = 0;
    memset(g_changes, 0, sizeof(g_changes));

    g_config_mgr.section_count = 0;

    ret = parse_file(g_config_file_path);
    if (ret != IDCU_ERR_SUCCESS) {
        memcpy(&g_config_mgr, &old_config, sizeof(idcu_ConfigManager));
        idcu_mutex_unlock(&g_config_mgr.lock);
        return ret;
    }

    for (uint32_t i = 0; i < old_config.section_count; i++) {
        idcu_ConfigSection *old_sec = &old_config.sections[i];
        idcu_ConfigSection *new_sec = find_section(old_sec->name);

        if (!new_sec) {
            for (uint32_t j = 0; j < old_sec->entry_count; j++) {
                record_change(old_sec->name, old_sec->entries[j].key, old_sec->entries[j].value,
                              NULL);
            }
            continue;
        }

        for (uint32_t j = 0; j < old_sec->entry_count; j++) {
            idcu_ConfigEntry *old_entry = &old_sec->entries[j];
            idcu_ConfigEntry *new_entry = find_entry(new_sec, old_entry->key);

            if (!new_entry) {
                record_change(old_sec->name, old_entry->key, old_entry->value, NULL);
            } else if (strcmp(old_entry->value, new_entry->value) != 0) {
                record_change(old_sec->name, old_entry->key, old_entry->value, new_entry->value);
            }
        }

        for (uint32_t j = 0; j < new_sec->entry_count; j++) {
            idcu_ConfigEntry *new_entry = &new_sec->entries[j];
            int found = 0;
            for (uint32_t k = 0; k < old_sec->entry_count; k++) {
                if (strcmp(old_sec->entries[k].key, new_entry->key) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                record_change(new_sec->name, new_entry->key, NULL, new_entry->value);
            }
        }
    }

    for (uint32_t i = 0; i < g_config_mgr.section_count; i++) {
        idcu_ConfigSection *new_sec = &g_config_mgr.sections[i];
        int found = 0;
        for (uint32_t j = 0; j < old_config.section_count; j++) {
            if (strcmp(old_config.sections[j].name, new_sec->name) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            for (uint32_t j = 0; j < new_sec->entry_count; j++) {
                record_change(new_sec->name, new_sec->entries[j].key, NULL,
                              new_sec->entries[j].value);
            }
        }
    }

    idcu_mutex_unlock(&g_config_mgr.lock);

    if (g_change_count > 0) {
        idcu_config_notify_changes();
    }

    return IDCU_ERR_SUCCESS;
}

int idcu_config_save(const char *file_path) {
    if (!g_initialized || !file_path) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    FILE *fp = fopen(file_path, "w");
    if (!fp) {
        idcu_mutex_unlock(&g_config_mgr.lock);
        return IDCU_ERR_CONFIG_LOAD;
    }
    for (uint32_t i = 0; i < g_config_mgr.section_count; i++) {
        idcu_ConfigSection *section = &g_config_mgr.sections[i];
        fprintf(fp, "[%s]\n", section->name);
        for (uint32_t j = 0; j < section->entry_count; j++) {
            idcu_ConfigEntry *entry = &section->entries[j];
            fprintf(fp, "%s = %s\n", entry->key, entry->value);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    idcu_mutex_unlock(&g_config_mgr.lock);
    return IDCU_ERR_SUCCESS;
}

const char *idcu_config_get_string(const char *section, const char *key,
                                   const char *default_value) {
    if (!g_initialized || !section || !key) {
        return default_value;
    }
    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return default_value;
    }
    idcu_ConfigSection *sec = find_section(section);
    idcu_ConfigEntry *entry = sec ? find_entry(sec, key) : NULL;
    const char *result = entry ? entry->value : default_value;
    idcu_mutex_unlock(&g_config_mgr.lock);
    return result;
}

int idcu_config_get_int(const char *section, const char *key, int default_value) {
    const char *str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return (int)strtol(str, NULL, 10);
}

int64_t idcu_config_get_int64(const char *section, const char *key, int64_t default_value) {
    const char *str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return strtoll(str, NULL, 10);
}

double idcu_config_get_double(const char *section, const char *key, double default_value) {
    const char *str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return strtod(str, NULL);
}

int idcu_config_get_bool(const char *section, const char *key, int default_value) {
    const char *str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    if (strcasecmp(str, "true") == 0 || strcasecmp(str, "yes") == 0 || strcasecmp(str, "on") == 0 ||
        strcmp(str, "1") == 0) {
        return 1;
    }
    if (strcasecmp(str, "false") == 0 || strcasecmp(str, "no") == 0 ||
        strcasecmp(str, "off") == 0 || strcmp(str, "0") == 0) {
        return 0;
    }
    return default_value;
}

int idcu_config_set_string(const char *section, const char *key, const char *value) {
    if (!g_initialized || !section || !key) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    idcu_ConfigSection *sec = find_section(section);
    if (!sec) {
        sec = add_section(section);
        if (!sec) {
            idcu_mutex_unlock(&g_config_mgr.lock);
            return IDCU_ERR_QUEUE_FULL;
        }
    }
    idcu_ConfigEntry *entry = find_entry(sec, key);
    if (entry) {
        if (value) {
            strncpy(entry->value, value, IDCU_CONFIG_VALUE_MAX - 1);
            entry->value[IDCU_CONFIG_VALUE_MAX - 1] = '\0';
        } else {
            entry->value[0] = '\0';
        }
    } else {
        ret = add_entry(sec, key, value);
    }
    idcu_mutex_unlock(&g_config_mgr.lock);
    return ret;
}

int idcu_config_set_int(const char *section, const char *key, int value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    return idcu_config_set_string(section, key, buf);
}

int idcu_config_set_int64(const char *section, const char *key, int64_t value) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%lld", (long long)value);
    return idcu_config_set_string(section, key, buf);
}

int idcu_config_set_double(const char *section, const char *key, double value) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%f", value);
    return idcu_config_set_string(section, key, buf);
}

int idcu_config_set_bool(const char *section, const char *key, int value) {
    return idcu_config_set_string(section, key, value ? "true" : "false");
}

int idcu_config_has_section(const char *section) {
    if (!g_initialized || !section) {
        return 0;
    }
    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }
    int has = find_section(section) != NULL;
    idcu_mutex_unlock(&g_config_mgr.lock);
    return has;
}

int idcu_config_has_key(const char *section, const char *key) {
    if (!g_initialized || !section || !key) {
        return 0;
    }
    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }
    idcu_ConfigSection *sec = find_section(section);
    int has = sec && find_entry(sec, key) != NULL;
    idcu_mutex_unlock(&g_config_mgr.lock);
    return has;
}

int idcu_config_remove_key(const char *section, const char *key) {
    if (!g_initialized || !section || !key) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    idcu_ConfigSection *sec = find_section(section);
    if (!sec) {
        idcu_mutex_unlock(&g_config_mgr.lock);
        return IDCU_ERR_NOT_FOUND;
    }
    for (uint32_t i = 0; i < sec->entry_count; i++) {
        if (strcmp(sec->entries[i].key, key) == 0) {
            if (i < sec->entry_count - 1) {
                memmove(&sec->entries[i], &sec->entries[i + 1],
                        (sec->entry_count - i - 1) * sizeof(idcu_ConfigEntry));
            }
            sec->entry_count--;
            idcu_mutex_unlock(&g_config_mgr.lock);
            return IDCU_ERR_SUCCESS;
        }
    }
    idcu_mutex_unlock(&g_config_mgr.lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_config_remove_section(const char *section) {
    if (!g_initialized || !section) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    for (uint32_t i = 0; i < g_config_mgr.section_count; i++) {
        if (strcmp(g_config_mgr.sections[i].name, section) == 0) {
            if (i < g_config_mgr.section_count - 1) {
                memmove(&g_config_mgr.sections[i], &g_config_mgr.sections[i + 1],
                        (g_config_mgr.section_count - i - 1) * sizeof(idcu_ConfigSection));
            }
            g_config_mgr.section_count--;
            idcu_mutex_unlock(&g_config_mgr.lock);
            return IDCU_ERR_SUCCESS;
        }
    }
    idcu_mutex_unlock(&g_config_mgr.lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_config_get_list(const char *section, const char *key, const char *delimiter,
                         idcu_ConfigList *out_list) {
    if (!g_initialized || !section || !key || !delimiter || !out_list) {
        return IDCU_ERR_INVALID_PARAM;
    }
    const char *value = idcu_config_get_string(section, key, NULL);
    if (!value) {
        out_list->count = 0;
        return IDCU_ERR_SUCCESS;
    }
    char temp[IDCU_CONFIG_VALUE_MAX];
    strncpy(temp, value, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    out_list->count = 0;
    char *token = strtok(temp, delimiter);
    while (token && out_list->count < IDCU_CONFIG_MAX_LIST_ITEMS) {
        char *trimmed = trim_whitespace(token);
        if (*trimmed) {
            strncpy(out_list->items[out_list->count], trimmed, IDCU_CONFIG_LIST_ITEM_MAX - 1);
            out_list->items[out_list->count][IDCU_CONFIG_LIST_ITEM_MAX - 1] = '\0';
            out_list->count++;
        }
        token = strtok(NULL, delimiter);
    }
    return IDCU_ERR_SUCCESS;
}

int idcu_config_list_contains(const char *section, const char *key, const char *delimiter,
                              const char *value) {
    if (!g_initialized || !section || !key || !delimiter || !value) {
        return 0;
    }
    idcu_ConfigList list;
    int ret = idcu_config_get_list(section, key, delimiter, &list);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }
    for (int i = 0; i < list.count; i++) {
        if (strcmp(list.items[i], value) == 0) {
            return 1;
        }
    }
    return 0;
}

int idcu_config_get_nested_bool(const char *section, const char *prefix, const char *subkey,
                                int default_value) {
    if (!section || !prefix || !subkey) {
        return default_value;
    }
    char full_key[IDCU_CONFIG_KEY_MAX];
    snprintf(full_key, sizeof(full_key), "%s.%s", prefix, subkey);
    return idcu_config_get_bool(section, full_key, default_value);
}

int idcu_config_get_nested_int(const char *section, const char *prefix, const char *subkey,
                               int default_value) {
    if (!section || !prefix || !subkey) {
        return default_value;
    }
    char full_key[IDCU_CONFIG_KEY_MAX];
    snprintf(full_key, sizeof(full_key), "%s.%s", prefix, subkey);
    return idcu_config_get_int(section, full_key, default_value);
}

const char *idcu_config_get_nested_string(const char *section, const char *prefix,
                                          const char *subkey, const char *default_value) {
    if (!section || !prefix || !subkey) {
        return default_value;
    }
    char full_key[IDCU_CONFIG_KEY_MAX];
    snprintf(full_key, sizeof(full_key), "%s.%s", prefix, subkey);
    return idcu_config_get_string(section, full_key, default_value);
}

void idcu_config_enable_env_var(int enable) {
    if (g_initialized) {
        g_config_mgr.env_var_enabled = enable ? 1 : 0;
    }
}

void idcu_config_enable_validation(int enable) {
    if (g_initialized) {
        g_config_mgr.validation_enabled = enable ? 1 : 0;
    }
}

int idcu_config_validate(void) { return IDCU_ERR_SUCCESS; }

int idcu_config_load_profile(const char *profile_name) {
    if (!g_initialized || !profile_name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    char profile_section[IDCU_CONFIG_SECTION_MAX];
    snprintf(profile_section, sizeof(profile_section), "profile.%s", profile_name);
    idcu_ConfigSection *profile_sec = find_section(profile_section);
    if (!profile_sec) {
        return IDCU_ERR_NOT_FOUND;
    }
    for (uint32_t i = 0; i < profile_sec->entry_count; i++) {
        idcu_ConfigEntry *entry = &profile_sec->entries[i];
        char *dot_pos = strchr(entry->key, '.');
        if (dot_pos) {
            char target_section[IDCU_CONFIG_SECTION_MAX];
            char target_key[IDCU_CONFIG_KEY_MAX];
            size_t section_len = dot_pos - entry->key;
            strncpy(target_section, entry->key, section_len);
            target_section[section_len] = '\0';
            strncpy(target_key, dot_pos + 1, sizeof(target_key) - 1);
            target_key[sizeof(target_key) - 1] = '\0';
            idcu_config_set_string(target_section, target_key, entry->value);
        } else {
            idcu_config_set_string("general", entry->key, entry->value);
        }
    }
    return IDCU_ERR_SUCCESS;
}

int idcu_config_register_change_callback(idcu_ConfigChangeCallback callback, void *user_data) {
    if (!g_initialized || !callback) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (int i = 0; i < g_callback_count; i++) {
        if (g_callbacks[i].callback == callback) {
            return IDCU_ERR_ALREADY_EXISTS;
        }
    }

    if (g_callback_count >= IDCU_CONFIG_MAX_CALLBACKS) {
        return IDCU_ERR_NO_MEMORY;
    }

    g_callbacks[g_callback_count].callback = callback;
    g_callbacks[g_callback_count].user_data = user_data;
    g_callback_count++;

    return IDCU_ERR_SUCCESS;
}

int idcu_config_unregister_change_callback(idcu_ConfigChangeCallback callback) {
    if (!g_initialized || !callback) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (int i = 0; i < g_callback_count; i++) {
        if (g_callbacks[i].callback == callback) {
            if (i < g_callback_count - 1) {
                memmove(&g_callbacks[i], &g_callbacks[i + 1],
                        (g_callback_count - i - 1) * sizeof(idcu_ConfigCallbackEntry));
            }
            g_callback_count--;
            return IDCU_ERR_SUCCESS;
        }
    }

    return IDCU_ERR_NOT_FOUND;
}

void idcu_config_notify_changes(void) {
    if (!g_initialized) {
        return;
    }

    for (int i = 0; i < g_change_count; i++) {
        idcu_ConfigChange *change = &g_changes[i];
        for (int j = 0; j < g_callback_count; j++) {
            g_callbacks[j].callback(
                change->section, change->key, change->old_value[0] ? change->old_value : NULL,
                change->new_value[0] ? change->new_value : NULL, g_callbacks[j].user_data);
        }
    }
}

int idcu_config_get_file_path(char *buffer, size_t buffer_size) {
    if (!g_initialized || !buffer) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (!g_config_file_path[0]) {
        return IDCU_ERR_NOT_FOUND;
    }

    strncpy(buffer, g_config_file_path, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return IDCU_ERR_SUCCESS;
}

int idcu_config_get_last_modified_time(uint64_t *timestamp) {
    if (!g_initialized || !timestamp || !g_config_file_path[0]) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA file_attr;
    if (!GetFileAttributesExA(g_config_file_path, GetFileExInfoStandard, &file_attr)) {
        return IDCU_ERR_NOT_FOUND;
    }
    ULARGE_INTEGER uli;
    uli.LowPart = file_attr.ftLastWriteTime.dwLowDateTime;
    uli.HighPart = file_attr.ftLastWriteTime.dwHighDateTime;
    *timestamp = (uint64_t)uli.QuadPart;
#else
    struct stat stat_buf;
    if (stat(g_config_file_path, &stat_buf) != 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    *timestamp = (uint64_t)stat_buf.st_mtime;
#endif

    return IDCU_ERR_SUCCESS;
}

static void sleep_ms(uint32_t ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

static THREAD_FUNC_PREFIX config_watch_thread(void *arg) {
    (void)arg;
    uint64_t current_mtime;

    while (!g_watch_stop_requested) {
        if (g_config_file_path[0] != '\0' &&
            idcu_config_get_last_modified_time(&current_mtime) == IDCU_ERR_SUCCESS) {
            if (g_last_modified_time != 0 && current_mtime != g_last_modified_time) {
                idcu_config_reload();
            }
            g_last_modified_time = current_mtime;
        }
        sleep_ms(g_watch_interval_ms);
    }

    g_watch_running = 0;
#ifdef _WIN32
    _endthreadex(0);
    return 0;
#else
    return NULL;
#endif
}

int idcu_config_watch_start(uint32_t interval_ms) {
    if (!g_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    if (g_watch_running) {
        return IDCU_ERR_ALREADY_EXISTS;
    }

    if (g_config_file_path[0] == '\0') {
        return IDCU_ERR_INVALID_PARAM;
    }

    g_watch_interval_ms = (interval_ms > 0) ? interval_ms : 5000;
    g_watch_stop_requested = 0;

    if (idcu_config_get_last_modified_time(&g_last_modified_time) != IDCU_ERR_SUCCESS) {
        g_last_modified_time = 0;
    }

#ifdef _WIN32
    g_watch_thread = (HANDLE)_beginthreadex(NULL, 0, config_watch_thread, NULL, 0, NULL);
    if (g_watch_thread == NULL) {
        return IDCU_ERR_NO_MEMORY;
    }
#else
    if (pthread_create(&g_watch_thread, NULL, config_watch_thread, NULL) != 0) {
        return IDCU_ERR_NO_MEMORY;
    }
#endif

    g_watch_running = 1;
    return IDCU_ERR_SUCCESS;
}

void idcu_config_watch_stop(void) {
    if (!g_watch_running) {
        return;
    }

    g_watch_stop_requested = 1;

#ifdef _WIN32
    if (g_watch_thread != NULL) {
        WaitForSingleObject(g_watch_thread, INFINITE);
        CloseHandle(g_watch_thread);
        g_watch_thread = NULL;
    }
#else
    pthread_join(g_watch_thread, NULL);
#endif

    g_watch_running = 0;
}

int idcu_config_watch_is_running(void) { return g_watch_running; }
