# 任务 3.5: idcu-utils - 工具库

## 目标

创建工具库，支持：
- 字符串工具
- 时间工具
- 编码工具（Base64、URL编码）
- 哈希工具（MD5、SHA）
- 随机数生成
- 文件工具
- 环境变量工具
- 命令执行

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-utils/include/idcu/utils
mkdir -p libs/idcu-utils/src/idcu/utils
mkdir -p libs/idcu-utils/tests
mkdir -p libs/idcu-utils/examples
```

### 2. 创建工具头文件 (utils.h)

创建 `libs/idcu-utils/include/idcu/utils/utils.h`：

```c
#ifndef IDCU_UTILS_UTILS_H
#define IDCU_UTILS_UTILS_H

#include "idcu/common/error_code.h"
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int  idcu_str_trim(char* str);
int  idcu_str_trim_left(char* str);
int  idcu_str_trim_right(char* str);
char* idcu_str_dup(const char* str);
char* idcu_str_ndup(const char* str, size_t n);
int  idcu_str_split(const char* str, char delimiter, char** parts, size_t max_parts, size_t* count);
int  idcu_str_join(const char** parts, size_t count, char delimiter, char* buffer, size_t buffer_size);
int  idcu_str_replace(char* str, const char* search, const char* replace, char* buffer, size_t buffer_size);
int  idcu_str_to_upper(char* str);
int  idcu_str_to_lower(char* str);
int  idcu_str_starts_with(const char* str, const char* prefix);
int  idcu_str_ends_with(const char* str, const char* suffix);
int  idcu_str_contains(const char* str, const char* substr);
int  idcu_str_count(const char* str, const char* substr);
int  idcu_str_parse_int(const char* str, int64_t* value);
int  idcu_str_parse_double(const char* str, double* value);
int  idcu_str_format_int(int64_t value, char* buffer, size_t buffer_size);
int  idcu_str_format_double(double value, int precision, char* buffer, size_t buffer_size);
int  idcu_str_snprintf(char* buffer, size_t buffer_size, const char* format, ...);
char* idcu_str_asprintf(const char* format, ...);

uint64_t idcu_time_now_ms(void);
uint64_t idcu_time_now_us(void);
uint64_t idcu_time_now_ns(void);
int  idcu_time_sleep_ms(uint64_t ms);
int  idcu_time_sleep_us(uint64_t us);
int  idcu_time_format(uint64_t timestamp_ms, const char* format, char* buffer, size_t buffer_size);
int  idcu_time_parse(const char* str, const char* format, uint64_t* timestamp_ms);
int  idcu_time_format_iso8601(uint64_t timestamp_ms, char* buffer, size_t buffer_size);
uint64_t idcu_time_from_iso8601(const char* str);
uint64_t idcu_time_diff_ms(uint64_t start, uint64_t end);
int  idcu_time_get_local(time_t* t, struct tm* tm);
int  idcu_time_get_utc(time_t* t, struct tm* tm);

int  idcu_base64_encode(const uint8_t* data, size_t data_len, char* output, size_t* output_len);
int  idcu_base64_decode(const char* input, uint8_t* output, size_t* output_len);
size_t idcu_base64_encode_len(size_t data_len);
size_t idcu_base64_decode_len(const char* input, size_t input_len);

int  idcu_url_encode(const char* str, char* output, size_t* output_len);
int  idcu_url_decode(const char* input, char* output, size_t* output_len);

typedef struct idcu_MD5Context
{
    uint32_t state[4];
    uint32_t count[2];
    uint8_t buffer[64];
} idcu_MD5Context;

typedef struct idcu_SHA256Context
{
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];
} idcu_SHA256Context;

void idcu_md5_init(idcu_MD5Context* ctx);
void idcu_md5_update(idcu_MD5Context* ctx, const uint8_t* data, size_t len);
void idcu_md5_final(idcu_MD5Context* ctx, uint8_t digest[16]);
int  idcu_md5(const uint8_t* data, size_t len, uint8_t digest[16]);
int  idcu_md5_file(const char* path, uint8_t digest[16]);
int  idcu_md5_to_string(const uint8_t digest[16], char* buffer, size_t buffer_size);

void idcu_sha256_init(idcu_SHA256Context* ctx);
void idcu_sha256_update(idcu_SHA256Context* ctx, const uint8_t* data, size_t len);
void idcu_sha256_final(idcu_SHA256Context* ctx, uint8_t digest[32]);
int  idcu_sha256(const uint8_t* data, size_t len, uint8_t digest[32]);
int  idcu_sha256_file(const char* path, uint8_t digest[32]);
int  idcu_sha256_to_string(const uint8_t digest[32], char* buffer, size_t buffer_size);

