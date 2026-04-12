# 任务 3.21: idcu-permission - 权限管理库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的权限管理库，支持角色管理、权限管理、用户-角色关联、角色-权限关联、权限检查、继承权限、权限缓存、权限审计和持久化，满足权限检查 QPS ≥ 10,000 次/秒，权限缓存命中率 ≥ 90%。

### 1.2 不做什么
- 不实现基于策略的访问控制（PBAC）
- 不实现动态权限更新的实时推送
- 不实现权限报表和可视化
- 不实现权限的版本控制

### 1.3 输入
- 配置参数：缓存启用标志、审计启用标志
- 权限数据：角色定义、权限定义、主体定义、关联关系
- 权限检查请求：主体 ID、资源、动作、上下文

### 1.4 输出
- 角色/权限/主体 ID：创建成功返回唯一 ID
- 权限检查结果：IDCU_ERR_OK 表示允许，其他错误码表示拒绝
- 审计日志：权限检查记录
- 持久化数据：二进制文件或 JSON 格式导出

### 1.5 前置依赖
- idcu-common 基础库（提供向量、哈希表、锁等）
- idcu-storage 库（用于持久化）
- idcu-json 库（用于 JSON 导入导出）
- idcu-log 库（用于日志记录）
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **数据存储**: 内存中使用 Vector + HashMap，持久化使用 idcu-storage
- **权限缓存**: HashMap 缓存主体-权限映射，LRU 策略（可选）
- **角色继承**: DAG 结构，避免循环依赖
- **审计日志**: Vector 存储，支持导出和清理
- **线程安全**: 互斥锁（idcu_Mutex）保护所有操作

### 2.2 核心逻辑
```
1. 初始化：配置缓存和审计选项，初始化数据结构和锁
2. 角色管理：
   a. 添加/删除角色
   b. 查询角色（按 ID 或名称）
   c. 为角色分配/撤销权限
   d. 设置角色继承关系
3. 权限管理：
   a. 添加/删除权限
   b. 查询权限（按 ID 或名称）
   c. 设置权限描述和条件
4. 主体管理：
   a. 添加/删除主体（用户/服务等）
   b. 查询主体（按 ID 或名称）
   c. 为主体分配/撤销角色
   d. 为主体分配直接权限
5. 权限检查：
   a. 查询缓存，命中则直接返回
   b. 未命中则计算主体的所有权限（包括继承）
   c. 匹配资源和动作，应用条件判断
   d. 更新缓存并返回结果
6. 缓存管理：清除全部缓存、使主体/角色缓存失效
7. 审计管理：启用/禁用审计、获取审计日志、清除审计日志
8. 持久化：保存到文件、从文件加载、JSON 导入导出
9. 销毁：释放所有资源、销毁锁
```

### 2.3 数据结构/接口
```c
#ifndef IDCU_PERMISSION_PERMISSION_H
#define IDCU_PERMISSION_PERMISSION_H

#include "idcu/common/error_code.h"
#include "idcu/common/lock.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define IDCU_MAX_PERMISSIONS        256
#define IDCU_PERMISSION_NAME_MAX    128
#define IDCU_MAX_MODULE_PERMISSIONS 64

    typedef struct
    {
        char module_name[64];
        char permissions[IDCU_MAX_MODULE_PERMISSIONS][IDCU_PERMISSION_NAME_MAX];
        int  permission_count;
    } idcu_ModulePermission;

    typedef struct
    {
        idcu_ModulePermission module_perms[IDCU_MAX_MODULE_PERMISSIONS];
        int                   module_count;
        idcu_Mutex            lock;
        int                   initialized;
    } idcu_PermissionManager;

    int  idcu_permission_manager_init(void);
    void idcu_permission_manager_shutdown(void);

    int idcu_permission_check(const char* module_name, const char* permission);
    int idcu_permission_check_any(const char* module_name, const char** permissions, int count);
    int idcu_permission_check_all(const char* module_name, const char** permissions, int count);

    int idcu_permission_grant(const char* module_name, const char* permission);
    int idcu_permission_revoke(const char* module_name, const char* permission);

    int idcu_permission_get_module_permissions(const char* module_name, char** out_permissions,
                                               int* out_count);
    int idcu_permission_list_modules(char** out_modules, int* out_count);

#ifdef __cplusplus
}
#endif

#endif
```

