#include <idcu/config/config.h>
#include <idcu/json/json.h>
#include <idcu/yaml/yaml.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

// 全局配置管理器
static idcu_ConfigManager g_config_manager;
static int g_initialized = 0;

// ========== 内部辅助函数 ==========
static idcu_ConfigSection* find_or_create_section(const char* name) {
    for (uint32_t i = 0; i < g_config_manager.section_count; i++) {
        if (strcmp(g_config_manager.sections[i].name, name) == 0) {
            return &g_config_manager.sections[i];
        }
    }
    
    if (g_config_manager.section_count >= IDCU_CONFIG_MAX_SECTIONS) {
        return NULL;
    }
    
    idcu_ConfigSection* section = &g_config_manager.sections[g_config_manager.section_count++];
    strncpy(section->name, name, IDCU_CONFIG_SECTION_MAX - 1);
    section->name[IDCU_CONFIG_SECTION_MAX - 1] = '\0';
    section->entry_count = 0;
    return section;
}

static idcu_ConfigEntry* find_entry(idcu_ConfigSection* section, const char* key) {
    for (uint32_t i = 0; i < section->entry_count; i++) {
        if (strcmp(section->entries[i].key, key) == 0) {
            return &section->entries[i];
        }
    }
    return NULL;
}

static idcu_ConfigEntry* create_entry(idcu_ConfigSection* section, const char* key) {
    if (section->entry_count >= IDCU_CONFIG_MAX_KEYS_PER_SECTION) {
        return NULL;
    }
    
    idcu_ConfigEntry* entry = &section->entries[section->entry_count++];
    strncpy(entry->key, key, IDCU_CONFIG_KEY_MAX - 1);
    entry->key[IDCU_CONFIG_KEY_MAX - 1] = '\0';
    entry->value[0] = '\0';
    return entry;
}

static void notify_change_callbacks(const char* section, const char* key,
                                     const char* old_value, const char* new_value) {
    for (int i = 0; i < g_config_manager.callback_count; i++) {
        if (g_config_manager.callbacks[i].callback) {
            g_config_manager.callbacks[i].callback(section, key, old_value, new_value,
                                                   g_config_manager.callbacks[i].user_data);
        }
    }
}

static int parse_simple_config(const char* file_path) {
    FILE* fp = fopen(file_path, "r");
    if (!fp) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    char line[1024];
    char current_section[IDCU_CONFIG_SECTION_MAX] = "";
    
    while (fgets(line, sizeof(line), fp)) {
        // 去除首尾空白
        char* ptr = line;
        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
        
        // 空行或注释
        if (!*ptr || *ptr == '#' || *ptr == ';') {
            continue;
        }
        
        // 去除换行符
        char* newline = strchr(ptr, '\n');
        if (newline) *newline = '\0';
        newline = strchr(ptr, '\r');
        if (newline) *newline = '\0';
        
        // 节标题 [section]
        if (*ptr == '[') {
            char* end = strchr(ptr + 1, ']');
            if (end) {
                *end = '\0';
                strncpy(current_section, ptr + 1, IDCU_CONFIG_SECTION_MAX - 1);
                current_section[IDCU_CONFIG_SECTION_MAX - 1] = '\0';
            }
            continue;
        }
        
        // 键值对 key=value
        char* eq = strchr(ptr, '=');
        if (eq) {
            *eq = '\0';
            char* key = ptr;
            char* value = eq + 1;
            
            // 去除key的尾部空白
            char* key_end = key;
            while (*key_end && !isspace((unsigned char)*key_end)) key_end++;
            *key_end = '\0';
            
            // 去除value的开头空白
            while (*value && isspace((unsigned char)*value)) value++;
            
            idcu_ConfigSection* section = find_or_create_section(
                current_section[0] ? current_section : "default"
            );
            if (section) {
                idcu_ConfigEntry* entry = find_entry(section, key);
                if (!entry) {
                    entry = create_entry(section, key);
                }
                if (entry) {
                    strncpy(entry->value, value, IDCU_CONFIG_VALUE_MAX - 1);
                    entry->value[IDCU_CONFIG_VALUE_MAX - 1] = '\0';
                }
            }
        }
    }
    
    fclose(fp);
    return IDCU_ERR_OK;
}

