# 任务 3.22: idcu-plugin - 插件系统库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的插件系统库，支持动态加载插件、插件生命周期管理、插件依赖管理、插件通信、插件沙箱、插件配置、插件版本管理和消息传递，满足插件加载耗时 ≤ 100ms、支持同时加载 100+ 个插件、支持插件热重载的性能要求。

### 1.2 不做什么
- 不实现跨平台的通用插件格式（仅支持 .so/.dll/.dylib）
- 不实现插件签名验证（由 idcu-module-verifier 负责）
- 不实现分布式插件管理
- 不实现插件市场和在线更新

### 1.3 输入
- 插件文件路径（支持 .so/.dll/.dylib）
- 插件配置（YAML 格式）
- 插件操作命令（加载、卸载、启动、停止、暂停、恢复）
- 插件消息数据

### 1.4 输出
- 插件 ID（唯一标识）
- 插件状态（已加载/已初始化/运行中/已暂停/错误）
- 返回码：0 表示成功，非 0 表示错误
- 插件统计信息

### 1.5 前置依赖
- idcu-common 库已实现
- idcu-yaml 库已实现
- idcu-log 库已实现
- idcu-sandbox 库已实现（可选）
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **动态加载**: Linux 使用 dlopen，Windows 使用 LoadLibrary，macOS 使用 dlopen
- **插件生命周期**: 6 状态机（UNLOADED → LOADED → INITIALIZED → RUNNING ↔ PAUSED）
- **数据结构**: Vector 存储插件列表，HashMap 快速查找
- **线程安全**: 互斥锁保护插件管理器状态
- **消息传递**: 消息队列 + 回调机制

### 2.2 核心逻辑
```
插件加载流程：
1. 检查插件文件是否存在
2. 加载动态库
3. 查找插件入口符号
4. 读取插件信息（名称、版本、作者等）
5. 检查依赖关系
6. 初始化插件数据结构
7. 加入插件列表
8. 返回插件 ID

插件生命周期：
UNLOADED → LOADED → INITIALIZED → RUNNING
                     ↓________________↑
                     ↓
                  PAUSED
                     ↓
                  ERROR

消息传递：
1. 发送方构造消息
2. 通过插件管理器路由
3. 调用目标插件的消息处理器
4. 支持广播和点对点
```

### 2.3 数据结构/接口
```c
#ifndef IDCU_PLUGIN_PLUGIN_H
#define IDCU_PLUGIN_PLUGIN_H

#include "idcu/common/error_code.h"

#include <stddef.h>
#include <stdint.h>

#define IDCU_PLUGIN_NAME_MAX    128
#define IDCU_PLUGIN_VERSION_MAX 32
#define IDCU_PLUGIN_PATH_MAX    512

typedef struct idcu_PluginInfo
{
    char     name[IDCU_PLUGIN_NAME_MAX];
    char     version[IDCU_PLUGIN_VERSION_MAX];
    char     author[128];
    char     description[512];
    int      api_version;
    uint32_t flags;
} idcu_PluginInfo;

typedef struct idcu_PluginHandle idcu_PluginHandle;

typedef int (*idcu_PluginInitFunc)(void);
typedef void (*idcu_PluginCleanupFunc)(void);
typedef int (*idcu_PluginGetInfoFunc)(idcu_PluginInfo* info);

int  idcu_plugin_system_init(void);
void idcu_plugin_system_shutdown(void);

int idcu_plugin_load(const char* path, idcu_PluginHandle** handle);
int idcu_plugin_unload(idcu_PluginHandle* handle);

int idcu_plugin_get_info(idcu_PluginHandle* handle, idcu_PluginInfo* info);
int idcu_plugin_get_name(idcu_PluginHandle* handle, char* buffer, size_t buffer_size);
int idcu_plugin_is_loaded(const char* name);

int idcu_plugin_scan_directory(const char* directory);
int idcu_plugin_load_all(void);
int idcu_plugin_unload_all(void);

int idcu_plugin_get_count(void);
int idcu_plugin_get_all_names(char** names, size_t max_names, size_t* actual_count);

int idcu_plugin_get_symbol(idcu_PluginHandle* handle, const char* symbol_name, void** symbol);

#endif  // IDCU_PLUGIN_PLUGIN_H
```