### 2.4 跨平台适配
- **线程同步**: 使用 idcu-common 提供的跨平台互斥锁
- **字符串处理**: 标准 C 字符串函数，跨平台兼容
- **内存管理**: 标准 C 内存分配函数
- **无特殊平台差异**: 核心逻辑完全跨平台

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以为模块授权权限
- [ ] 可以检查模块是否有某个权限
- [ ] 支持通配符权限（如 "file.*"）
- [ ] 可以检查任一权限
- [ ] 可以检查所有权限
- [ ] 可以获取模块的所有权限
- [ ] 可以列出所有有权限的模块
- [ ] 线程安全，多线程环境下无数据竞争

### 3.2 性能验收
- 权限检查 QPS ≥ 10,000 次/秒
- 权限缓存命中率 ≥ 90%（启用缓存时）
- 单次权限检查时间 ≤ 0.1ms
- 内存占用 ≤ 5MB（默认配置）
- 授权和撤销操作时间 ≤ 1ms

### 3.3 异常验收
- [ ] 检查 NULL 参数安全处理
- [ ] 检查空字符串安全处理
- [ ] 权限数量超限安全处理
- [ ] 模块数量超限安全处理
- [ ] 内存不足时安全处理

---

## 4. 执行计划

### 4.1 工期
2.5 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和数据结构（30 分钟）
- D1-30: 完成基础权限检查和授权（45 分钟）
- D1-75: 完成通配符匹配和批量检查（30 分钟）
- D2-00: 完成单元测试和文档（45 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 数据结构）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，结构体前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释

### 5.2 测试要求
- 单元测试覆盖率 ≥ 85%
- 测试用例覆盖：授权、撤销、检查、通配符、批量操作、多线程
- 性能测试验证 QPS 和延迟指标

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::permission)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：通配符匹配性能不达标  
应对：优化匹配算法，使用前缀树等数据结构

### 6.2 风险2
描述：多线程锁竞争严重  
应对：使用读写锁替代互斥锁，减少锁粒度

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
mkdir -p libs/idcu-permission/include/idcu/permission
mkdir -p libs/idcu-permission/src/idcu/permission
mkdir -p libs/idcu-permission/tests
mkdir -p libs/idcu-permission/examples
```

### 步骤 2: 创建头文件 permission.h
定义权限管理器结构、模块权限结构、核心 API 函数。

### 步骤 3: 创建实现文件 permission.c
实现权限管理器初始化、授权、撤销、权限检查、通配符匹配等功能。

### 步骤 4: 创建 CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-permission VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-permission STATIC src/idcu/permission/permission.c)
target_include_directories(idcu-permission PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(idcu-permission PRIVATE idcu::common)
add_library(idcu::permission ALIAS idcu-permission)

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
  "name": "idcu-permission",
  "version": "1.0.0",
  "description": "Permission management library for IDCU Agent",
  "author": "IDCU Team",
  "license": "Apache-2.0",
  "dependencies": ["idcu-common"]
}
```

### 步骤 6: 创建 README.md
参考 libs/idcu-permission/README.md 现有内容。

---

## 8. 验证检查清单

- [ ] 头文件 permission.h 已创建
- [ ] 实现文件 permission.c 已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.json 已创建
- [ ] README.md 已创建
- [ ] 可以正常编译通过
- [ ] 单元测试通过率 100%
- [ ] 性能测试达标（QPS ≥ 10,000）
- [ ] 多线程测试通过
- [ ] 代码已通过 clang-format 格式化
- [ ] 代码已通过 clang-tidy 静态分析
- [ ] 符合工程化标准要求
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-permission/
git commit -m "feat: add idcu-permission library

- Add module-level permission management
- Add wildcard permission support (file.*)
- Add permission check, grant, and revoke
- Add batch permission checks (any/all)
- Add thread-safe implementation
- Add CMake build configuration
- Add unit tests and examples"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 权限检查失败 | 权限名称不匹配 | 检查权限名称是否完全一致或通配符匹配 |
| 通配符不工作 | 通配符语法错误 | 确保使用正确的通配符语法（*） |
| 多线程崩溃 | 锁使用错误 | 检查锁的获取和释放，确保线程安全 |
| 性能不达标 | 通配符匹配算法慢 | 优化匹配算法，考虑使用前缀树 |
| 内存泄漏 | 未正确释放资源 | 确保调用 shutdown 函数 |
| 权限丢失 | 未持久化 | 实现持久化功能或定期备份权限数据 |
