#ifndef IDCU_UTILS_TYPES_H
#define IDCU_UTILS_TYPES_H

#include <idcu/common/config.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_MD5_DIGEST_SIZE   16
#define IDCU_SHA256_DIGEST_SIZE 32
#define IDCU_UUID_STRING_SIZE   37

// MD5 上下文
typedef struct {
    uint32_t state[4];
    uint64_t count;
    uint8_t  buffer[64];
} idcu_MD5Context;

// SHA256 上下文
typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t  buffer[64];
} idcu_SHA256Context;

// 应用信息
typedef struct {
    char exe_path[1024];
    char exe_name[256];
    char data_dir[1024];
    char config_dir[1024];
    char log_dir[1024];
} idcu_AppInfo;

#ifdef __cplusplus
}
#endif

#endif
