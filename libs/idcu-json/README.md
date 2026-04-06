# idcu-json

IDCU 项目的独立 JSON 解析库，提供简单高效的 JSON 解析和序列化功能。

## 特性

- 跨平台支持（Windows、Linux、macOS）
- 支持完整的 JSON 数据类型
- 简单易用的 API
- 支持从字符串解析和序列化为字符串

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-common REQUIRED)
find_package(idcu-json REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::json)
```

## API 文档

### 解析 JSON

```c
#include <idcu/json/json.h>

const char* json_str = "{\"name\": \"test\", \"value\": 42}";
idcu_JsonValue result;
int ret = idcu_json_parse(json_str, &result);
if (ret == IDCU_ERR_SUCCESS) {
    // 使用解析结果
    idcu_json_free(&result);
}
```

### 访问数据

```c
// 获取类型
idcu_JsonType type = idcu_json_get_type(&value);

// 获取布尔值
int bool_val;
idcu_json_get_bool(&value, &bool_val);

// 获取整数
int64_t int_val;
idcu_json_get_int(&value, &int_val);

// 获取浮点数
double double_val;
idcu_json_get_double(&value, &double_val);

// 获取字符串
const char* str_val;
idcu_json_get_string(&value, &str_val);

// 访问数组
size_t size = idcu_json_array_size(array);
idcu_JsonValue* elem = idcu_json_array_get(array, index);

// 访问对象
idcu_JsonValue* val = idcu_json_object_get(object, "key");
int has = idcu_json_object_has(object, "key");
```

### 序列化 JSON

```c
char buffer[1024];
idcu_json_to_string(&value, buffer, sizeof(buffer));

// 或者分配内存
char* str = idcu_json_to_string_alloc(&value);
if (str) {
    free(str);
}
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [简单解析示例](examples/example_parse.c) - 演示 JSON 解析的基本使用
- [序列化示例](examples/example_serialize.c) - 演示如何序列化为字符串

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

./example_parse
./example_serialize
```

## API 文档

详细的 API 文档请参考：[idcu-json API 文档](../../docs/api/idcu-json.md)

## 依赖

- idcu-common - IDCU 基础通用组件库

## 许可证

详见项目根目录的 LICENSE 文件。
