# 任务 5.10: storage-module - 存储业务模块

## 目标

创建存储业务模块，支持：
- 多种存储后端（文件、数据库、对象存储）
- 数据序列化和反序列化
- 数据压缩
- 数据加密
- 数据索引
- 事务支持
- 数据备份和恢复

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/business/storage-module/include/idcu/storage_module
mkdir -p modules/business/storage-module/src/idcu/storage_module
mkdir -p modules/business/storage-module/tests
mkdir -p modules/business/storage-module/examples
```

### 2. 创建存储模块头文件 (storage_module.h)

创建 `modules/business/storage-module/include/idcu/storage_module/storage_module.h`：

```c
#ifndef IDCU_STORAGE_MODULE_STORAGE_MODULE_H
#define IDCU_STORAGE_MODULE_STORAGE_MODULE_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/sdk/sdk.h"
#include "idcu/storage/storage.h"
#include "idcu/json/json.h"
#include "idcu/yaml/yaml.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_StorageId;
typedef uint64_t idcu_TransactionId;

typedef enum
{
    IDCU_STORAGE_BACKEND_FILE = 0,
    IDCU_STORAGE_BACKEND_SQLITE,
    IDCU_STORAGE_BACKEND_MYSQL,
    IDCU_STORAGE_BACKEND_POSTGRESQL,
    IDCU_STORAGE_BACKEND_REDIS,
    IDCU_STORAGE_BACKEND_S3,
    IDCU_STORAGE_BACKEND_CUSTOM
} idcu_StorageBackend;

typedef enum
{
    IDCU_SERIALIZATION_JSON = 0,
    IDCU_SERIALIZATION_YAML,
    IDCU_SERIALIZATION_MESSAGEPACK,
    IDCU_SERIALIZATION_RAW
} idcu_SerializationFormat;

typedef enum
{
    IDCU_COMPRESSION_NONE = 0,
    IDCU_COMPRESSION_GZIP,
    IDCU_COMPRESSION_ZLIB,
    IDCU_COMPRESSION_LZ4
} idcu_CompressionType;

typedef enum
{
    IDCU_ENCRYPTION_NONE = 0,
    IDCU_ENCRYPTION_AES_256,
    IDCU_ENCRYPTION_CHACHA20
} idcu_EncryptionType;

typedef struct
{
    char key[256];
    char value[8192];
    size_t value_size;
    idcu_SerializationFormat format;
    idcu_CompressionType compression;
    idcu_EncryptionType encryption;
    uint64_t created_time;
    uint64_t modified_time;
    uint64_t version;
    char tags[512];
} idcu_StorageRecord;

typedef struct
{
    idcu_TransactionId tx_id;
    int active;
    uint64_t start_time;
    idcu_Vector operations;
} idcu_StorageTransaction;

typedef struct
{
    char config_path[1024];
    idcu_StorageBackend backend;
    char connection_string[2048];
    idcu_SerializationFormat default_format;
    idcu_CompressionType default_compression;
    idcu_EncryptionType default_encryption;
    char encryption_key[256];
    int enable_indexing;
    int enable_transactions;
    char backup_path[1024];
} idcu_StorageModuleConfig;

typedef struct
{
    idcu_SdkContext* sdk;
    idcu_StorageModuleConfig config;
    
    idcu_Storage* storage;
    idcu_HashMap index_map;
    idcu_Mutex storage_lock;
    
    idcu_StorageTransaction* current_tx;
    idcu_Mutex tx_lock;
    
    int initialized;
} idcu_StorageModule;

int  idcu_storage_module_config_init(idcu_StorageModuleConfig* config);

int  idcu_storage_module_init(idcu_StorageModule* module, idcu_SdkContext* sdk,
                                const idcu_StorageModuleConfig* config);
void idcu_storage_module_destroy(idcu_StorageModule* module);

int  idcu_storage_module_start(idcu_StorageModule* module);
int  idcu_storage_module_stop(idcu_StorageModule* module);

int  idcu_storage_module_put(idcu_StorageModule* module, const char* key,
                              const void* value, size_t value_size);