// ========== 核心 API ==========
int idcu_config_init(const char* file_path) {
    if (!file_path) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (g_initialized) {
        idcu_config_shutdown();
    }
    
    memset(&g_config_manager, 0, sizeof(g_config_manager));
    idcu_mutex_init(&g_config_manager.lock);
    
    strncpy(g_config_manager.file_path, file_path, IDCU_CONFIG_PATH_MAX - 1);
    g_config_manager.file_path[IDCU_CONFIG_PATH_MAX - 1] = '\0';
    
    int ret = parse_simple_config(file_path);
    if (ret == IDCU_ERR_OK) {
        g_config_manager.loaded = 1;
        g_initialized = 1;
    }
    
    return ret;
}

void idcu_config_shutdown(void) {
    if (!g_initialized) {
        return;
    }
    
    idcu_mutex_lock(&g_config_manager.lock);
    memset(&g_config_manager, 0, sizeof(g_config_manager));
    idcu_mutex_destroy(&g_config_manager.lock);
    g_initialized = 0;
}

int idcu_config_is_loaded(void) {
    return g_config_manager.loaded;
}

int idcu_config_reload(void) {
    if (!g_initialized || !g_config_manager.file_path[0]) {
        return IDCU_ERR_INVALID_STATE;
    }
    
    idcu_mutex_lock(&g_config_manager.lock);
    
    // 清空并重新加载
    g_config_manager.section_count = 0;
    int ret = parse_simple_config(g_config_manager.file_path);
    
    idcu_mutex_unlock(&g_config_manager.lock);
    
    if (ret == IDCU_ERR_OK) {
        // 简单通知所有配置可能变更
        notify_change_callbacks("*", "*", NULL, NULL);
    }
    
    return ret;
}

int idcu_config_save(const char* file_path) {
    if (!g_initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    
    const char* path = file_path ? file_path : g_config_manager.file_path;
    if (!path) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    FILE* fp = fopen(path, "w");
    if (!fp) {
        return IDCU_ERR_UNKNOWN;
    }
    
    idcu_mutex_lock(&g_config_manager.lock);
    
    for (uint32_t s = 0; s < g_config_manager.section_count; s++) {
        idcu_ConfigSection* section = &g_config_manager.sections[s];
        fprintf(fp, "[%s]\n", section->name);
        
        for (uint32_t e = 0; e < section->entry_count; e++) {
            fprintf(fp, "%s=%s\n", section->entries[e].key, section->entries[e].value);
        }
        
        fprintf(fp, "\n");
    }
    
    idcu_mutex_unlock(&g_config_manager.lock);
    
    fclose(fp);
    return IDCU_ERR_OK;
}

// ========== 类型安全访问 ==========
const char* idcu_config_get_string(const char* section, const char* key, const char* default_value) {
    if (!g_initialized || !section || !key) {
        return default_value;
    }
    
    // 检查环境变量
    if (g_config_manager.env_var_enabled) {
        char env_key[256];
        snprintf(env_key, sizeof(env_key), "%s_%s", section, key);
        char* env_val = getenv(env_key);
        if (env_val) {
            return env_val;
        }
    }
    
    idcu_mutex_lock(&g_config_manager.lock);
    
    idcu_ConfigSection* sec = NULL;
    for (uint32_t i = 0; i < g_config_manager.section_count; i++) {
        if (strcmp(g_config_manager.sections[i].name, section) == 0) {
            sec = &g_config_manager.sections[i];
            break;
        }
    }
    
    const char* result = default_value;
    if (sec) {
        idcu_ConfigEntry* entry = find_entry(sec, key);
        if (entry && entry->value[0]) {
            result = entry->value;
        }
    }
    
    idcu_mutex_unlock(&g_config_manager.lock);
    return result;
}

int idcu_config_get_int(const char* section, const char* key, int default_value) {
    const char* str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return (int)strtol(str, NULL, 10);
}

int64_t idcu_config_get_int64(const char* section, const char* key, int64_t default_value) {
    const char* str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return strtoll(str, NULL, 10);
}

double idcu_config_get_double(const char* section, const char* key, double default_value) {
    const char* str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return strtod(str, NULL);
}

int idcu_config_get_bool(const char* section, const char* key, int default_value) {
    const char* str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        return default_value;
    }
    return (strcmp(str, "true") == 0 || strcmp(str, "1") == 0 || 
            strcmp(str, "yes") == 0 || strcmp(str, "on") == 0);
}

