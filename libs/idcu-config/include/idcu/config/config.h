#ifndef IDCU_CONFIG_CONFIG_H
#define IDCU_CONFIG_CONFIG_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/config/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ========== 核心 API ==========
int   idcu_config_init(const char* file_path);
void  idcu_config_shutdown(void);
int   idcu_config_is_loaded(void);
int   idcu_config_reload(void);
int   idcu_config_save(const char* file_path);

// ========== 类型安全访问 ==========
const char* idcu_config_get_string(const char* section, const char* key, const char* default_value);
int         idcu_config_get_int(const char* section, const char* key, int default_value);
int64_t     idcu_config_get_int64(const char* section, const char* key, int64_t default_value);
double      idcu_config_get_double(const char* section, const char* key, double default_value);
int         idcu_config_get_bool(const char* section, const char* key, int default_value);

// ========== 配置写入 ==========
int idcu_config_set_string(const char* section, const char* key, const char* value);
int idcu_config_set_int(const char* section, const char* key, int value);
int idcu_config_set_int64(const char* section, const char* key, int64_t value);
int idcu_config_set_double(const char* section, const char* key, double value);
int idcu_config_set_bool(const char* section, const char* key, int value);

// ========== 列表支持 ==========
int idcu_config_get_list(const char* section, const char* key, const char* delimiter, idcu_ConfigList* out_list);
int idcu_config_set_list(const char* section, const char* key, const char* delimiter, const idcu_ConfigList* list);

// ========== 变更通知 ==========
int idcu_config_register_change_callback(idcu_ConfigChangeCallback callback, void* user_data);
int idcu_config_unregister_change_callback(idcu_ConfigChangeCallback callback);

// ========== 文件监控 (简化版) ==========
int  idcu_config_watch_start(uint32_t interval_ms);
void idcu_config_watch_stop(void);
int  idcu_config_watch_is_running(void);

// ========== 环境变量支持 ==========
void idcu_config_enable_env_var(int enable);
int  idcu_config_load_profile(const char* profile_name);

#ifdef __cplusplus
}
#endif

#endif