### 2.4 跨平台适配
- **动态加载**: Linux/macOS 使用 dlopen/dlsym/dlclose，Windows 使用 LoadLibrary/GetProcAddress/FreeLibrary
- **路径分隔符**: 使用 / 作为统一分隔符，在 Windows 上自动转换
- **文件扩展名**: Linux/macOS 使用 .so/.dylib，Windows 使用 .dll
- **错误处理**: 统一使用 errno 和 GetLastError 转换为内部错误码

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以成功加载 .so/.dll/.dylib 插件文件
- [ ] 可以获取插件信息（名称、版本、作者、描述）
- [ ] 可以获取插件导出的符号
- [ ] 可以扫描目录并加载多个插件
- [ ] 可以正确卸载插件
- [ ] 插件初始化和清理函数正常工作
- [ ] 支持插件的批量操作（load_all/unload_all）

### 3.2 性能验收
- 单个插件加载时间 ≤ 100ms
- 支持同时加载 100+ 个插件
- 插件符号查找时间 ≤ 1ms
- 插件枚举时间 ≤ 10ms（100个插件）
- 内存占用 ≤ 1MB/插件（不含插件自身代码）

### 3.3 异常验收
- [ ] 加载不存在的文件返回明确错误码
- [ ] 加载损坏的插件文件安全处理
- [ ] 插件依赖缺失时返回错误
- [ ] 重复加载同一插件返回错误或正常处理
- [ ] 卸载未加载的插件返回错误
- [ ] 内存不足时安全处理

---

## 4. 执行计划

### 4.1 工期
3 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和基础结构（30 分钟）
- D1-30: 完成跨平台动态加载实现（1 小时）
- D1-90: 完成插件信息查询和符号解析（30 分钟）
- D2-00: 完成批量操作和插件枚举（30 分钟）
- D2-30: 完成单元测试和文档（30 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 跨平台开发）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，结构体前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试用例覆盖：加载、卸载、信息查询、符号解析、批量操作
- 跨平台测试（Windows + Linux + macOS）

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::plugin)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：跨平台动态加载 API 差异导致实现复杂  
应对：抽象平台层，使用条件编译隔离平台差异

### 6.2 风险2
描述：插件之间的依赖关系管理复杂  
应对：当前阶段简化为独立插件，后续可扩展依赖管理

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
mkdir -p libs/idcu-plugin/include/idcu/plugin
mkdir -p libs/idcu-plugin/src/idcu/plugin
mkdir -p libs/idcu-plugin/tests
mkdir -p libs/idcu-plugin/examples
```

### 步骤 2: 创建头文件 plugin.h
定义插件信息结构体、插件句柄、核心 API 函数。

### 步骤 3: 创建实现文件 plugin.c
实现跨平台动态加载、插件信息查询、符号解析、批量操作等功能。

### 步骤 4: 创建 CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-plugin VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-plugin STATIC src/idcu/plugin/plugin.c)
target_include_directories(idcu-plugin PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(idcu-plugin PRIVATE idcu::common)
add_library(idcu::plugin ALIAS idcu-plugin)

if(WIN32)
    target_link_libraries(idcu-plugin PRIVATE psapi)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 步骤 5: 创建 module.json
```json
{
  "name": "idcu-plugin",
  "version": "1.0.0",
  "description": "Plugin system library for IDCU Agent",
  "author": "IDCU Team",
  "license": "Apache-2.0",
  "dependencies": ["idcu-common"]
}
```

### 步骤 6: 创建 README.md
参考 libs/idcu-plugin/README.md 现有内容。

---

## 8. 验证检查清单

- [ ] 头文件 plugin.h 已创建
- [ ] 实现文件 plugin.c 已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.json 已创建
- [ ] README.md 已创建
- [ ] 可以正常编译通过
- [ ] 单元测试通过率 100%
- [ ] 性能测试达标（加载时间 ≤ 100ms）
- [ ] 跨平台测试通过（Windows + Linux + macOS）
- [ ] 代码已通过 clang-format 格式化
- [ ] 代码已通过 clang-tidy 静态分析
- [ ] 符合工程化标准要求
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-plugin/
git commit -m "feat: add idcu-plugin library

- Add cross-platform dynamic plugin loading
- Add plugin information query
- Add plugin symbol resolution
- Add plugin directory scanning
- Add batch plugin operations
- Add CMake build configuration
- Add unit tests"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 插件加载失败 | 文件路径错误或权限不足 | 检查文件路径和权限，使用绝对路径 |
| 符号查找失败 | 符号名称错误或插件未导出 | 检查符号名称和插件的导出声明 |
| 跨平台不工作 | 平台特定代码有问题 | 检查条件编译和平台 API 使用 |
| 内存泄漏 | 插件未正确清理 | 确保调用 cleanup 函数和正确卸载 |
| 性能不达标 | 插件加载过程有冗余操作 | 优化加载流程，缓存必要信息 |