void idcu_random_seed(uint64_t seed);
uint32_t idcu_random_uint32(void);
uint64_t idcu_random_uint64(void);
int  idcu_random_int(int min, int max);
double idcu_random_double(void);
int  idcu_random_bytes(uint8_t* buffer, size_t len);
int  idcu_random_uuid(char* buffer, size_t buffer_size);

int  idcu_file_exists(const char* path);
int  idcu_file_is_file(const char* path);
int  idcu_file_is_dir(const char* path);
int64_t idcu_file_size(const char* path);
int  idcu_file_read(const char* path, char* buffer, size_t buffer_size, size_t* read_len);
int  idcu_file_read_alloc(const char* path, char** buffer, size_t* size);
int  idcu_file_write(const char* path, const void* data, size_t len);
int  idcu_file_append(const char* path, const void* data, size_t len);
int  idcu_file_copy(const char* src, const char* dst);
int  idcu_file_move(const char* src, const char* dst);
int  idcu_file_delete(const char* path);
int  idcu_file_mkdir(const char* path, int recursive);
int  idcu_file_rmdir(const char* path, int recursive);
int  idcu_file_list(const char* path, char** files, size_t max_files, size_t* count);
int  idcu_file_getcwd(char* buffer, size_t buffer_size);
int  idcu_file_chdir(const char* path);
int  idcu_file_join_path(const char* a, const char* b, char* buffer, size_t buffer_size);
int  idcu_file_dirname(const char* path, char* buffer, size_t buffer_size);
int  idcu_file_basename(const char* path, char* buffer, size_t buffer_size);
int  idcu_file_extname(const char* path, char* buffer, size_t buffer_size);

int  idcu_env_get(const char* name, char* buffer, size_t buffer_size);
int  idcu_env_set(const char* name, const char* value, int overwrite);
int  idcu_env_unset(const char* name);
int  idcu_env_exists(const char* name);
int  idcu_env_get_int(const char* name, int64_t* value, int64_t default_value);
int  idcu_env_get_double(const char* name, double* value, double default_value);
int  idcu_env_get_bool(const char* name, int* value, int default_value);

int  idcu_exec(const char* cmd, char* output, size_t output_size, int* exit_code);
int  idcu_exec_async(const char* cmd, void** handle);
int  idcu_exec_wait(void* handle, char* output, size_t output_size, int* exit_code);
int  idcu_exec_kill(void* handle);
int  idcu_exec_get_pid(void* handle);
int  idcu_exec_is_running(void* handle);
int  idcu_exec_get_exit_code(void* handle);

typedef struct
{
    char path[1024];
    char name[256];
    char version[64];
    char os[64];
    char arch[64];
} idcu_AppInfo;

int  idcu_app_info_init(idcu_AppInfo* info);
int  idcu_app_info_get_exe_path(char* buffer, size_t buffer_size);
int  idcu_app_info_get_exe_dir(char* buffer, size_t buffer_size);
int  idcu_app_info_get_data_dir(char* buffer, size_t buffer_size);
int  idcu_app_info_get_config_dir(char* buffer, size_t buffer_size);
int  idcu_app_info_get_cache_dir(char* buffer, size_t buffer_size);
int  idcu_app_info_get_temp_dir(char* buffer, size_t buffer_size);
int  idcu_app_info_get_home_dir(char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-utils/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-utils VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-utils STATIC
    src/idcu/utils/utils.c
    src/idcu/utils/string.c
    src/idcu/utils/time.c
    src/idcu/utils/base64.c
    src/idcu/utils/md5.c
    src/idcu/utils/sha256.c
    src/idcu/utils/random.c
    src/idcu/utils/file.c
    src/idcu/utils/env.c
    src/idcu/utils/exec.c
)

target_include_directories(idcu-utils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-utils PRIVATE
    idcu::common
)

if(WIN32)
    target_link_libraries(idcu-utils PRIVATE advapi32 shell32)
else()
    target_link_libraries(idcu-utils PRIVATE dl)
endif()

add_library(idcu::utils ALIAS idcu-utils)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-utils/module.yaml`：

```yaml
name: idcu-utils
version: 1.0.0
description: Utility library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common

build:
  type: cmake
  targets:
    - idcu-utils

headers:
  - idcu/utils/utils.h

features:
  - string: String utilities
  - time: Time utilities
  - base64: Base64 encoding/decoding
  - url: URL encoding/decoding
  - hash: MD5, SHA256 hashing
  - random: Random number generation
  - file: File utilities
  - env: Environment variable utilities
  - exec: Command execution
  - app: Application information

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-utils/README.md`：

```markdown
# idcu-utils

IDCU Agent 的工具库。

## 功能特性

