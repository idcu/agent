# idcu-yaml

Simplified YAML parsing and serialization library for IDCU Agent.

## Features

- Simplified YAML parser
- YAML serializer
- Support for key-value pairs, lists, and nested structures
- Type-safe accessor functions
- JSON conversion support
- Automatic memory management

## Supported YAML Syntax

This library supports a simplified subset of YAML:

```yaml
# Key-value pairs
key: value
number: 123
flag: true

# Lists
items:
  - item1
  - item2
  - item3

# Nested structures
config:
  host: localhost
  port: 8080
  settings:
    debug: true
    log_level: info
```

## Usage

```c
#include <idcu/yaml/yaml.h>

// Parse YAML
idcu_YamlValue root;
int ret = idcu_yaml_parse("key: value\nnumber: 123", &root);
if (ret == IDCU_ERR_OK) {
    // Use the parsed value
    idcu_yaml_free(&root);
}

// Serialize to YAML
char buffer[1024];
idcu_yaml_to_string(&value, buffer, sizeof(buffer));

// Convert to JSON
char json_buffer[1024];
idcu_yaml_to_json(&value, json_buffer, sizeof(json_buffer));
```

## Build

```bash
cmake -B build && cmake --build build
```

## License

MIT
