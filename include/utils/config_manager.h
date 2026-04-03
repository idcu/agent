#ifndef IDCU_UTILS_CONFIG_MANAGER_H
#define IDCU_UTILS_CONFIG_MANAGER_H

#include "common/error_code.h"
#include "common/lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // IDCU_UTILS_CONFIG_MANAGER_H

#define IDCU_MAX_CONFIG_SECTIONS 64
#define IDCU_MAX_CONFIG_KEYS_PER_SECTION 128
#define IDCU_CONFIG_KEY_MAX 64
#define IDCU_CONFIG_VALUE_MAX 256
#define IDCU_CONFIG_SECTION_MAX 64

typedef struct {
    char key[IDCU_CONFIG_KEY_MAX];
    char value[IDCU_CONFIG_VALUE_MAX];
} idcu_ConfigEntry;

typedef struct {
    char name[IDCU_CONFIG_SECTION_MAX];
    idcu_ConfigEntry entries[IDCU_MAX_CONFIG_KEYS_PER_SECTION];
    uint32_t entry_count;
} idcu_ConfigSection;

typedef struct {
    idcu_ConfigSection sections[IDCU_MAX_CONFIG_SECTIONS];
    uint32_t section_count;
    idcu_Mutex lock;
    int loaded;
} idcu_ConfigManager;

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

#ifdef __cplusplus
}
#endif // IDCU_UTILS_CONFIG_MANAGER_H

#endif // IDCU_UTILS_CONFIG_MANAGER_H
