# idcu-json

JSON parsing and serialization library.

## Features

- Parse JSON from string
- Serialize JSON to string
- Support all JSON types
- Type-safe accessor functions
- Automatic memory management

## Usage

```c
#include <idcu/json/json.h>

int main() {
    idcu_JsonValue value;
    int ret = idcu_json_parse("{\"name\": \"test\", \"value\": 123}", &value);
    if (ret == IDCU_ERR_OK) {
        // Use the JSON value
        idcu_json_free(&value);
    }
    return 0;
}
```

## Build

```bash
cmake -B build
cmake --build build
```
