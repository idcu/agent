# 任务 3.2: idcu-json - JSON 解析与序列化库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的、高性能的 JSON 解析与序列化库，支持从字符串解析 JSON、序列化为字符串、所有 JSON 类型（null、bool、number、string、array、object）、类型安全的访问接口、自动内存管理，满足解析速度 ≥ 100MB/s、序列化速度 ≥ 150MB/s 的性能要求。

### 1.2 不做什么
- 不实现 JSON 流式解析
- 不实现 JSON Schema 验证
- 不实现 JSON 路径查询（如 JSONPath）
- 不实现远程 JSON 加载

### 1.3 输入
- JSON 字符串（UTF-8 编码）
- JSON 类型值（用于序列化）
- 输出缓冲区或文件路径

### 1.4 输出
- 解析后的 JSON 值树（idcu_JsonValue）
- 序列化后的 JSON 字符串
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用（提供错误码等）
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **解析器**: 手写递归下降解析器（高性能、无依赖）
- **数据结构**: 动态扩容的数组和对象
- **内存管理**: 手动内存管理，提供 free 函数释放整个 JSON 树
- **错误处理**: 使用 idcu-common 的错误码体系

### 2.2 核心逻辑
```
解析流程：
1. 初始化解析器（位置、行号、列号）
2. 跳过空白字符
3. 根据首字符判断类型（" 为字符串，-或数字为数字，t/f为布尔，n为null，[为数组，{为对象）
4. 递归解析对应类型
5. 返回解析结果

序列化流程：
1. 根据值类型选择序列化方式
2. 递归序列化子元素（数组、对象）
3. 写入到缓冲区
4. 返回序列化结果
```

### 2.3 数据结构/接口
```c
typedef enum {
    IDCU_JSON_TYPE_NULL = 0,
    IDCU_JSON_TYPE_BOOL,
    IDCU_JSON_TYPE_INT,
    IDCU_JSON_TYPE_DOUBLE,
    IDCU_JSON_TYPE_STRING,
    IDCU_JSON_TYPE_ARRAY,
    IDCU_JSON_TYPE_OBJECT
} idcu_JsonType;

typedef struct idcu_JsonValue  idcu_JsonValue;
typedef struct idcu_JsonArray  idcu_JsonArray;
typedef struct idcu_JsonObject idcu_JsonObject;

struct idcu_JsonArray {
    idcu_JsonValue* elements;
    size_t          count;
    size_t          capacity;
};

struct idcu_JsonObject {
    char**          keys;
    idcu_JsonValue* values;
    size_t          count;
    size_t          capacity;
};

struct idcu_JsonValue {
    idcu_JsonType type;
    union {
        int              bool_val;
        int64_t          int_val;
        double           double_val;
        char*            string_val;
        idcu_JsonArray*  array_val;
        idcu_JsonObject* object_val;
    } data;
};

// 核心 API
int  idcu_json_parse(const char* json_str, idcu_JsonValue* result);
void idcu_json_free(idcu_JsonValue* value);

// 类型访问
idcu_JsonType idcu_json_get_type(const idcu_JsonValue* value);
int idcu_json_get_bool(const idcu_JsonValue* value, int* out);
int idcu_json_get_int(const idcu_JsonValue* value, int64_t* out);
int idcu_json_get_double(const idcu_JsonValue* value, double* out);
int idcu_json_get_string(const idcu_JsonValue* value, const char** out);

// 数组操作
size_t          idcu_json_array_size(const idcu_JsonArray* array);
idcu_JsonValue* idcu_json_array_get(const idcu_JsonArray* array, size_t index);

// 对象操作
idcu_JsonValue* idcu_json_object_get(const idcu_JsonObject* object, const char* key);
int             idcu_json_object_has(const idcu_JsonObject* object, const char* key);

// 序列化
int   idcu_json_to_string(const idcu_JsonValue* value, char* buffer, size_t buffer_size);
char* idcu_json_to_string_alloc(const idcu_JsonValue* value);
```