int  idcu_storage_module_put_with_options(idcu_StorageModule* module, const char* key,
                                            const void* value, size_t value_size,
                                            idcu_SerializationFormat format,
                                            idcu_CompressionType compression,
                                            idcu_EncryptionType encryption);

int  idcu_storage_module_get(idcu_StorageModule* module, const char* key,
                              void* value, size_t* value_size);
int  idcu_storage_module_exists(idcu_StorageModule* module, const char* key);

int  idcu_storage_module_delete(idcu_StorageModule* module, const char* key);
int  idcu_storage_module_clear(idcu_StorageModule* module);

int  idcu_storage_module_get_record(idcu_StorageModule* module, const char* key,
                                      idcu_StorageRecord* record);
int  idcu_storage_module_put_record(idcu_StorageModule* module, 
                                      const idcu_StorageRecord* record);

int  idcu_storage_module_begin_transaction(idcu_StorageModule* module,
                                             idcu_TransactionId* out_tx_id);
int  idcu_storage_module_commit_transaction(idcu_StorageModule* module,
                                              idcu_TransactionId tx_id);
int  idcu_storage_module_rollback_transaction(idcu_StorageModule* module,
                                                idcu_TransactionId tx_id);

int  idcu_storage_module_get_all_keys(idcu_StorageModule* module, idcu_Vector* keys);
int  idcu_storage_module_get_keys_by_prefix(idcu_StorageModule* module,
                                               const char* prefix,
                                               idcu_Vector* keys);
int  idcu_storage_module_get_keys_by_tag(idcu_StorageModule* module,
                                            const char* tag,
                                            idcu_Vector* keys);

int  idcu_storage_module_add_tag(idcu_StorageModule* module, const char* key, const char* tag);
int  idcu_storage_module_remove_tag(idcu_StorageModule* module, const char* key, const char* tag);

int  idcu_storage_module_backup(idcu_StorageModule* module, const char* backup_path);
int  idcu_storage_module_restore(idcu_StorageModule* module, const char* backup_path);

int  idcu_storage_module_count(idcu_StorageModule* module, size_t* count);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建模块配置文件 (module.yaml)

创建 `modules/business/storage-module/module.yaml`：

```yaml
name: storage-module
version: 1.0.0
description: Storage business module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-sdk
  - idcu-storage
  - idcu-json
  - idcu-yaml
  - idcu-log

build:
  type: cmake
  targets:
    - storage-module

headers:
  - idcu/storage_module/storage_module.h

features:
  - backends: Multiple storage backends (file, database, object storage)
  - serialization: Data serialization and deserialization
  - compression: Data compression
  - encryption: Data encryption
  - indexing: Data indexing
  - transactions: Transaction support
  - backup: Data backup and restore

testing:
  enabled: true
  framework: internal
```

### 4. 创建 README.md

创建 `modules/business/storage-module/README.md`：

```markdown
# storage-module

IDCU Agent 的存储业务模块。

## 功能特性

- **多后端**: 文件、数据库、对象存储等多种存储后端
- **序列化**: 数据序列化和反序列化
- **压缩**: 数据压缩
- **加密**: 数据加密
- **索引**: 数据索引
- **事务**: 事务支持
- **备份**: 数据备份和恢复

## 快速开始

### 初始化存储模块

```c
#include "idcu/storage_module/storage_module.h"

idcu_StorageModuleConfig config;
idcu_storage_module_config_init(&config);

config.backend = IDCU_STORAGE_BACKEND_FILE;
config.connection_string = "./data";
config.default_format = IDCU_SERIALIZATION_JSON;
config.default_compression = IDCU_COMPRESSION_NONE;
config.default_encryption = IDCU_ENCRYPTION_NONE;
config.enable_indexing = 1;
config.enable_transactions = 1;

idcu_StorageModule module;
idcu_storage_module_init(&module, sdk_context, &config);
```

### 启动存储

```c
idcu_storage_module_start(&module);
```

### 存储数据

```c
const char* key = "user:123";
const char* value = "{\"name\":\"John\",\"age\":30}";
idcu_storage_module_put(&module, key, value, strlen(value));
```

### 存储带选项的数据

```c
const char* key = "config:app";
const char* value = "app_config_data";
idcu_storage_module_put_with_options(&module, key, value, strlen(value),
                                       IDCU_SERIALIZATION_YAML,
                                       IDCU_COMPRESSION_GZIP,
                                       IDCU_ENCRYPTION_AES_256);