// ========== 配置写入 ==========
int idcu_config_set_string(const char* section, const char* key, const char* value) {
    if (!g_initialized || !section || !key || !value) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&g_config_manager.lock);
    
    idcu_ConfigSection* sec = find_or_create_section(section);
    if (!sec) {
        idcu_mutex_unlock(&g_config_manager.lock);
        return IDCU_ERR_OUT_OF_RANGE;
    }
    
    char old_value[IDCU_CONFIG_VALUE_MAX] = "";
    idcu_ConfigEntry* entry = find_entry(sec, key);
    if (entry) {
        strncpy(old_value, entry->value, sizeof(old_value) - 1);
    } else {
        entry = create_entry(sec, key);
        if (!entry) {
            idcu_mutex_unlock(&g_config_manager.lock);
            return IDCU_ERR_OUT_OF_RANGE;
        }
    }
    
    strncpy(entry->value, value, IDCU_CONFIG_VALUE_MAX - 1);
    entry->value[IDCU_CONFIG_VALUE_MAX - 1] = '\0';
    
    idcu_mutex_unlock(&g_config_manager.lock);
    
    notify_change_callbacks(section, key, old_value, value);
    return IDCU_ERR_OK;
}

int idcu_config_set_int(const char* section, const char* key, int value) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%d", value);
    return idcu_config_set_string(section, key, buf);
}

int idcu_config_set_int64(const char* section, const char* key, int64_t value) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%lld", (long long)value);
    return idcu_config_set_string(section, key, buf);
}

int idcu_config_set_double(const char* section, const char* key, double value) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%f", value);
    return idcu_config_set_string(section, key, buf);
}

int idcu_config_set_bool(const char* section, const char* key, int value) {
    return idcu_config_set_string(section, key, value ? "true" : "false");
}

