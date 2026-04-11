# idcu-yaml

IDCU Agent 的简化 YAML 解析和序列化库。

## 功能特性

- 简化的 YAML 解析器
- YAML 序列化器
- 支持键值对、列表和嵌套结构
- 类型安全的访问函数
- JSON 转换支持
- 自动内存管理

## 支持的 YAML 语法

本库支持 YAML 的简化子集：

```yaml
# 键值对
key: value
number: 123
flag: true

# 列表
items:
  - item1
  - item2
  - item3

# 嵌套结构
config:
  host: localhost
  port: 8080
  settings:
    debug: true
    log_level: info
```

## 使用方法

```c
#include <idcu/yaml/yaml.h>

// 解析 YAML
idcu_YamlValue root;
int ret = idcu_yaml_parse("key: value\nnumber: 123", &root);
if (ret == IDCU_ERR_OK) {
    // 使用解析后的值
    idcu_yaml_free(&root);
}

// 序列化为 YAML
char buffer[1024];
idcu_yaml_to_string(&value, buffer, sizeof(buffer));

// 转换为 JSON
char json_buffer[1024];
idcu_yaml_to_json(&value, json_buffer, sizeof(json_buffer));
```

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
