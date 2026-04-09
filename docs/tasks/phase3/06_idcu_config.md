# 任务 3.6: idcu-config - 配置管理库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的配置管理库，支持 YAML 和 JSON 格式配置文件、热重载、多环境配置、类型安全访问、变更通知、文件监控自动重载，满足配置加载时间 ≤ 100ms、热重载延迟 ≤ 500ms、支持 1000+ 配置项的性能要求。

### 1.2 不做什么
- 不实现加密配置（由上层模块处理）
- 不实现远程配置中心（独立模块）
- 不实现配置版本控制
- 不实现复杂配置验证规则（仅基础类型）

### 1.3 输入
- 配置文件路径（YAML/JSON）
- 配置项访问路径（section + key）
- 配置值（string/int/bool/double/list）
- 环境变量前缀
- 监控间隔（毫秒）

### 1.4 输出
- 配置值（各类型）
- 配置变更回调通知
- 配置加载/保存结果（成功/失败）
- 配置统计信息（项数、最后修改时间）
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用
- idcu-json 库已可用
- idcu-yaml 库已可用
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **配置格式**: YAML（优先）和 JSON（备选）
- **数据存储**: 内存哈希表 + 链表，支持多节
- **线程安全**: 读写锁保护，支持多线程读取
- **文件监控**: Windows ReadDirectoryChangesW，Linux inotify
- **热重载**: 原子指针替换，避免锁争用

### 2.2 核心逻辑
```
初始化流程：
1. 解析文件路径和格式
2. 读取并解析配置文件
3. 构建内存数据结构
4. 初始化读写锁
5. 可选：启动文件监控线程

配置读取流程：
1. 获取读锁
2. 查找 section 和 key
3. 类型转换
4. 返回结果或默认值
5. 释放读锁

配置热重载流程：
1. 监控线程检测文件变更
2. 后台线程解析新配置
3. 获取写锁，原子替换配置指针
4. 触发所有变更回调
5. 释放写锁
```

### 2.3 数据结构/接口
```c
// 主要头文件：idcu/config/config.h

#define IDCU_CONFIG_MAX_SECTIONS         64
#define IDCU_CONFIG_MAX_KEYS_PER_SECTION 128
#define IDCU_CONFIG_KEY_MAX              128
#define IDCU_CONFIG_VALUE_MAX            512

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

// 配置管理器
typedef struct {
    idcu_ConfigSection sections[IDCU_CONFIG_MAX_SECTIONS];
    uint32_t           section_count;
    idcu_RWLock        lock;
    int                loaded;
    char               file_path[IDCU_CONFIG_PATH_MAX];
} idcu_ConfigManager;

// 核心 API
int   idcu_config_init(const char* file_path);
void  idcu_config_shutdown(void);
int   idcu_config_is_loaded(void);
int   idcu_config_reload(void);
int   idcu_config_save(const char* file_path);

// 类型安全访问
const char* idcu_config_get_string(const char* section, const char* key, const char* default_value);
int         idcu_config_get_int(const char* section, const char* key, int default_value);
int64_t     idcu_config_get_int64(const char* section, const char* key, int64_t default_value);
double      idcu_config_get_double(const char* section, const char* key, double default_value);
int         idcu_config_get_bool(const char* section, const char* key, int default_value);

// 配置写入
int idcu_config_set_string(const char* section, const char* key, const char* value);
int idcu_config_set_int(const char* section, const char* key, int value);
int idcu_config_set_int64(const char* section, const char* key, int64_t value);
int idcu_config_set_double(const char* section, const char* key, double value);
int idcu_config_set_bool(const char* section, const char* key, int value);

// 列表支持
typedef struct { char items[32][128]; int count; } idcu_ConfigList;
int idcu_config_get_list(const char* section, const char* key, const char* delimiter, idcu_ConfigList* out_list);

// 变更通知
typedef void (*idcu_ConfigChangeCallback)(const char* section, const char* key,
                                          const char* old_value, const char* new_value,
                                          void* user_data);
int  idcu_config_register_change_callback(idcu_ConfigChangeCallback callback, void* user_data);
int  idcu_config_unregister_change_callback(idcu_ConfigChangeCallback callback);

// 文件监控
int  idcu_config_watch_start(uint32_t interval_ms);
void idcu_config_watch_stop(void);
int  idcu_config_watch_is_running(void);

// 环境变量支持
void idcu_config_enable_env_var(int enable);
int  idcu_config_load_profile(const char* profile_name);
```

