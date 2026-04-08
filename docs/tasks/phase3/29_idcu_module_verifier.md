# 任务 3.29: idcu-module-verifier - 模块验证

## 目标

创建模块验证库，支持：
- 模块签名验证
- 模块完整性检查
- 模块依赖验证
- 模块兼容性检查
- 模块安全扫描
- 模块性能验证
- 模块测试验证

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-module-verifier/include/idcu/module_verifier
mkdir -p libs/idcu-module-verifier/src/idcu/module_verifier
mkdir -p libs/idcu-module-verifier/tests
mkdir -p libs/idcu-module-verifier/examples
```

### 2. 创建模块验证头文件 (module_verifier.h)

创建 `libs/idcu-module-verifier/include/idcu/module_verifier/module_verifier.h`：

```c
#ifndef IDCU_MODULE_VERIFIER_MODULE_VERIFIER_H
#define IDCU_MODULE_VERIFIER_MODULE_VERIFIER_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_VerificationId;

typedef enum
{
    IDCU_VERIFY_STATUS_PENDING = 0,
    IDCU_VERIFY_STATUS_RUNNING,
    IDCU_VERIFY_STATUS_PASSED,
    IDCU_VERIFY_STATUS_FAILED,
    IDCU_VERIFY_STATUS_WARNING,
    IDCU_VERIFY_STATUS_SKIPPED
} idcu_VerifyStatus;

typedef enum
{
    IDCU_VERIFY_CHECK_SIGNATURE = 0,
    IDCU_VERIFY_CHECK_INTEGRITY,
    IDCU_VERIFY_CHECK_DEPENDENCIES,
    IDCU_VERIFY_CHECK_COMPATIBILITY,
    IDCU_VERIFY_CHECK_SECURITY,
    IDCU_VERIFY_CHECK_PERFORMANCE,
    IDCU_VERIFY_CHECK_TESTS,
    IDCU_VERIFY_CHECK_API,
    IDCU_VERIFY_CHECK_MEMORY_SAFETY,
    IDCU_VERIFY_CHECK_THREAD_SAFETY
} idcu_VerifyCheckType;

typedef struct
{
    idcu_VerifyCheckType type;
    idcu_VerifyStatus status;
    char check_name[128];
    char description[512];
    char error_message[1024];
    uint64_t start_time;
    uint64_t end_time;
    uint64_t duration_ms;
} idcu_VerifyCheck;

typedef struct
{
    char module_name[128];
    char module_version[64];
    char module_path[1024];
    char module_hash[256];
    char signature[1024];
    char public_key[2048];
} idcu_ModuleInfo;

typedef struct
{
    char name[128];
    char version[64];
    char min_version[64];
    char max_version[64];
    int optional;
} idcu_ModuleDependency;

typedef struct
{
    char api_name[256];
    char expected_signature[1024];
    char actual_signature[1024];
    int present;
    int compatible;
} idcu_ApiCheck;

typedef struct
{
    char issue_id[128];
    char severity[32];
    char description[1024];
    char file_path[1024];
    int line_number;
} idcu_SecurityIssue;

typedef struct
{
    char metric_name[128];
    double expected_value;
    double actual_value;
    char unit[32];
    int passed;
} idcu_PerformanceMetric;

typedef struct
{
    idcu_VerificationId id;
    idcu_ModuleInfo module_info;
    idcu_VerifyStatus overall_status;
    
    idcu_Vector checks;
    idcu_Vector dependencies;
    idcu_Vector api_checks;
    idcu_Vector security_issues;
    idcu_Vector performance_metrics;
    
    char summary[2048];
    uint64_t start_time;
    uint64_t end_time;
    uint64_t total_duration_ms;
    
    int passed_count;
    int failed_count;
    int warning_count;
    int skipped_count;
} idcu_VerificationReport;

typedef struct
{
    int enable_signature_check;
    int enable_integrity_check;
    int enable_dependency_check;
    int enable_compatibility_check;
    int enable_security_check;
    int enable_performance_check;
    int enable_test_check;
    int enable_api_check;
    int enable_memory_safety_check;
    int enable_thread_safety_check;
    
    char trusted_keys_dir[1024];
    char compatible_versions[1024];
    uint64_t max_verify_time_ms;
    int strict_mode;
} idcu_ModuleVerifierConfig;

