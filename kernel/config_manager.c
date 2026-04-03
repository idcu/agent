#include "config_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static ConfigManager g_config_mgr;
static int g_initialized = 0;

static ConfigSection* find_section(const char* name)
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

static ConfigEntry* find_entry(ConfigSection* section, const char* key)
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

static ConfigSection* add_section(const char* name)
{
    if (!name || g_config_mgr.section_count >= MAX_CONFIG_SECTIONS) {
        return NULL;
    }
    ConfigSection* section = &g_config_mgr.sections[g_config_mgr.section_count];
    strncpy(section->name, name, CONFIG_SECTION_MAX - 1);
    section->name[CONFIG_SECTION_MAX - 1] = '\0';
    section->entry_count = 0;
    g_config_mgr.section_count++;
    return section;
}

static int add_entry(ConfigSection* section, const char* key, const char* value)
{
    if (!section || !key || section->entry_count >= MAX_CONFIG_KEYS_PER_SECTION) {
        return ERR_QUEUE_FULL;
    }
    ConfigEntry* entry = &section->entries[section->entry_count];
    strncpy(entry->key, key, CONFIG_KEY_MAX - 1);
    entry->key[CONFIG_KEY_MAX - 1] = '\0';
    if (value) {
        strncpy(entry->value, value, CONFIG_VALUE_MAX - 1);
        entry->value[CONFIG_VALUE_MAX - 1] = '\0';
    } else {
        entry->value[0] = '\0';
    }
    section->entry_count++;
    return ERR_SUCCESS;
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

static int parse_file(const char* file_path)
{
    FILE* fp = fopen(file_path, "r");
    if (!fp) {
        return ERR_CONFIG_LOAD;
    }
    char line[512];
    ConfigSection* current_section = NULL;
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
            ConfigEntry* existing = find_entry(current_section, key);
            if (existing) {
                strncpy(existing->value, value, CONFIG_VALUE_MAX - 1);
                existing->value[CONFIG_VALUE_MAX - 1] = '\0';
            } else {
                add_entry(current_section, key, value);
            }
        }
    }
    fclose(fp);
    return ERR_SUCCESS;
}

