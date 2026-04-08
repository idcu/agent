# 任务 3.3: idcu-yaml - YAML 解析与序列化库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始（注：当前项目使用 INI 格式配置，本库为可选扩展）

---

## 1. 任务边界

### 1.1 核心目标
创建简化版的 YAML 解析与序列化库，支持基本 YAML 语法（键值对、列表、嵌套结构）、与 idcu-json 类似的 API、类型安全的访问接口、自动内存管理，满足解析速度 ≥ 50MB/s 的性能要求。

### 1.2 不做什么
- 不实现完整的 YAML 1.2 规范（仅支持简化子集）
- 不实现 YAML 流式解析
- 不实现 YAML 标签和锚点
- 不实现多文档 YAML

### 1.3 输入
- YAML 字符串（UTF-8 编码）
- YAML 类型值（用于序列化）
- 输出缓冲区或文件路径

### 1.4 输出
- 解析后的 YAML 值树（idcu_YamlValue）
- 序列化后的 YAML 字符串
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用（提供错误码等）
- phase2 已完成
- 注：当前项目使用 idcu-config 的 INI 格式，本库为可选扩展

---

## 2. 技术实现方案

### 2.1 核心选型
- **解析器**: 手写递归下降解析器（简化版 YAML）
- **数据结构**: 与 idcu-json 类似的动态扩容序列和映射
- **内存管理**: 手动内存管理，提供 free 函数释放整个 YAML 树
- **错误处理**: 使用 idcu-common 的错误码体系

### 2.2 核心逻辑
```
解析流程：
1. 逐行扫描，处理缩进
2. 根据缩进级别确定嵌套关系
3. 解析键值对、列表项
4. 递归处理嵌套结构
5. 返回解析结果

支持的简化 YAML 语法：
- 键值对: key: value
- 列表: - item
- 嵌套: 使用缩进（2 或 4 空格）
- 注释: # comment
```

### 2.3 数据结构/接口
```c
typedef enum {
    IDCU_YAML_TYPE_NULL = 0,
    IDCU_YAML_TYPE_BOOL,
    IDCU_YAML_TYPE_INT,
    IDCU_YAML_TYPE_DOUBLE,
    IDCU_YAML_TYPE_STRING,
    IDCU_YAML_TYPE_SEQUENCE,
    IDCU_YAML_TYPE_MAPPING
} idcu_YamlType;

typedef struct idcu_YamlValue  idcu_YamlValue;
typedef struct idcu_YamlSequence idcu_YamlSequence;
typedef struct idcu_YamlMapping  idcu_YamlMapping;

struct idcu_YamlSequence {
    idcu_YamlValue* elements;
    size_t          count;
    size_t          capacity;
};

struct idcu_YamlMapping {
    char**          keys;
    idcu_YamlValue* values;
    size_t          count;
    size_t          capacity;
};

struct idcu_YamlValue {
    idcu_YamlType type;
    union {
        int               bool_val;
        int64_t           int_val;
        double            double_val;
        char*             string_val;
        idcu_YamlSequence* sequence_val;
        idcu_YamlMapping*  mapping_val;
    } data;
};

// 核心 API
int  idcu_yaml_parse(const char* yaml_str, idcu_YamlValue* result);
void idcu_yaml_free(idcu_YamlValue* value);

// 类型访问
idcu_YamlType idcu_yaml_get_type(const idcu_YamlValue* value);
int idcu_yaml_get_bool(const idcu_YamlValue* value, int* out);
int idcu_yaml_get_int(const idcu_YamlValue* value, int64_t* out);
int idcu_yaml_get_double(const idcu_YamlValue* value, double* out);
int idcu_yaml_get_string(const idcu_YamlValue* value, const char** out);

// 序列操作
size_t           idcu_yaml_sequence_size(const idcu_YamlSequence* seq);
idcu_YamlValue*  idcu_yaml_sequence_get(const idcu_YamlSequence* seq, size_t index);

// 映射操作
idcu_YamlValue*  idcu_yaml_mapping_get(const idcu_YamlMapping* map, const char* key);
int              idcu_yaml_mapping_has(const idcu_YamlMapping* map, const char* key);

// 序列化
int   idcu_yaml_to_string(const idcu_YamlValue* value, char* buffer, size_t buffer_size);
char* idcu_yaml_to_string_alloc(const idcu_YamlValue* value);

// 转换
int idcu_yaml_to_json(const idcu_YamlValue* yaml_value, char* buffer, size_t buffer_size);
```

