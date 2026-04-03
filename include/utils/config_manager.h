#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "common/error_code.h"
#include "common/lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CONFIG_SECTIONS 64
#define MAX_CONFIG_KEYS_PER_SECTION 128
#define CONFIG_KEY_MAX 64
#define CONFIG_VALUE_MAX 256
#define CONFIG_SECTION_MAX 64

typedef struct {
    char key[CONFIG_KEY_MAX];
    char value[CONFIG_VALUE_MAX];
} ConfigEntry;

typedef struct {
    char name[CONFIG_SECTION_MAX];
    ConfigEntry entries[MAX_CONFIG_KEYS_PER_SECTION];
    uint32_t entry_count;
} ConfigSection;

typedef struct {
    ConfigSection sections[MAX_CONFIG_SECTIONS];
    uint32_t section_count;
    Mutex lock;
    int loaded;
} ConfigManager;

int config_init(const char* file_path);
void config_shutdown(void);
int config_is_loaded(void);
int config_reload(void);
int config_save(const char* file_path);

const char* config_get_string(const char* section, const char* key, const char* default_value);
int config_get_int(const char* section, const char* key, int default_value);
int64_t config_get_int64(const char* section, const char* key, int64_t default_value);
double config_get_double(const char* section, const char* key, double default_value);
int config_get_bool(const char* section, const char* key, int default_value);

int config_set_string(const char* section, const char* key, const char* value);
int config_set_int(const char* section, const char* key, int value);
int config_set_int64(const char* section, const char* key, int64_t value);
int config_set_double(const char* section, const char* key, double value);
int config_set_bool(const char* section, const char* key, int value);

int config_has_section(const char* section);
int config_has_key(const char* section, const char* key);
int config_remove_key(const char* section, const char* key);
int config_remove_section(const char* section);

#ifdef __cplusplus
}
#endif

#endif