// ========== 列表支持 ==========
int idcu_config_get_list(const char* section, const char* key, const char* delimiter, idcu_ConfigList* out_list) {
    if (!g_initialized || !section || !key || !delimiter || !out_list) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    const char* str = idcu_config_get_string(section, key, NULL);
    if (!str) {
        out_list->count = 0;
        return IDCU_ERR_OK;
    }
    
    char copy[IDCU_CONFIG_VALUE_MAX];
    strncpy(copy, str, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';
    
    out_list->count = 0;
    char* token = strtok(copy, delimiter);
    
    while (token && out_list->count < IDCU_CONFIG_LIST_MAX_ITEMS) {
        // 去除空白
        while (*token && isspace((unsigned char)*token)) token++;
        char* end = token + strlen(token) - 1;
        while (end > token && isspace((unsigned char)*end)) *end-- = '\0';
        
        if (*token) {
            strncpy(out_list->items[out_list->count], token, IDCU_CONFIG_LIST_ITEM_MAX - 1);
            out_list->items[out_list->count][IDCU_CONFIG_LIST_ITEM_MAX - 1] = '\0';
            out_list->count++;
        }
        
        token = strtok(NULL, delimiter);
    }
    
    return IDCU_ERR_OK;
}

int idcu_config_set_list(const char* section, const char* key, const char* delimiter, const idcu_ConfigList* list) {
    if (!g_initialized || !section || !key || !delimiter || !list) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    char value[IDCU_CONFIG_VALUE_MAX] = "";
    size_t pos = 0;
    
    for (int i = 0; i < list->count && pos < sizeof(value) - 1; i++) {
        if (i > 0) {
            strncat(value, delimiter, sizeof(value) - pos - 1);
            pos += strlen(delimiter);
        }
        strncat(value, list->items[i], sizeof(value) - pos - 1);
        pos += strlen(list->items[i]);
    }
    
    return idcu_config_set_string(section, key, value);
}

// ========== 变更通知 ==========
int idcu_config_register_change_callback(idcu_ConfigChangeCallback callback, void* user_data) {
    if (!g_initialized || !callback) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&g_config_manager.lock);
    
    if (g_config_manager.callback_count >= IDCU_CONFIG_MAX_CALLBACKS) {
        idcu_mutex_unlock(&g_config_manager.lock);
        return IDCU_ERR_OUT_OF_RANGE;
    }
    
    // 检查是否已注册
    for (int i = 0; i < g_config_manager.callback_count; i++) {
        if (g_config_manager.callbacks[i].callback == callback) {
            idcu_mutex_unlock(&g_config_manager.lock);
            return IDCU_ERR_OK;
        }
    }
    
    g_config_manager.callbacks[g_config_manager.callback_count].callback = callback;
    g_config_manager.callbacks[g_config_manager.callback_count].user_data = user_data;
    g_config_manager.callback_count++;
    
    idcu_mutex_unlock(&g_config_manager.lock);
    return IDCU_ERR_OK;
}

int idcu_config_unregister_change_callback(idcu_ConfigChangeCallback callback) {
    if (!g_initialized || !callback) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&g_config_manager.lock);
    
    for (int i = 0; i < g_config_manager.callback_count; i++) {
        if (g_config_manager.callbacks[i].callback == callback) {
            // 将后面的回调前移
            for (int j = i; j < g_config_manager.callback_count - 1; j++) {
                g_config_manager.callbacks[j] = g_config_manager.callbacks[j + 1];
            }
            g_config_manager.callback_count--;
            idcu_mutex_unlock(&g_config_manager.lock);
            return IDCU_ERR_OK;
        }
    }
    
    idcu_mutex_unlock(&g_config_manager.lock);
    return IDCU_ERR_NOT_FOUND;
}

// ========== 文件监控 (简化版 - 暂未实现) ==========
static int g_watch_running = 0;