- **字符串工具**: 字符串处理工具
- **时间工具**: 时间处理工具
- **Base64**: Base64 编码/解码
- **URL编码**: URL 编码/解码
- **哈希**: MD5、SHA256 哈希
- **随机数**: 随机数生成
- **文件工具**: 文件处理工具
- **环境变量**: 环境变量工具
- **命令执行**: 命令执行
- **应用信息**: 应用信息

## 快速开始

### 字符串工具

```c
#include "idcu/utils/utils.h"

char str[] = "  Hello World!  ";
idcu_str_trim(str);
printf("Trimmed: '%s'\n", str);

char* dup = idcu_str_dup("Hello");
printf("Duplicate: %s\n", dup);
free(dup);

int64_t value;
idcu_str_parse_int("12345", &value);
printf("Parsed: %" PRId64 "\n", value);
```

### 时间工具

```c
uint64_t now = idcu_time_now_ms();
printf("Now: %" PRIu64 " ms\n", now);

char buffer[64];
idcu_time_format_iso8601(now, buffer, sizeof(buffer));
printf("ISO8601: %s\n", buffer);

idcu_time_sleep_ms(1000);
```

### Base64

```c
const char* input = "Hello, World!";
char encoded[256];
size_t encoded_len;

idcu_base64_encode((const uint8_t*)input, strlen(input), encoded, &encoded_len);
printf("Encoded: %s\n", encoded);

uint8_t decoded[256];
size_t decoded_len;
idcu_base64_decode(encoded, decoded, &decoded_len);
```

### 哈希

```c
uint8_t md5_digest[16];
idcu_md5((const uint8_t*)"Hello", 5, md5_digest);

char md5_str[33];
idcu_md5_to_string(md5_digest, md5_str, sizeof(md5_str));
printf("MD5: %s\n", md5_str);

uint8_t sha256_digest[32];
idcu_sha256((const uint8_t*)"Hello", 5, sha256_digest);

char sha256_str[65];
idcu_sha256_to_string(sha256_digest, sha256_str, sizeof(sha256_str));
printf("SHA256: %s\n", sha256_str);
```

### 随机数

```c
uint32_t r = idcu_random_uint32();
printf("Random: %u\n", r);

int i = idcu_random_int(1, 100);
printf("Random int: %d\n", i);

char uuid[37];
idcu_random_uuid(uuid, sizeof(uuid));
printf("UUID: %s\n", uuid);
```

### 文件工具

```c
if (idcu_file_exists("/path/to/file")) {
    int64_t size = idcu_file_size("/path/to/file");
    printf("Size: %" PRId64 "\n", size);
}

char* content;
size_t size;
idcu_file_read_alloc("/path/to/file", &content, &size);
printf("Content: %.*s\n", (int)size, content);
free(content);

idcu_file_write("/path/to/file", "Hello", 5);

idcu_file_mkdir("/path/to/dir", 1);
```

### 环境变量

```c
char value[256];
if (idcu_env_get("HOME", value, sizeof(value)) == IDCU_ERR_OK) {
    printf("HOME: %s\n", value);
}

idcu_env_set("MY_VAR", "my_value", 1);

int64_t port;
idcu_env_get_int("PORT", &port, 8080);
```

### 命令执行

```c
char output[1024];
int exit_code;

if (idcu_exec("echo hello", output, sizeof(output), &exit_code) == IDCU_ERR_OK) {
    printf("Output: %s\n", output);
    printf("Exit code: %d\n", exit_code);
}
```

### 应用信息

```c
char exe_path[1024];
idcu_app_info_get_exe_path(exe_path, sizeof(exe_path));
printf("Exe path: %s\n", exe_path);

char data_dir[1024];
idcu_app_info_get_data_dir(data_dir, sizeof(data_dir));
printf("Data dir: %s\n", data_dir);
```

## API 文档

详见 [include/idcu/utils/utils.h](include/idcu/utils/utils.h)
```

## 验证检查清单

- [ ] 工具头文件已创建
- [ ] 工具实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 字符串工具可以正常使用
- [ ] 时间工具可以正常使用
- [ ] 文件工具可以正常使用

## Git 提交

```bash
git add libs/idcu-utils/
git commit -m "feat: add idcu-utils library

- Add string utilities
- Add time utilities
- Add Base64 encoding/decoding
- Add URL encoding/decoding
- Add MD5, SHA256 hashing
- Add random number generation
- Add file utilities
- Add environment variable utilities
- Add command execution
- Add application information
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 字符串处理错误 | 缓冲区溢出 | 使用足够大的缓冲区 |
| 时间解析错误 | 格式不匹配 | 确保使用正确的格式字符串 |
| 文件操作失败 | 权限问题 | 检查文件权限 |