typedef int (*idcu_CustomVerifyFunc)(const idcu_ModuleInfo* module, 
                                       idcu_VerifyCheck* check,
                                       void* user_data);

typedef struct
{
    idcu_ModuleVerifierConfig config;
    idcu_Vector verification_reports;
    idcu_HashMap module_cache;
    idcu_Mutex lock;
    
    idcu_CustomVerifyFunc custom_verify;
    void* custom_user_data;
    
    int initialized;
} idcu_ModuleVerifier;

int  idcu_module_verifier_config_init(idcu_ModuleVerifierConfig* config);

int  idcu_module_verifier_init(idcu_ModuleVerifier* verifier, 
                                 const idcu_ModuleVerifierConfig* config);
void idcu_module_verifier_destroy(idcu_ModuleVerifier* verifier);

int  idcu_module_verifier_verify(idcu_ModuleVerifier* verifier, 
                                   const char* module_path,
                                   idcu_VerificationId* out_id);
int  idcu_module_verifier_verify_async(idcu_ModuleVerifier* verifier, 
                                         const char* module_path);

int  idcu_module_verifier_get_report(idcu_ModuleVerifier* verifier, 
                                       idcu_VerificationId id,
                                       idcu_VerificationReport* report);
int  idcu_module_verifier_get_latest_report(idcu_ModuleVerifier* verifier,
                                              const char* module_path,
                                              idcu_VerificationReport* report);

int  idcu_module_verifier_cancel(idcu_ModuleVerifier* verifier, idcu_VerificationId id);

int  idcu_module_verifier_add_trusted_key(idcu_ModuleVerifier* verifier, 
                                            const char* key_name,
                                            const char* public_key);
int  idcu_module_verifier_remove_trusted_key(idcu_ModuleVerifier* verifier, 
                                               const char* key_name);

int  idcu_module_verifier_set_custom_verify(idcu_ModuleVerifier* verifier,
                                              idcu_CustomVerifyFunc func,
                                              void* user_data);

int  idcu_module_verifier_export_report(idcu_ModuleVerifier* verifier,
                                          idcu_VerificationId id,
                                          char* buffer,
                                          size_t buffer_size,
                                          const char* format);

int  idcu_module_verifier_get_status(idcu_ModuleVerifier* verifier,
                                       idcu_VerificationId id,
                                       idcu_VerifyStatus* status);

int  idcu_verification_report_init(idcu_VerificationReport* report);
void idcu_verification_report_destroy(idcu_VerificationReport* report);

int  idcu_verification_report_add_check(idcu_VerificationReport* report, 
                                          const idcu_VerifyCheck* check);
int  idcu_verification_report_add_dependency(idcu_VerificationReport* report,
                                               const idcu_ModuleDependency* dep);
int  idcu_verification_report_add_security_issue(idcu_VerificationReport* report,
                                                   const idcu_SecurityIssue* issue);
int  idcu_verification_report_add_performance_metric(idcu_VerificationReport* report,
                                                      const idcu_PerformanceMetric* metric);