int idcu_config_watch_start(uint32_t interval_ms) {
    (void)interval_ms;
    if (!g_initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    g_watch_running = 1;
    return IDCU_ERR_OK;
}

void idcu_config_watch_stop(void) {
    g_watch_running = 0;
}

int idcu_config_watch_is_running(void) {
    return g_watch_running;
}

// ========== 环境变量支持 ==========
void idcu_config_enable_env_var(int enable) {
    if (g_initialized) {
        g_config_manager.env_var_enabled = enable;
    }
}

static char g_env_prefix[64] = "IDCU_";

int idcu_config_load_profile(const char* profile_name) {
    if (!g_initialized || !profile_name) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    strncpy(g_config_manager.current_profile, profile_name, sizeof(g_config_manager.current_profile) - 1);
    g_config_manager.current_profile[sizeof(g_config_manager.current_profile) - 1] = '\0';
    
    return IDCU_ERR_OK;
}

// ========== JSON/YAML support (Stubbed for now) ==========
int idcu_config_init_from_json(const char* file_path) {
    (void)file_path;
    return IDCU_ERR_UNKNOWN;
}

int idcu_config_init_from_yaml(const char* file_path) {
    (void)file_path;
    return IDCU_ERR_UNKNOWN;
}

int idcu_config_save_to_json(const char* file_path) {
    (void)file_path;
    return IDCU_ERR_UNKNOWN;
}

int idcu_config_save_to_yaml(const char* file_path) {
    (void)file_path;
    return IDCU_ERR_UNKNOWN;
}

// ========== Environment variable substitution ==========
void idcu_config_set_env_prefix(const char* prefix) {
    if (prefix) {
        strncpy(g_env_prefix, prefix, sizeof(g_env_prefix) - 1);
        g_env_prefix[sizeof(g_env_prefix) - 1] = '\0';
    }
}

static char* substitute_env_vars(const char* input) {
    if (!input) {
        return NULL;
    }
    
    size_t result_capacity = 256;
    char* result = (char*)malloc(result_capacity);
    if (!result) {
        return NULL;
    }
    result[0] = '\0';
    size_t result_len = 0;
    
    const char* p = input;
    while (*p) {
        if (*p == '$' && *(p+1) == '{') {
            // Found ${VAR} pattern
            p += 2;
            const char* var_start = p;
            while (*p && *p != '}') p++;
            
            if (*p == '}') {
                size_t var_len = p - var_start;
                char var_name[256];
                if (var_len < sizeof(var_name)) {
                    strncpy(var_name, var_start, var_len);
                    var_name[var_len] = '\0';
                    
                    char* env_val = getenv(var_name);
                    if (env_val) {
                        size_t env_len = strlen(env_val);
                        if (result_len + env_len >= result_capacity) {
                            result_capacity = result_len + env_len + 256;
                            char* new_result = (char*)realloc(result, result_capacity);
                            if (!new_result) {
                                free(result);
                                return NULL;
                            }
                            result = new_result;
                        }
                        strcat(result, env_val);
                        result_len += env_len;
                    }
                }
                p++;
            }
        } else {
            if (result_len + 1 >= result_capacity) {
                result_capacity += 256;
                char* new_result = (char*)realloc(result, result_capacity);
                if (!new_result) {
                    free(result);
                    return NULL;
                }
                result = new_result;
            }
            result[result_len++] = *p++;
            result[result_len] = '\0';
        }
    }
    
    return result;
}

int idcu_config_apply_env_overrides(void) {
    if (!g_initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    
    idcu_mutex_lock(&g_config_manager.lock);
    
    for (uint32_t s = 0; s < g_config_manager.section_count; s++) {
        idcu_ConfigSection* section = &g_config_manager.sections[s];
        
        for (uint32_t e = 0; e < section->entry_count; e++) {
            idcu_ConfigEntry* entry = &section->entries[e];
            
            // Check for environment variables with prefix
            char env_key[256];
            snprintf(env_key, sizeof(env_key), "%s%s_%s", 
                     g_env_prefix, section->name, entry->key);
            char* env_val = getenv(env_key);
            
            if (env_val) {
                strncpy(entry->value, env_val, IDCU_CONFIG_VALUE_MAX - 1);
                entry->value[IDCU_CONFIG_VALUE_MAX - 1] = '\0';
            } else {
                // Substitute environment variables in the value
                char* substituted = substitute_env_vars(entry->value);
                if (substituted) {
                    strncpy(entry->value, substituted, IDCU_CONFIG_VALUE_MAX - 1);
                    entry->value[IDCU_CONFIG_VALUE_MAX - 1] = '\0';
                    free(substituted);
                }
            }
        }
    }
    
    idcu_mutex_unlock(&g_config_manager.lock);
    
    return IDCU_ERR_OK;
}

// ========== Command line override ==========
int idcu_config_apply_cmdline(int argc, char* argv[]) {
    if (!g_initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    
    for (int i = 1; i < argc; i++) {
        char* arg = argv[i];
        if (strncmp(arg, "--config.", 9) == 0) {
            char* eq = strchr(arg + 9, '=');
            if (eq) {
                *eq = '\0';
                char* section_key = arg + 9;
                char* value = eq + 1;
                
                char* dot = strchr(section_key, '.');
                if (dot) {
                    *dot = '\0';
                    char* section = section_key;
                    char* key = dot + 1;
                    idcu_config_set_string(section, key, value);
                }
            }
        }
    }
    
    return IDCU_ERR_OK;
}