int config_init(const char* file_path)
{
    if (g_initialized) {
        config_shutdown();
    }
    memset(&g_config_mgr, 0, sizeof(ConfigManager));
    int ret = mutex_init(&g_config_mgr.lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    g_initialized = 1;
    if (file_path) {
        ret = parse_file(file_path);
        if (ret == ERR_SUCCESS) {
            g_config_mgr.loaded = 1;
        }
    }
    return ERR_SUCCESS;
}

void config_shutdown(void)
{
    if (!g_initialized) {
        return;
    }
    mutex_destroy(&g_config_mgr.lock);
    memset(&g_config_mgr, 0, sizeof(ConfigManager));
    g_initialized = 0;
}

int config_is_loaded(void)
{
    return g_initialized && g_config_mgr.loaded;
}

int config_reload(void)
{
    return ERR_SUCCESS;
}

int config_save(const char* file_path)
{
    if (!g_initialized || !file_path) {
        return ERR_INVALID_PARAM;
    }
    int ret = mutex_lock(&g_config_mgr.lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    FILE* fp = fopen(file_path, "w");
    if (!fp) {
        mutex_unlock(&g_config_mgr.lock);
        return ERR_CONFIG_LOAD;
    }
    for (uint32_t i = 0; i < g_config_mgr.section_count; i++) {
        ConfigSection* section = &g_config_mgr.sections[i];
        fprintf(fp, "[%s]\n", section->name);
        for (uint32_t j = 0; j < section->entry_count; j++) {
            ConfigEntry* entry = &section->entries[j];
            fprintf(fp, "%s = %s\n", entry->key, entry->value);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    mutex_unlock(&g_config_mgr.lock);
    return ERR_SUCCESS;
}

const char* config_get_string(const char* section, const char* key, const char* default_value)
{
    if (!g_initialized || !section || !key) {
        return default_value;
    }
    int ret = mutex_lock(&g_config_mgr.lock);
    if (ret != ERR_SUCCESS) {
        return default_value;
    }
    ConfigSection* sec = find_section(section);
    ConfigEntry* entry = sec ? find_entry(sec, key) : NULL;
    const char* result = entry ? entry->value : default_value;
    mutex_unlock(&g_config_mgr.lock);
    return result;
}

int config_get_int(const char* section, const char* key, int default_value)
{
    const char* str = config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return (int)strtol(str, NULL, 10);
}

int64_t config_get_int64(const char* section, const char* key, int64_t default_value)
{
    const char* str = config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return strtoll(str, NULL, 10);
}

double config_get_double(const char* section, const char* key, double default_value)
{
    const char* str = config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return strtod(str, NULL);
}

int config_get_bool(const char* section, const char* key, int default_value)
{
    const char* str = config_get_string(section, key, NULL);
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

int config_set_string(const char* section, const char* key, const char* value)
{
    if (!g_initialized || !section || !key) {
        return ERR_INVALID_PARAM;
    }
    int ret = mutex_lock(&g_config_mgr.lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    ConfigSection* sec = find_section(section);
    if (!sec) {
        sec = add_section(section);
        if (!sec) {
            mutex_unlock(&g_config_mgr.lock);
            return ERR_QUEUE_FULL;
        }
    }
    ConfigEntry* entry = find_entry(sec, key);
    if (entry) {
        if (value) {
            strncpy(entry->value, value, CONFIG_VALUE_MAX - 1);
            entry->value[CONFIG_VALUE_MAX - 1] = '\0';
        } else {
            entry->value[0] = '\0';
        }
    } else {
        ret = add_entry(sec, key, value);
    }
    mutex_unlock(&g_config_mgr.lock);
    return ret;
}

int config_set_int(const char* section, const char* key, int value)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    return config_set_string(section, key, buf);
}

int config_set_int64(const char* section, const char* key, int64_t value)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%lld", (long long)value);
    return config_set_string(section, key, buf);
}

int config_set_double(const char* section, const char* key, double value)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%f", value);
    return config_set_string(section, key, buf);
}

int config_set_bool(const char* section, const char* key, int value)
{
    return config_set_string(section, key, value ? "true" : "false");
}

int config_has_section(const char* section)
{
    if (!g_initialized || !section) {
        return 0;
    }
    int ret = mutex_lock(&g_config_mgr.lock);
    if (ret != ERR_SUCCESS) {
        return 0;
    }
    int has = find_section(section) != NULL;
    mutex_unlock(&g_config_mgr.lock);
    return has;
}

int config_has_key(const char* section, const char* key)
{
    if (!g_initialized || !section || !key) {
        return 0;
    }
    int ret = mutex_lock(&g_config_mgr.lock);
    if (ret != ERR_SUCCESS) {
        return 0;
    }
    ConfigSection* sec = find_section(section);
    int has = sec && find_entry(sec, key) != NULL;
    mutex_unlock(&g_config_mgr.lock);
    return has;
}

int config_remove_key(const char* section, const char* key)
{
    if (!g_initialized || !section || !key) {
        return ERR_INVALID_PARAM;
    }
    int ret = mutex_lock(&g_config_mgr.lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    ConfigSection* sec = find_section(section);
    if (!sec) {
        mutex_unlock(&g_config_mgr.lock);
        return ERR_NOT_FOUND;
    }
    for (uint32_t i = 0; i < sec->entry_count; i++) {
        if (strcmp(sec->entries[i].key, key) == 0) {
            if (i < sec->entry_count - 1) {
                memmove(&sec->entries[i], &sec->entries[i + 1],
                        (sec->entry_count - i - 1) * sizeof(ConfigEntry));
            }
            sec->entry_count--;
            mutex_unlock(&g_config_mgr.lock);
            return ERR_SUCCESS;
        }
    }
    mutex_unlock(&g_config_mgr.lock);
    return ERR_NOT_FOUND;
}

int config_remove_section(const char* section)
{
    if (!g_initialized || !section) {
        return ERR_INVALID_PARAM;
    }
    int ret = mutex_lock(&g_config_mgr.lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    for (uint32_t i = 0; i < g_config_mgr.section_count; i++) {
        if (strcmp(g_config_mgr.sections[i].name, section) == 0) {
            if (i < g_config_mgr.section_count - 1) {
                memmove(&g_config_mgr.sections[i], &g_config_mgr.sections[i + 1],
                        (g_config_mgr.section_count - i - 1) * sizeof(ConfigSection));
            }
            g_config_mgr.section_count--;
            mutex_unlock(&g_config_mgr.lock);
            return ERR_SUCCESS;
        }
    }
    mutex_unlock(&g_config_mgr.lock);
    return ERR_NOT_FOUND;
}
