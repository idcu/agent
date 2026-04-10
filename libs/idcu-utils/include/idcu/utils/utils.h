#ifndef IDCU_UTILS_UTILS_H
#define IDCU_UTILS_UTILS_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/utils/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ========== 字符串工具 ==========
int   idcu_str_trim(char* str);
int   idcu_str_trim_left(char* str);
int   idcu_str_trim_right(char* str);
char* idcu_str_dup(const char* str);
char* idcu_str_ndup(const char* str, size_t n);
int   idcu_str_split(const char* str, char delimiter, char** parts, size_t max_parts, size_t* count);
int   idcu_str_starts_with(const char* str, const char* prefix);
int   idcu_str_ends_with(const char* str, const char* suffix);
int   idcu_str_replace(char* str, size_t buffer_size, const char* old_str, const char* new_str);

// ========== 时间工具 ==========
uint64_t idcu_time_now_ms(void);
uint64_t idcu_time_now_us(void);
uint64_t idcu_time_now_ns(void);
int      idcu_time_sleep_ms(uint64_t ms);
int      idcu_time_format(uint64_t timestamp_ms, const char* format, char* buffer, size_t buffer_size);
int      idcu_time_format_iso8601(uint64_t timestamp_ms, char* buffer, size_t buffer_size);

// ========== 编码工具 ==========
int idcu_base64_encode(const uint8_t* data, size_t data_len, char* output, size_t* output_len);
int idcu_base64_decode(const char* input, uint8_t* output, size_t* output_len);
int idcu_url_encode(const char* str, char* output, size_t* output_len);
int idcu_url_decode(const char* input, char* output, size_t* output_len);

// ========== 哈希工具 ==========
int idcu_md5_init(idcu_MD5Context* ctx);
int idcu_md5_update(idcu_MD5Context* ctx, const uint8_t* data, size_t len);
int idcu_md5_final(idcu_MD5Context* ctx, uint8_t digest[IDCU_MD5_DIGEST_SIZE]);
int idcu_md5(const uint8_t* data, size_t len, uint8_t digest[IDCU_MD5_DIGEST_SIZE]);
int idcu_sha256_init(idcu_SHA256Context* ctx);
int idcu_sha256_update(idcu_SHA256Context* ctx, const uint8_t* data, size_t len);
int idcu_sha256_final(idcu_SHA256Context* ctx, uint8_t digest[IDCU_SHA256_DIGEST_SIZE]);
int idcu_sha256(const uint8_t* data, size_t len, uint8_t digest[IDCU_SHA256_DIGEST_SIZE]);

// ========== 随机数 ==========
void idcu_random_seed(uint64_t seed);
uint32_t idcu_random_uint32(void);
int idcu_random_int(int min, int max);
int idcu_random_uuid(char* buffer, size_t buffer_size);

// ========== 文件工具 ==========
int  idcu_file_exists(const char* path);
int64_t idcu_file_size(const char* path);
int  idcu_file_read(const char* path, char* buffer, size_t buffer_size, size_t* read_len);
int  idcu_file_write(const char* path, const void* data, size_t len);
int  idcu_file_delete(const char* path);
int  idcu_file_mkdir(const char* path);
char* idcu_file_read_all(const char* path, size_t* out_len);

// ========== 环境变量 ==========
int idcu_env_get(const char* name, char* buffer, size_t buffer_size);
int idcu_env_set(const char* name, const char* value, int overwrite);
int idcu_env_unset(const char* name);
int idcu_env_get_int(const char* name, int64_t* value, int64_t default_value);

// ========== 应用信息 ==========
int idcu_app_info_init(idcu_AppInfo* info);
int idcu_app_info_get_exe_path(char* buffer, size_t buffer_size);
int idcu_app_info_get_exe_name(char* buffer, size_t buffer_size);
int idcu_app_info_get_data_dir(char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
