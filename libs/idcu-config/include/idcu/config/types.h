#ifndef IDCU_CONFIG_TYPES_H
#define IDCU_CONFIG_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_CONFIG_MAX_SECTIONS         64
#define IDCU_CONFIG_MAX_KEYS_PER_SECTION 128
#define IDCU_CONFIG_KEY_MAX              128
#define IDCU_CONFIG_VALUE_MAX            512
#define IDCU_CONFIG_MAX_CALLBACKS        16
#define IDCU_CONFIG_LIST_MAX_ITEMS       32
#define IDCU_CONFIG_LIST_ITEM_MAX        128

// 配置项
typedef struct {
    char key[IDCU_CONFIG_KEY_MAX];
    char value[IDCU_CONFIG_VALUE_MAX];
} idcu_ConfigEntry;

// 配置节
typedef struct {
    char             name[IDCU_CONFIG_SECTION_MAX];
    idcu_ConfigEntry entries[IDCU_CONFIG_MAX_KEYS_PER_SECTION];
    uint32_t         entry_count;
} idcu_ConfigSection;

// 配置列表
typedef struct {
    char items[IDCU_CONFIG_LIST_MAX_ITEMS][IDCU_CONFIG_LIST_ITEM_MAX];
    int  count;
} idcu_ConfigList;

// 配置变更回调
typedef void (*idcu_ConfigChangeCallback)(const char* section, const char* key,
                                           const char* old_value, const char* new_value,
                                           void* user_data);

// 回调注册项
typedef struct {
    idcu_ConfigChangeCallback callback;
    void* user_data;
} idcu_ConfigCallbackItem;

// 配置管理器
typedef struct {
    idcu_ConfigSection    sections[IDCU_CONFIG_MAX_SECTIONS];
    uint32_t              section_count;
    idcu_Mutex            lock;
    int                   loaded;
    char                  file_path[IDCU_CONFIG_PATH_MAX];
    idcu_ConfigCallbackItem callbacks[IDCU_CONFIG_MAX_CALLBACKS];
    int                   callback_count;
    int                   env_var_enabled;
    char                  current_profile[64];
} idcu_ConfigManager;

#ifdef __cplusplus
}
#endif

#endif
