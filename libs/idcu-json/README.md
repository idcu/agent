# idcu-json

JSON 解析和序列化库。

## 功能特性

- 从字符串解析 JSON
- 将 JSON 序列化为字符串
- 支持所有 JSON 类型
- 类型安全的访问函数
- 自动内存管理

## 使用方法

```c
#include <idcu/json/json.h>

int main() {
    idcu_JsonValue value;
    int ret = idcu_json_parse("{\"name\": \"test\", \"value\": 123}", &value);
    if (ret == IDCU_ERR_OK) {
        // 使用 JSON 值
        idcu_json_free(&value);
    }
    return 0;
}
```

## 构建

```bash
cmake -B build
cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
