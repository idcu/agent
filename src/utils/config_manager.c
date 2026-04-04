#include "utils/config_manager.h"
#include "utils/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>
#endif

static idcu_ConfigManager g_config_mgr;
static int g_initialized = 0;

static idcu_ConfigSection* find_section(const char* name)
{
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

static idcu_ConfigEntry* find_entry(idcu_ConfigSection* section, const char* key)
{
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

static idcu_ConfigSection* add_section(const char* name)
{
    if (!name || g_config_mgr.section_count >= IDCU_MAX_CONFIG_SECTIONS) {
        return NULL;
    }
    idcu_ConfigSection* section = &g_config_mgr.sections[g_config_mgr.section_count];
    strncpy(section->name, name, IDCU_CONFIG_SECTION_MAX - 1);
    section->name[IDCU_CONFIG_SECTION_MAX - 1] = '\0';
    section->entry_count = 0;
    g_config_mgr.section_count++;
    return section;
}

static int add_entry(idcu_ConfigSection* section, const char* key, const char* value)
{
    if (!section || !key || section->entry_count >= IDCU_MAX_CONFIG_KEYS_PER_SECTION) {
        return IDCU_ERR_QUEUE_FULL;
    }
    idcu_ConfigEntry* entry = &section->entries[section->entry_count];
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

static char* trim_whitespace(char* str)
{
    if (!str) {
        return NULL;
    }
    char* end;
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

static const char* get_env_var(const char* name)
{
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

static int expand_env_vars(char* value, size_t max_len)
{
    if (!value || !g_config_mgr.env_var_enabled) {
        return 0;
    }
    char temp[IDCU_CONFIG_VALUE_MAX];
    char* src = value;
    char* dst = temp;
    size_t dst_remaining = sizeof(temp) - 1;
    
    while (*src && dst_remaining > 0) {
        if (*src == '$' && *(src + 1) == '{') {
            char* end_brace = strchr(src + 2, '}');
            if (end_brace) {
                char var_name[IDCU_CONFIG_KEY_MAX];
                size_t var_len = end_brace - (src + 2);
                if (var_len < sizeof(var_name)) {
                    strncpy(var_name, src + 2, var_len);
                    var_name[var_len] = '\0';
                    const char* env_val = get_env_var(var_name);
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

static int parse_file(const char* file_path)
{
    FILE* fp = fopen(file_path, "r");
    if (!fp) {
        IDCU_LOG_ERROR("Failed to open config file: %s", file_path);
        return IDCU_ERR_CONFIG_LOAD;
    }
    char line[512];
    idcu_ConfigSection* current_section = NULL;
    while (fgets(line, sizeof(line), fp)) {
        char* trimmed = trim_whitespace(line);
        if (*trimmed == '\0' || *trimmed == '#' || *trimmed == ';') {
            continue;
        }
        if (*trimmed == '[') {
            char* end_bracket = strchr(trimmed, ']');
            if (end_bracket) {
                *end_bracket = '\0';
                char* section_name = trim_whitespace(trimmed + 1);
                current_section = find_section(section_name);
                if (!current_section) {
                    current_section = add_section(section_name);
                }
            }
            continue;
        }
        char* equals = strchr(trimmed, '=');
        if (equals && current_section) {
            *equals = '\0';
            char* key = trim_whitespace(trimmed);
            char* value = trim_whitespace(equals + 1);
            expand_env_vars(value, IDCU_CONFIG_VALUE_MAX);
            idcu_ConfigEntry* existing = find_entry(current_section, key);
            if (existing) {
                strncpy(existing->value, value, IDCU_CONFIG_VALUE_MAX - 1);
                existing->value[IDCU_CONFIG_VALUE_MAX - 1] = '\0';
            } else {
                add_entry(current_section, key, value);
            }
        }
    }
    fclose(fp);
    IDCU_LOG_INFO("Config file loaded successfully: %s", file_path);
    return IDCU_ERR_SUCCESS;
}

int idcu_config_init(const char* file_path)
{
    if (g_initialized) {
        idcu_config_shutdown();
    }
    memset(&g_config_mgr, 0, sizeof(idcu_ConfigManager));
    int ret = idcu_mutex_init(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    g_config_mgr.env_var_enabled = 1;
    g_config_mgr.validation_enabled = 1;
    g_initialized = 1;
    if (file_path) {
        ret = parse_file(file_path);
        if (ret == IDCU_ERR_SUCCESS) {
            g_config_mgr.loaded = 1;
        }
    }
    return IDCU_ERR_SUCCESS;
}

void idcu_config_shutdown(void)
{
    if (!g_initialized) {
        return;
    }
    idcu_mutex_destroy(&g_config_mgr.lock);
    memset(&g_config_mgr, 0, sizeof(idcu_ConfigManager));
    g_initialized = 0;
}

int idcu_config_is_loaded(void)
{
    return g_initialized && g_config_mgr.loaded;
}

int idcu_config_reload(void)
{
    return IDCU_ERR_SUCCESS;
}

int idcu_config_save(const char* file_path)
{
    if (!g_initialized || !file_path) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    FILE* fp = fopen(file_path, "w");
    if (!fp) {
        idcu_mutex_unlock(&g_config_mgr.lock);
        return IDCU_ERR_CONFIG_LOAD;
    }
    for (uint32_t i = 0; i < g_config_mgr.section_count; i++) {
        idcu_ConfigSection* section = &g_config_mgr.sections[i];
        fprintf(fp, "[%s]\n", section->name);
        for (uint32_t j = 0; j < section->entry_count; j++) {
            idcu_ConfigEntry* entry = &section->entries[j];
            fprintf(fp, "%s = %s\n", entry->key, entry->value);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    idcu_mutex_unlock(&g_config_mgr.lock);
    return IDCU_ERR_SUCCESS;
}

const char* idcu_config_get_string(const char* section, const char* key, const char* default_value)
{
    if (!g_initialized || !section || !key) {
        return default_value;
    }
    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return default_value;
    }
    idcu_ConfigSection* sec = find_section(section);
    idcu_ConfigEntry* entry = sec ? find_entry(sec, key) : NULL;
    const char* result = entry ? entry->value : default_value;
    idcu_mutex_unlock(&g_config_mgr.lock);
    return result;
}

int idcu_config_get_int(const char* section, const char* key, int default_value)
{
    const char* str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return (int)strtol(str, NULL, 10);
}

int64_t idcu_config_get_int64(const char* section, const char* key, int64_t default_value)
{
    const char* str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return strtoll(str, NULL, 10);
}

double idcu_config_get_double(const char* section, const char* key, double default_value)
{
    const char* str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return strtod(str, NULL);
}

int idcu_config_get_bool(const char* section, const char* key, int default_value)
{
    const char* str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    if (strcasecmp(str, "true") == 0 || strcasecmp(str, "yes") == 0 || 
        strcasecmp(str, "on") == 0 || strcmp(str, "1") == 0) {
        return 1;
    }
    if (strcasecmp(str, "false") == 0 || strcasecmp(str, "no") == 0 || 
        strcasecmp(str, "off") == 0 || strcmp(str, "0") == 0) {
        return 0;
    }
    return default_value;
}

int idcu_config_set_string(const char* section, const char* key, const char* value)
{
    if (!g_initialized || !section || !key) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    idcu_ConfigSection* sec = find_section(section);
    if (!sec) {
        sec = add_section(section);
        if (!sec) {
            idcu_mutex_unlock(&g_config_mgr.lock);
            return IDCU_ERR_QUEUE_FULL;
        }
    }
    idcu_ConfigEntry* entry = find_entry(sec, key);
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

int idcu_config_set_int(const char* section, const char* key, int value)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    return idcu_config_set_string(section, key, buf);
}

int idcu_config_set_int64(const char* section, const char* key, int64_t value)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%lld", (long long)value);
    return idcu_config_set_string(section, key, buf);
}

int idcu_config_set_double(const char* section, const char* key, double value)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%f", value);
    return idcu_config_set_string(section, key, buf);
}

int idcu_config_set_bool(const char* section, const char* key, int value)
{
    return idcu_config_set_string(section, key, value ? "true" : "false");
}

int idcu_config_has_section(const char* section)
{
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

int idcu_config_has_key(const char* section, const char* key)
{
    if (!g_initialized || !section || !key) {
        return 0;
    }
    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }
    idcu_ConfigSection* sec = find_section(section);
    int has = sec && find_entry(sec, key) != NULL;
    idcu_mutex_unlock(&g_config_mgr.lock);
    return has;
}

int idcu_config_remove_key(const char* section, const char* key)
{
    if (!g_initialized || !section || !key) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&g_config_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    idcu_ConfigSection* sec = find_section(section);
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

int idcu_config_remove_section(const char* section)
{
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

int idcu_config_get_list(const char* section, const char* key, const char* delimiter, idcu_ConfigList* out_list)
{
    if (!g_initialized || !section || !key || !delimiter || !out_list) {
        return IDCU_ERR_INVALID_PARAM;
    }
    const char* value = idcu_config_get_string(section, key, NULL);
    if (!value) {
        out_list->count = 0;
        return IDCU_ERR_SUCCESS;
    }
    char temp[IDCU_CONFIG_VALUE_MAX];
    strncpy(temp, value, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    out_list->count = 0;
    char* token = strtok(temp, delimiter);
    while (token && out_list->count < IDCU_MAX_LIST_ITEMS) {
        char* trimmed = trim_whitespace(token);
        if (*trimmed) {
            strncpy(out_list->items[out_list->count], trimmed, IDCU_LIST_ITEM_MAX - 1);
            out_list->items[out_list->count][IDCU_LIST_ITEM_MAX - 1] = '\0';
            out_list->count++;
        }
        token = strtok(NULL, delimiter);
    }
    return IDCU_ERR_SUCCESS;
}

int idcu_config_list_contains(const char* section, const char* key, const char* delimiter, const char* value)
{
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

int idcu_config_get_nested_bool(const char* section, const char* prefix, const char* subkey, int default_value)
{
    if (!section || !prefix || !subkey) {
        return default_value;
    }
    char full_key[IDCU_CONFIG_KEY_MAX];
    snprintf(full_key, sizeof(full_key), "%s.%s", prefix, subkey);
    return idcu_config_get_bool(section, full_key, default_value);
}

int idcu_config_get_nested_int(const char* section, const char* prefix, const char* subkey, int default_value)
{
    if (!section || !prefix || !subkey) {
        return default_value;
    }
    char full_key[IDCU_CONFIG_KEY_MAX];
    snprintf(full_key, sizeof(full_key), "%s.%s", prefix, subkey);
    return idcu_config_get_int(section, full_key, default_value);
}

const char* idcu_config_get_nested_string(const char* section, const char* prefix, const char* subkey, const char* default_value)
{
    if (!section || !prefix || !subkey) {
        return default_value;
    }
    char full_key[IDCU_CONFIG_KEY_MAX];
    snprintf(full_key, sizeof(full_key), "%s.%s", prefix, subkey);
    return idcu_config_get_string(section, full_key, default_value);
}

void idcu_config_enable_env_var(int enable)
{
    if (g_initialized) {
        g_config_mgr.env_var_enabled = enable ? 1 : 0;
    }
}

void idcu_config_enable_validation(int enable)
{
    if (g_initialized) {
        g_config_mgr.validation_enabled = enable ? 1 : 0;
    }
}

int idcu_config_validate(void)
{
    if (!g_initialized || !g_config_mgr.validation_enabled) {
        return IDCU_ERR_SUCCESS;
    }
    IDCU_LOG_INFO("Validating configuration...");
    int valid = 1;
    int log_level_val = idcu_config_get_int("general", "log_level", -1);
    if (log_level_val == -1) {
        const char* log_level_str = idcu_config_get_string("general", "log_level", "info");
        if (strcmp(log_level_str, "debug") != 0 && 
            strcmp(log_level_str, "info") != 0 && 
            strcmp(log_level_str, "warn") != 0 && 
            strcmp(log_level_str, "error") != 0) {
            IDCU_LOG_WARN("Invalid log_level: %s, using 'info'", log_level_str);
            valid = 0;
        }
    }
    if (valid) {
        IDCU_LOG_INFO("Configuration validation passed");
    } else {
        IDCU_LOG_WARN("Configuration validation completed with warnings");
    }
    return IDCU_ERR_SUCCESS;
}

int idcu_config_load_profile(const char* profile_name)
{
    if (!g_initialized || !profile_name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    char profile_section[IDCU_CONFIG_SECTION_MAX];
    snprintf(profile_section, sizeof(profile_section), "profile.%s", profile_name);
    idcu_ConfigSection* profile_sec = find_section(profile_section);
    if (!profile_sec) {
        IDCU_LOG_WARN("Profile not found: %s", profile_name);
        return IDCU_ERR_NOT_FOUND;
    }
    IDCU_LOG_INFO("Loading profile: %s", profile_name);
    for (uint32_t i = 0; i < profile_sec->entry_count; i++) {
        idcu_ConfigEntry* entry = &profile_sec->entries[i];
        char* dot_pos = strchr(entry->key, '.');
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
    IDCU_LOG_INFO("Profile loaded successfully: %s", profile_name);
    return IDCU_ERR_SUCCESS;
}
