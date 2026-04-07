#ifndef IDCU_CONFIG_CONFIG_H
#define IDCU_CONFIG_CONFIG_H

#include "idcu/common/error_code.h"
#include "idcu/common/lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_CONFIG_MAX_SECTIONS 64
#define IDCU_CONFIG_MAX_KEYS_PER_SECTION 128
#define IDCU_CONFIG_KEY_MAX 128
#define IDCU_CONFIG_VALUE_MAX 512
#ifndef IDCU_CONFIG_SECTION_MAX
#define IDCU_CONFIG_SECTION_MAX 128
#endif
#define IDCU_CONFIG_MAX_LIST_ITEMS 32
#define IDCU_CONFIG_LIST_ITEM_MAX 128
#ifndef IDCU_CONFIG_PATH_MAX
#define IDCU_CONFIG_PATH_MAX 1024
#endif

typedef struct {
    char key[IDCU_CONFIG_KEY_MAX];
    char value[IDCU_CONFIG_VALUE_MAX];
} idcu_ConfigEntry;

typedef struct {
    char name[IDCU_CONFIG_SECTION_MAX];
    idcu_ConfigEntry entries[IDCU_CONFIG_MAX_KEYS_PER_SECTION];
    uint32_t entry_count;
} idcu_ConfigSection;

typedef struct {
    idcu_ConfigSection sections[IDCU_CONFIG_MAX_SECTIONS];
    uint32_t section_count;
    idcu_Mutex lock;
    int loaded;
    int env_var_enabled;
    int validation_enabled;
} idcu_ConfigManager;

typedef struct {
    char items[IDCU_CONFIG_MAX_LIST_ITEMS][IDCU_CONFIG_LIST_ITEM_MAX];
    int count;
} idcu_ConfigList;

int idcu_config_init(const char* file_path);
void idcu_config_shutdown(void);
int idcu_config_is_loaded(void);
int idcu_config_reload(void);
int idcu_config_save(const char* file_path);

const char* idcu_config_get_string(const char* section, const char* key, const char* default_value);
int idcu_config_get_int(const char* section, const char* key, int default_value);
int64_t idcu_config_get_int64(const char* section, const char* key, int64_t default_value);
double idcu_config_get_double(const char* section, const char* key, double default_value);
int idcu_config_get_bool(const char* section, const char* key, int default_value);

int idcu_config_set_string(const char* section, const char* key, const char* value);
int idcu_config_set_int(const char* section, const char* key, int value);
int idcu_config_set_int64(const char* section, const char* key, int64_t value);
int idcu_config_set_double(const char* section, const char* key, double value);
int idcu_config_set_bool(const char* section, const char* key, int value);

int idcu_config_has_section(const char* section);
int idcu_config_has_key(const char* section, const char* key);
int idcu_config_remove_key(const char* section, const char* key);
int idcu_config_remove_section(const char* section);

int idcu_config_get_list(const char* section, const char* key, const char* delimiter, idcu_ConfigList* out_list);
int idcu_config_list_contains(const char* section, const char* key, const char* delimiter, const char* value);

int idcu_config_get_nested_bool(const char* section, const char* prefix, const char* subkey, int default_value);
int idcu_config_get_nested_int(const char* section, const char* prefix, const char* subkey, int default_value);
const char* idcu_config_get_nested_string(const char* section, const char* prefix, const char* subkey, const char* default_value);

void idcu_config_enable_env_var(int enable);
void idcu_config_enable_validation(int enable);
int idcu_config_validate(void);

int idcu_config_load_profile(const char* profile_name);

typedef void (*idcu_ConfigChangeCallback)(const char* section, const char* key, const char* old_value, const char* new_value, void* user_data);

int idcu_config_register_change_callback(idcu_ConfigChangeCallback callback, void* user_data);
int idcu_config_unregister_change_callback(idcu_ConfigChangeCallback callback);
void idcu_config_notify_changes(void);

int idcu_config_get_file_path(char* buffer, size_t buffer_size);
int idcu_config_get_last_modified_time(uint64_t* timestamp);

int idcu_config_watch_start(uint32_t interval_ms);
void idcu_config_watch_stop(void);
int idcu_config_watch_is_running(void);

#ifdef __cplusplus
}
#endif

#endif