```

### 获取数据

```c
char value[1024];
size_t value_size = sizeof(value);

if (idcu_storage_module_get(&module, "user:123", value, &value_size) == IDCU_ERR_OK) {
    printf("Value: %s\n", value);
}
```

### 检查数据是否存在

```c
if (idcu_storage_module_exists(&module, "user:123")) {
    printf("Data exists\n");
}
```

### 删除数据

```c
idcu_storage_module_delete(&module, "user:123");
```

### 使用事务

```c
idcu_TransactionId tx_id;
idcu_storage_module_begin_transaction(&module, &tx_id);

idcu_storage_module_put(&module, "key1", "value1", 6);
idcu_storage_module_put(&module, "key2", "value2", 6);

idcu_storage_module_commit_transaction(&module, tx_id);
```

### 回滚事务

```c
idcu_TransactionId tx_id;
idcu_storage_module_begin_transaction(&module, &tx_id);

idcu_storage_module_put(&module, "key1", "value1", 6);

idcu_storage_module_rollback_transaction(&module, tx_id);
```

### 添加标签

```c
idcu_storage_module_add_tag(&module, "user:123", "user");
idcu_storage_module_add_tag(&module, "user:123", "active");
```

### 按标签查询

```c
idcu_Vector keys;
idcu_vector_init(&keys, sizeof(char*));

idcu_storage_module_get_keys_by_tag(&module, "user", &keys);

for (size_t i = 0; i < keys.size; i++) {
    char* key = *(char**)idcu_vector_get(&keys, i);
    printf("Key: %s\n", key);
}

idcu_vector_destroy(&keys);
```

### 备份数据

```c
idcu_storage_module_backup(&module, "./backup/backup_20260408.dat");
```

### 恢复数据

```c
idcu_storage_module_restore(&module, "./backup/backup_20260408.dat");
```

### 停止存储

```c
idcu_storage_module_stop(&module);
idcu_storage_module_destroy(&module);
```

## 存储后端

| 后端 | 说明 |
|-----|------|
| FILE | 文件存储 |
| SQLITE | SQLite数据库 |
| MYSQL | MySQL数据库 |
| POSTGRESQL | PostgreSQL数据库 |
| REDIS | Redis缓存 |
| S3 | S3对象存储 |
| CUSTOM | 自定义 |

## 序列化格式

| 格式 | 说明 |
|-----|------|
| JSON | JSON格式 |
| YAML | YAML格式 |
| MESSAGEPACK | MessagePack格式 |
| RAW | 原始格式 |

## 压缩类型

| 类型 | 说明 |
|-----|------|
| NONE | 不压缩 |
| GZIP | GZIP压缩 |
| ZLIB | ZLIB压缩 |
| LZ4 | LZ4压缩 |

## 加密类型

| 类型 | 说明 |
|-----|------|
| NONE | 不加密 |
| AES_256 | AES-256加密 |
| CHACHA20 | ChaCha20加密 |

## API 文档

详见 [include/idcu/storage_module/storage_module.h](include/idcu/storage_module/storage_module.h)
```

## 验证检查清单

- [ ] 存储模块头文件已创建
- [ ] 存储模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以存储和获取数据
- [ ] 事务功能正常工作
- [ ] 备份恢复功能正常

## Git 提交

```bash
git add modules/business/storage-module/
git commit -m "feat: add storage-module business module

- Add multiple storage backends (file, database, object storage)
- Add data serialization and deserialization
- Add data compression
- Add data encryption
- Add data indexing
- Add transaction support
- Add data backup and restore
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 存储失败 | 磁盘空间或权限问题 | 检查磁盘空间和权限 |
| 事务冲突 | 并发访问冲突 | 使用合适的事务隔离级别 |
| 备份失败 | 备份路径问题 | 检查备份路径配置 |