int  idcu_verification_report_summary(idcu_VerificationReport* report, 
                                        char* buffer,
                                        size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-module-verifier/module.yaml`：

```yaml
name: idcu-module-verifier
version: 1.0.0
description: Module verification library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-log
  - idcu-json
  - idcu-yaml

build:
  type: cmake
  targets:
    - idcu-module-verifier

headers:
  - idcu/module_verifier/module_verifier.h

features:
  - signature: Module signature verification
  - integrity: Module integrity check
  - dependencies: Module dependency verification
  - compatibility: Module compatibility check
  - security: Module security scanning
  - performance: Module performance verification
  - tests: Module test verification
  - api: API compatibility check

testing:
  enabled: true
  framework: internal
```

### 4. 创建 README.md

创建 `libs/idcu-module-verifier/README.md`：

```markdown
# idcu-module-verifier

IDCU Agent 的模块验证库。

## 功能特性

- **签名验证**: 模块签名验证
- **完整性检查**: 模块完整性检查
- **依赖验证**: 模块依赖验证
- **兼容性检查**: 模块兼容性检查
- **安全扫描**: 模块安全扫描
- **性能验证**: 模块性能验证
- **测试验证**: 模块测试验证
- **API检查**: API兼容性检查

## 快速开始

### 初始化验证器

```c
#include "idcu/module_verifier/module_verifier.h"

idcu_ModuleVerifierConfig config;
idcu_module_verifier_config_init(&config);

config.enable_signature_check = 1;
config.enable_integrity_check = 1;
config.enable_dependency_check = 1;
config.enable_compatibility_check = 1;
config.enable_security_check = 1;
config.enable_performance_check = 1;
config.enable_test_check = 1;
config.strict_mode = 0;

idcu_ModuleVerifier verifier;
idcu_module_verifier_init(&verifier, &config);
```

### 添加受信任的密钥

```c
idcu_module_verifier_add_trusted_key(&verifier, "default", 
                                      "-----BEGIN PUBLIC KEY-----\n...\n-----END PUBLIC KEY-----");
```

### 验证模块

```c
idcu_VerificationId verify_id;
idcu_module_verifier_verify(&verifier, "./modules/my_module.so", &verify_id);
```

### 获取验证报告

```c
idcu_VerificationReport report;
idcu_verification_report_init(&report);

idcu_module_verifier_get_report(&verifier, verify_id, &report);

printf("Overall status: %d\n", report.overall_status);
printf("Passed: %d, Failed: %d, Warnings: %d\n",
       report.passed_count, report.failed_count, report.warning_count);

for (size_t i = 0; i < report.checks.size; i++) {
    idcu_VerifyCheck* check = (idcu_VerifyCheck*)idcu_vector_get(&report.checks, i);
    printf("[%s] %s: %s\n",
           check->status == IDCU_VERIFY_STATUS_PASSED ? "PASS" : "FAIL",
           check->check_name,
           check->error_message[0] ? check->error_message : "OK");
}

idcu_verification_report_destroy(&report);
```

### 导出报告

```c
char buffer[32768];
idcu_module_verifier_export_report(&verifier, verify_id, buffer, sizeof(buffer), "json");
printf("%s\n", buffer);
```

### 自定义验证

```c
int custom_verify(const idcu_ModuleInfo* module, idcu_VerifyCheck* check, void* user_data)
{
    // 自定义验证逻辑
    check->status = IDCU_VERIFY_STATUS_PASSED;
    return 0;
}

idcu_module_verifier_set_custom_verify(&verifier, custom_verify, NULL);
```

### 清理

```c
idcu_module_verifier_destroy(&verifier);
```

## 验证状态

| 状态 | 说明 |
|-----|------|
| PENDING | 待验证 |
| RUNNING | 验证中 |
| PASSED | 通过 |
| FAILED | 失败 |
| WARNING | 警告 |
| SKIPPED | 跳过 |

## 验证检查类型

| 类型 | 说明 |
|-----|------|
| SIGNATURE | 签名验证 |
| INTEGRITY | 完整性检查 |
| DEPENDENCIES | 依赖验证 |
| COMPATIBILITY | 兼容性检查 |
| SECURITY | 安全扫描 |
| PERFORMANCE | 性能验证 |
| TESTS | 测试验证 |
| API | API检查 |
| MEMORY_SAFETY | 内存安全 |
| THREAD_SAFETY | 线程安全 |

## 安全问题级别

| 级别 | 说明 |
|-----|------|
| INFO | 信息 |
| LOW | 低 |
| MEDIUM | 中 |
| HIGH | 高 |
| CRITICAL | 严重 |

## API 文档

详见 [include/idcu/module_verifier/module_verifier.h](include/idcu/module_verifier/module_verifier.h)
```

## 验证检查清单

- [ ] 模块验证头文件已创建
- [ ] 模块验证实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以验证模块签名和完整性
- [ ] 可以检查模块依赖和兼容性
- [ ] 可以进行安全扫描

## Git 提交

```bash
git add libs/idcu-module-verifier/
git commit -m "feat: add idcu-module-verifier library

- Add module signature verification
- Add module integrity check
- Add module dependency verification
- Add module compatibility check
- Add module security scanning
- Add module performance verification
- Add module test verification
- Add API compatibility check
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 签名验证失败 | 密钥不匹配 | 检查公钥配置 |
| 完整性检查失败 | 文件被篡改 | 重新获取模块文件 |
| 依赖检查失败 | 缺少依赖 | 安装所需依赖模块 |
