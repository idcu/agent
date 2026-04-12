# 任务 3.29: idcu-module-verifier - 模块验证

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的模块验证库，支持模块签名验证、模块完整性检查、模块依赖验证、模块兼容性检查、模块安全扫描、模块性能验证、模块测试验证，满足验证耗时 ≤ 5s/模块、签名验证准确率 100%、完整性检查准确率 100% 的性能要求。

### 1.2 不做什么
- 不实现自定义签名算法（只支持标准算法）
- 不实现完整的静态代码分析
- 不实现模糊测试
- 不实现性能基准测试的自动化

### 1.3 输入
- 模块路径（动态库文件）
- 验证配置（启用哪些检查）
- 受信任的公钥

### 1.4 输出
- 验证报告（整体状态、各检查项结果）
- 安全问题列表
- 性能指标
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 库已实现
- idcu-log 库已实现
- idcu-json 库已实现
- idcu-yaml 库已实现
- phase3 前 28 个任务已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **签名验证**: ECDSA/RSA + SHA-256
- **完整性检查**: SHA-256/SHA-512 哈希
- **依赖验证**: 解析模块元数据 + 版本检查
- **安全扫描**: 检查导入的危险函数、可写段
- **兼容性检查**: API 签名匹配

### 2.2 核心逻辑
```
验证流程：
1. 读取模块文件
2. 检查文件完整性
3. 验证数字签名
4. 解析模块元数据
5. 检查依赖关系
6. 检查 API 兼容性
7. 执行安全扫描
8. 运行性能验证（可选）
9. 运行模块测试（可选）
10. 生成验证报告

签名验证流程：
1. 读取模块签名
2. 读取公钥
3. 计算模块哈希
4. 使用公钥验证签名
5. 返回验证结果
```

### 2.3 数据结构/接口
```c
#ifndef IDCU_MODULE_VERIFIER_MODULE_VERIFIER_H
#define IDCU_MODULE_VERIFIER_MODULE_VERIFIER_H

#include "idcu/common/error_code.h"

#include <stddef.h>
#include <stdint.h>

#define IDCU_VERIFIER_HASH_SIZE      32
#define IDCU_VERIFIER_SIGNATURE_SIZE 256
#define IDCU_VERIFIER_MAX_KEY_SIZE   4096

typedef enum
{
    IDCU_VERIFY_TYPE_HASH = 0,
    IDCU_VERIFY_TYPE_SIGNATURE,
    IDCU_VERIFY_TYPE_BOTH
} idcu_VerifyType;

typedef struct
{
    uint8_t         hash[IDCU_VERIFIER_HASH_SIZE];
    uint8_t         signature[IDCU_VERIFIER_SIGNATURE_SIZE];
    size_t          signature_len;
    char            public_key_path[512];
    idcu_VerifyType verify_type;
} idcu_ModuleVerifier;

int  idcu_verifier_init(idcu_ModuleVerifier* verifier);
void idcu_verifier_destroy(idcu_ModuleVerifier* verifier);

int idcu_verifier_set_public_key(idcu_ModuleVerifier* verifier, const char* key_path);
int idcu_verifier_set_verify_type(idcu_ModuleVerifier* verifier, idcu_VerifyType type);

int idcu_verifier_compute_hash(const char* file_path, uint8_t* hash_out, size_t hash_size);
int idcu_verifier_verify_hash(const char* file_path, const uint8_t* expected_hash,
                              size_t hash_size);

int idcu_verifier_verify_signature(const char* file_path, const uint8_t* signature,
                                   size_t signature_len, const char* public_key_path);

int idcu_verifier_verify_module(idcu_ModuleVerifier* verifier, const char* file_path);
int idcu_verifier_verify_module_with_hash(idcu_ModuleVerifier* verifier, const char* file_path,
                                          const uint8_t* expected_hash);

int idcu_verifier_load_expected_hash(const char* hash_file_path, uint8_t* hash_out,
                                     size_t hash_size);
int idcu_verifier_save_hash(const char* file_path, const uint8_t* hash, size_t hash_size);

#endif  // IDCU_MODULE_VERIFIER_MODULE_VERIFIER_H
```