### 2.4 跨平台适配
- 所有代码使用标准 C 库，无平台特定 API
- 使用 idcu-common 中的错误码，统一错误处理
- 文件路径使用标准 C 库函数处理

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以解析所有 JSON 类型（null、bool、int、double、string、array、object）
- [ ] 可以正确处理嵌套的 JSON 结构（数组嵌套对象、对象嵌套数组等）
- [ ] 可以正确处理转义字符（\n、\t、\r、\\、\"）
- [ ] 类型安全的访问函数可以正确检查类型并返回错误
- [ ] 可以序列化为紧凑的 JSON 字符串
- [ ] idcu_json_free 可以正确释放整个 JSON 树，无内存泄漏

### 3.2 性能验收
- 解析速度 ≥ 100MB/s（10MB JSON 文件解析时间 ≤ 100ms）
- 序列化速度 ≥ 150MB/s
- 内存占用：解析 1MB JSON 内存增量 ≤ 5MB
- 单次解析初始化时间 ≤ 1ms

### 3.3 异常验收
- [ ] 传入 NULL 参数时返回明确错误码，不崩溃
- [ ] 解析非法 JSON 时返回明确错误码和位置信息
- [ ] 缓冲区溢出时返回明确错误码
- [ ] 内存分配失败时返回明确错误码

---

## 4. 执行计划

### 4.1 工期
3 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和数据结构（30 分钟）
- D1-30: 完成解析器核心实现（1 小时）
- D1-90: 完成序列化实现（45 分钟）
- D1-135: 完成单元测试（45 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 编译器原理基础）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，结构体前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释

### 5.2 测试要求
- 单元测试覆盖率 ≥ 85%
- 测试用例覆盖：各种 JSON 类型、嵌套结构、转义字符、边界条件、异常场景
- 性能测试用例验证解析和序列化速度指标

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::json)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：JSON 解析器在处理大型 JSON 时性能不达标  
应对：优化解析器，减少不必要的内存拷贝，考虑使用更高效的字符串处理

### 6.2 风险2
描述：内存泄漏问题  
应对：使用 Valgrind 或 AddressSanitizer 进行内存泄漏检测，确保所有分配的内存都被正确释放

---

## 7. 详细实现步骤

### 步骤 1: 确认目录结构
```bash
# 目录结构已存在
libs/idcu-json/
├── include/idcu/json/
├── src/idcu/json/
├── tests/
├── examples/
├── CMakeLists.txt
├── README.md
└── module.json
```

### 步骤 2: 确认头文件 json.h
确认 libs/idcu-json/include/idcu/json/json.h 中的 API 定义完整。

### 步骤 3: 确认实现文件 json.c
确认 libs/idcu-json/src/idcu/json/json.c 中的实现完整。

### 步骤 4: 确认 CMakeLists.txt
确认 libs/idcu-json/CMakeLists.txt 配置正确，使用 idcu-module-build。

### 步骤 5: 确认 module.json
确认 libs/idcu-json/module.json 元数据完整。

### 步骤 6: 确认 README.md
确认 libs/idcu-json/README.md 文档完整。

---

## 8. 验证检查清单

- [ ] 头文件 json.h 已存在且 API 完整
- [ ] 实现文件 json.c 已存在且实现完整
- [ ] CMakeLists.txt 已存在且配置正确
- [ ] module.json 已存在
- [ ] README.md 已存在
- [ ] 可以正常编译通过
- [ ] 单元测试通过率 100%
- [ ] 性能测试达标（解析 ≥ 100MB/s，序列化 ≥ 150MB/s）
- [ ] 内存泄漏检测通过（Valgrind/AddressSanitizer）
- [ ] 跨平台测试通过（Windows + Linux）
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-json/
git commit -m "feat: add idcu-json library

- Add JSON parser from string
- Add JSON serializer to string
- Add support for all JSON types
- Add type-safe accessor functions
- Add automatic memory management
- Add CMake build configuration
- Add unit tests with 85%+ coverage"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 解析失败 | JSON 格式错误 | 验证 JSON 格式是否正确，检查错误位置 |
| 内存泄漏 | 忘记调用 idcu_json_free | 确保所有解析的 JSON 值都被释放 |
| 缓冲区溢出 | 序列化缓冲区太小 | 使用更大的缓冲区或 idcu_json_to_string_alloc |
| 类型访问错误 | 值类型不匹配 | 先调用 idcu_json_get_type 检查类型 |
| 性能不达标 | JSON 过大或解析器未优化 | 考虑分块处理或优化解析器实现 |