### 2.4 跨平台适配
- 所有代码使用标准 C 库，无平台特定 API
- 使用 idcu-common 中的错误码，统一错误处理
- 换行处理：Windows 使用 \r\n，Linux 使用 \n

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以解析简化 YAML 语法（键值对、列表、嵌套结构）
- [ ] 可以正确处理缩进（2 或 4 空格）
- [ ] 可以正确处理注释（# 开头的行）
- [ ] 类型安全的访问函数可以正确检查类型并返回错误
- [ ] 可以序列化为 YAML 字符串
- [ ] idcu_yaml_free 可以正确释放整个 YAML 树，无内存泄漏
- [ ] 可以将 YAML 转换为 JSON 格式

### 3.2 性能验收
- 解析速度 ≥ 50MB/s
- 序列化速度 ≥ 80MB/s
- 内存占用：解析 1MB YAML 内存增量 ≤ 5MB
- 单次解析初始化时间 ≤ 1ms

### 3.3 异常验收
- [ ] 传入 NULL 参数时返回明确错误码，不崩溃
- [ ] 缩进不一致时返回明确错误码
- [ ] 解析非法 YAML 时返回明确错误码和位置信息
- [ ] 缓冲区溢出时返回明确错误码

---

## 4. 执行计划

### 4.1 工期
4 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和数据结构（30 分钟）
- D1-30: 完成解析器核心实现（1.5 小时）
- D1-120: 完成序列化和 JSON 转换实现（1 小时）
- D1-180: 完成单元测试（30 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 编译器原理基础）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，结构体前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试用例覆盖：各种 YAML 结构、缩进、注释、边界条件、异常场景
- 性能测试用例验证解析和序列化速度指标

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::yaml)`
- 注：当前项目使用 idcu-config 的 INI 格式，本库为可选扩展

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：YAML 缩进解析复杂，容易出错  
应对：仅支持简化的 YAML 语法，明确文档说明支持的语法子集

### 6.2 风险2
描述：与现有 idcu-config 的 INI 格式功能重叠  
应对：明确本库为可选扩展，不强制替换现有 INI 格式

---

## 7. 详细实现步骤

### 步骤 1: 创建目录结构
```bash
mkdir -p libs/idcu-yaml/include/idcu/yaml
mkdir -p libs/idcu-yaml/src/idcu/yaml
mkdir -p libs/idcu-yaml/tests
mkdir -p libs/idcu-yaml/examples
```

### 步骤 2: 创建头文件 yaml.h
定义 YAML 类型、数据结构和 API。

### 步骤 3: 创建实现文件 yaml.c
实现解析器、序列化、JSON 转换等功能。

### 步骤 4: 创建 CMakeLists.txt
使用 idcu-module-build 配置构建。

### 步骤 5: 创建 module.json
定义库的元数据。

### 步骤 6: 创建 README.md
编写库的使用文档。

---

## 8. 验证检查清单

- [ ] 头文件 yaml.h 已创建
- [ ] 实现文件 yaml.c 已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.json 已创建
- [ ] README.md 已创建
- [ ] 可以正常编译通过
- [ ] 单元测试通过率 100%
- [ ] 性能测试达标（解析 ≥ 50MB/s）
- [ ] 内存泄漏检测通过（Valgrind/AddressSanitizer）
- [ ] 跨平台测试通过（Windows + Linux）
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-yaml/
git commit -m "feat: add idcu-yaml library (simplified)

- Add simplified YAML parser
- Add YAML serializer
- Add support for key-value, list, nested structure
- Add type-safe accessor functions
- Add JSON conversion support
- Add CMake build configuration
- Add unit tests with 80%+ coverage"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 解析失败 | 使用了完整 YAML 特性 | 使用文档中说明的简化 YAML 语法 |
| 缩进错误 | 缩进不一致 | 确保使用统一的缩进（2 或 4 空格） |
| 类型转换错误 | 值类型不匹配 | 先调用 idcu_yaml_get_type 检查类型 |
| 与 idcu-config 冲突 | 功能重叠 | 明确选择使用 INI 或 YAML，不要混用 |