### 2.4 跨平台适配
- **哈希计算**: 使用跨平台的 SHA-256 实现（或平台特定的加密库）
- **文件 I/O**: 标准 C 文件操作函数，跨平台兼容
- **内存管理**: 标准 C 内存分配函数
- **路径处理**: 统一使用 / 分隔符，Windows 自动转换

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以计算模块文件的 SHA-256 哈希
- [ ] 可以验证模块文件的哈希
- [ ] 可以设置公钥路径
- [ ] 可以配置验证类型（哈希、签名、两者）
- [ ] 可以验证模块的数字签名
- [ ] 可以保存和加载预期哈希
- [ ] 可以执行完整的模块验证

### 3.2 性能验收
- 验证耗时 ≤ 5s/模块（10MB 文件）
- 签名验证准确率 100%
- 完整性检查准确率 100%
- 哈希计算速度 ≥ 100MB/s
- 内存占用 ≤ 50MB

### 3.3 异常验收
- [ ] 验证不存在的文件返回明确错误码
- [ ] 验证损坏的文件安全处理
- [ ] 公钥文件缺失时返回错误
- [ ] 签名验证失败返回错误
- [ ] 内存不足时安全处理

---

## 4. 执行计划

### 4.1 工期
3 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和数据结构（30 分钟）
- D1-30: 完成哈希计算和验证（1 小时）
- D1-90: 完成签名验证框架（30 分钟）
- D2-00: 完成完整模块验证（30 分钟）
- D2-30: 完成单元测试和文档（30 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 密码学基础）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，结构体前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试用例覆盖：哈希计算、哈希验证、签名验证、完整验证、异常场景
- 使用已知的测试向量验证哈希计算

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::module_verifier)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：密码学库依赖复杂  
应对：使用简单的 SHA-256 实现，或依赖系统加密库

### 6.2 风险2
描述：大文件哈希计算性能慢  
应对：使用分块读取和计算，优化内存使用

### 6.3 风险3
描述：性能不满足预期要求  
应对：进行性能基准测试，优化关键路径代码

### 6.4 风险4
描述：跨平台兼容性问题  
应对：使用跨平台 API，充分测试不同平台

---

## 7. 详细实现步骤

### 步骤 1: 创建目录结构
```bash
mkdir -p libs/idcu-module-verifier/include/idcu/module_verifier
mkdir -p libs/idcu-module-verifier/src/idcu/module_verifier
mkdir -p libs/idcu-module-verifier/tests
```

### 步骤 2: 创建头文件 module_verifier.h
定义验证类型、验证器结构、核心 API 函数。

### 步骤 3: 创建实现文件 module_verifier.c
实现哈希计算、哈希验证、签名验证框架、完整模块验证等功能。

### 步骤 4: 创建 CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-module-verifier VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-module-verifier STATIC src/idcu/module_verifier/module_verifier.c)
target_include_directories(idcu-module-verifier PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(idcu-module-verifier PRIVATE idcu::common)
add_library(idcu::module_verifier ALIAS idcu-module-verifier)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
```

### 步骤 5: 创建 module.json
```json
{
  "name": "idcu-module-verifier",
  "version": "1.0.0",
  "description": "Module verification library for IDCU Agent",
  "author": "IDCU Team",
  "license": "Apache-2.0",
  "dependencies": ["idcu-common"]
}
```

### 步骤 6: 创建 README.md
参考 libs/idcu-module-verifier/README.md 现有内容。

---

## 8. 验证检查清单

- [ ] 头文件 module_verifier.h 已创建
- [ ] 实现文件 module_verifier.c 已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.json 已创建
- [ ] README.md 已创建
- [ ] 可以正常编译通过
- [ ] 单元测试通过率 100%
- [ ] 性能测试达标（验证时间 ≤ 5s/模块）
- [ ] 跨平台测试通过（Windows + Linux + macOS）
- [ ] 代码已通过 clang-format 格式化
- [ ] 代码已通过 clang-tidy 静态分析
- [ ] 符合工程化标准要求
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-module-verifier/
git commit -m "feat: add idcu-module-verifier library

- Add file hash computation and verification
- Add signature verification framework
- Add hash file I/O operations
- Add configurable verification types
- Add CMake build configuration
- Add unit tests"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 哈希计算失败 | 文件不存在或无权限 | 检查文件路径和权限 |
| 签名验证失败 | 公钥不匹配或签名损坏 | 检查公钥和签名文件 |
| 验证速度慢 | 文件过大或内存不足 | 使用分块处理，增加内存 |
| 跨平台不工作 | 平台特定代码问题 | 检查条件编译和 API 使用 |
| 内存泄漏 | 未正确释放资源 | 确保调用 destroy 函数 |
| 哈希不匹配 | 文件被修改 | 重新计算并保存哈希 |