### 2.4 跨平台适配
- **Windows**: 使用 ReadDirectoryChangesW 监控文件
- **Linux**: 使用 inotify 监控文件
- **路径处理**: 统一处理 \ 和 /
- **读写锁**: 使用 idcu-common 中的跨平台实现

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以加载 YAML 和 JSON 格式配置文件
- [ ] 可以读取各种类型的配置值（string/int/bool/double/list）
- [ ] 可以写入并保存配置
- [ ] 配置热重载功能正常
- [ ] 文件监控自动重载正常工作
- [ ] 配置变更回调通知正确触发
- [ ] 多环境配置支持正常
- [ ] 环境变量覆盖配置正常
- [ ] 多线程并发读取安全（无崩溃，数据一致性）
- [ ] 类型安全检查正确（类型不匹配返回默认值）

### 3.2 性能验收
- 配置加载时间 ≤ 100ms（1000 项配置）
- 配置项读取延迟 ≤ 1μs（平均）
- 配置热重载延迟 ≤ 500ms（从文件变更到生效）
- 支持 1000+ 配置项
- 文件监控 CPU 占用 ≤ 1%（空闲时）
- 多线程（10 线程）读取 QPS ≥ 1,000,000

### 3.3 异常验收
- [ ] 配置文件不存在返回明确错误码
- [ ] 配置格式错误返回明确错误码
- [ ] 配置项不存在返回默认值
- [ ] 类型转换失败返回默认值
- [ ] 监控失败不影响配置读取
- [ ] 内存不足时返回错误而非崩溃

---

## 4. 执行计划

### 4.1 工期
5 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和数据结构（45 分钟）
- D1-45: 完成配置解析和基础读写功能（1.5 小时）
- D1-135: 完成热重载和文件监控（1 小时）
- D1-195: 完成变更通知和多环境支持（45 分钟）
- D1-240: 完成单元测试（30 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 文件操作 + 多线程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释
- 内部函数使用 static 修饰

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试覆盖 YAML 和 JSON 格式
- 测试覆盖各种类型和异常场景
- 性能测试验证加载和读取速度
- 跨平台测试（Windows + Linux）

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::config)`
- 依赖：idcu-common, idcu-json, idcu-yaml

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：热重载时锁争用导致性能下降  
应对：使用原子指针替换，最小化锁持有时间

### 6.2 风险2
描述：跨平台文件监控差异导致兼容性问题  
应对：提供降级方案（定时轮询），统一 API

---

## 7. 详细实现步骤

### 1. 创建目录结构
```bash
mkdir -p libs/idcu-config/include/idcu/config
mkdir -p libs/idcu-config/src/idcu/config
mkdir -p libs/idcu-config/tests
mkdir -p libs/idcu-config/examples
```

### 2. 创建配置头文件 (config.h)
创建 `libs/idcu-config/include/idcu/config/config.h`。

### 3. 创建 CMakeLists.txt
创建 `libs/idcu-config/CMakeLists.txt`。

### 4. 创建模块配置文件 (module.yaml)
创建 `libs/idcu-config/module.yaml`。

### 5. 创建实现文件
- config.c: 核心实现
- parser_yaml.c: YAML 解析
- parser_json.c: JSON 解析
- watcher.c: 文件监控
- callback.c: 变更通知

### 6. 创建示例配置文件
创建 `config/default/agent.yaml` 示例。

### 7. 创建 README.md
创建 `libs/idcu-config/README.md`。

---

## 8. 验证检查清单

- [ ] 配置头文件已创建
- [ ] 配置实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以加载 YAML 配置文件
- [ ] 可以加载 JSON 配置文件
- [ ] 可以读取各种类型的配置值
- [ ] 可以写入和保存配置
- [ ] 配置热重载功能正常
- [ ] 文件监控自动重载正常
- [ ] 配置变更通知正常
- [ ] 多环境配置支持正常
- [ ] 环境变量覆盖正常
- [ ] 多线程测试通过
- [ ] 跨平台测试通过（Windows + Linux）
- [ ] 性能测试达标
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-config/
git commit -m "feat: add idcu-config library

- Add YAML and JSON config format support
- Add type-safe config access (string/int/bool/double/list)
- Add config hot reload with atomic update
- Add file monitoring and auto-reload
- Add config change callback notifications
- Add multi-environment config support
- Add environment variable override support
- Add CMake build configuration
- Add module.yaml metadata
- Add comprehensive unit tests"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 配置加载失败 | 文件格式错误或路径不对 | 检查配置文件格式和路径 |
| 热重载不生效 | 未正确启动监控 | 确保调用了 idcu_config_watch_start() |
| 类型转换错误 | 配置值类型不匹配 | 确保使用正确的 get_* 函数 |
| 文件监控在 Linux 不工作 | inotify 限制 | 检查 /proc/sys/fs/inotify/max_user_watches |
| 多线程读取崩溃 | 未正确使用锁 | 确保读写锁正确初始化和使用 |
| 配置保存丢失 | 未调用 save | 修改配置后记得调用 idcu_config_save() |
