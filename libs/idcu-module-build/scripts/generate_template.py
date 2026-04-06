#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
IDCU Module Build - Project Template Generator
一键项目模板生成脚本
"""

import os
import sys
import argparse
import datetime
import json


def get_script_dir():
    return os.path.dirname(os.path.abspath(__file__))


def get_templates_dir():
    return os.path.join(os.path.dirname(get_script_dir()), "templates")


def ensure_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)


def generate_idcu_agent_module(name, output_dir):
    print(f"Generating IDCU Agent module: {name}")
    
    module_dir = os.path.join(output_dir, name)
    ensure_dir(module_dir)
    
    include_dir = os.path.join(module_dir, "include", name.replace("-", "_"))
    ensure_dir(include_dir)
    
    src_dir = os.path.join(module_dir, "src")
    ensure_dir(src_dir)
    
    module_name_underscore = name.replace("-", "_")
    
    module_json = {
        "module": {
            "name": name,
            "version": "1.0.0",
            "type": "shared",
            "description": f"{name} module for IDCU Agent",
            "author": "Your Name",
            "license": "MIT",
            "auto_adapt": True
        },
        "build": {
            "c_standard": "auto",
            "src": "auto",
            "type": "shared",
            "includes": ["include"],
            "defines": [],
            "deps": ["idcu-common"]
        },
        "project": {
            "idcu_agent": {}
        }
    }
    
    with open(os.path.join(module_dir, "module.json"), "w", encoding="utf-8") as f:
        json.dump(module_json, f, indent=2, ensure_ascii=False)
    
    cmake_content = f"""cmake_minimum_required(VERSION 3.10)
project({name})

find_package(idcu-module-build REQUIRED)
idcu_build_module()
"""
    with open(os.path.join(module_dir, "CMakeLists.txt"), "w", encoding="utf-8") as f:
        f.write(cmake_content)
    
    header_content = f"""#ifndef {module_name_underscore.upper()}_H
#define {module_name_underscore.upper()}_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {{
#endif

int {module_name_underscore}_init(void);
int {module_name_underscore}_run(void);
int {module_name_underscore}_stop(void);

#ifdef __cplusplus
}}
#endif

#endif
"""
    with open(os.path.join(include_dir, f"{module_name_underscore}.h"), "w", encoding="utf-8") as f:
        f.write(header_content)
    
    src_content = f'''#include "{module_name_underscore}/{module_name_underscore}.h"
#include "module_def.h"
#include "idcu/log/log.h"

static int g_module_enabled = 1;

int {module_name_underscore}_init(void)
{{
    IDCU_LOG_INFO("{name} initialized");
    return IDCU_ERR_SUCCESS;
}}

int {module_name_underscore}_run(void)
{{
    if (!g_module_enabled) {{
        return IDCU_ERR_SUCCESS;
    }}
    return IDCU_ERR_SUCCESS;
}}

int {module_name_underscore}_stop(void)
{{
    IDCU_LOG_INFO("{name} stopped");
    return IDCU_ERR_SUCCESS;
}}

IDCU_REGISTER_MODULE({module_name_underscore}, IDCU_MODULE_VERSION(1, 0, 0), 
                     {module_name_underscore}_init, {module_name_underscore}_run, {module_name_underscore}_stop);
'''
    with open(os.path.join(src_dir, f"{module_name_underscore}.c"), "w", encoding="utf-8") as f:
        f.write(src_content)
    
    readme_content = f"""# {name}

IDCU Agent 模块

## 构建

```bash
cd /path/to/idcu-module-build
python scripts/build.py release --module {name}
```
"""
    with open(os.path.join(module_dir, "README.md"), "w", encoding="utf-8") as f:
        f.write(readme_content)
    
    print(f"✅ IDCU Agent module created at: {module_dir}")


def generate_library(name, output_dir):
    print(f"Generating C library: {name}")
    
    module_dir = os.path.join(output_dir, name)
    ensure_dir(module_dir)
    
    include_dir = os.path.join(module_dir, "include", name.replace("-", "_"))
    ensure_dir(include_dir)
    
    src_dir = os.path.join(module_dir, "src")
    ensure_dir(src_dir)
    
    module_name_underscore = name.replace("-", "_")
    
    module_json = {
        "module": {
            "name": name,
            "version": "1.0.0",
            "type": "shared",
            "description": f"{name} C library",
            "author": "Your Name",
            "license": "MIT",
            "auto_adapt": True
        },
        "build": {
            "c_standard": "auto",
            "src": "auto",
            "type": "shared",
            "includes": ["include"],
            "defines": [],
            "deps": []
        }
    }
    
    with open(os.path.join(module_dir, "module.json"), "w", encoding="utf-8") as f:
        json.dump(module_json, f, indent=2, ensure_ascii=False)
    
    cmake_content = f"""cmake_minimum_required(VERSION 3.10)
project({name})

find_package(idcu-module-build REQUIRED)
idcu_build_module()
"""
    with open(os.path.join(module_dir, "CMakeLists.txt"), "w", encoding="utf-8") as f:
        f.write(cmake_content)
    
    header_content = f"""#ifndef {module_name_underscore.upper()}_H
#define {module_name_underscore.upper()}_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {{
#endif

int {module_name_underscore}_add(int a, int b);
int {module_name_underscore}_subtract(int a, int b);

#ifdef __cplusplus
}}
#endif

#endif
"""
    with open(os.path.join(include_dir, f"{module_name_underscore}.h"), "w", encoding="utf-8") as f:
        f.write(header_content)
    
    src_content = f'''#include "{module_name_underscore}/{module_name_underscore}.h"

int {module_name_underscore}_add(int a, int b)
{{
    return a + b;
}}

int {module_name_underscore}_subtract(int a, int b)
{{
    return a - b;
}}
'''
    with open(os.path.join(src_dir, f"{module_name_underscore}.c"), "w", encoding="utf-8") as f:
        f.write(src_content)
    
    readme_content = f"""# {name}

C 语言库

## 构建

```bash
cd /path/to/idcu-module-build
python scripts/build.py release --module {name}
```

## 使用

```c
#include <{module_name_underscore}/{module_name_underscore}.h>

int main() {{
    int result = {module_name_underscore}_add(2, 3);
    return 0;
}}
```
"""
    with open(os.path.join(module_dir, "README.md"), "w", encoding="utf-8") as f:
        f.write(readme_content)
    
    print(f"✅ C library created at: {module_dir}")


def generate_custom_project(name, output_dir):
    print(f"Generating custom project: {name}")
    
    module_dir = os.path.join(output_dir, name)
    ensure_dir(module_dir)
    
    include_dir = os.path.join(module_dir, "include", name.replace("-", "_"))
    ensure_dir(include_dir)
    
    src_dir = os.path.join(module_dir, "src")
    ensure_dir(src_dir)
    
    tests_dir = os.path.join(module_dir, "tests")
    ensure_dir(tests_dir)
    
    module_name_underscore = name.replace("-", "_")
    
    module_json = {
        "module": {
            "name": name,
            "version": "1.0.0",
            "type": "both",
            "description": f"{name} custom project",
            "author": "Your Name",
            "license": "MIT",
            "auto_adapt": False
        },
        "build": {
            "c_standard": "11",
            "src": ["src/*.c"],
            "type": "both",
            "includes": ["include"],
            "defines": [],
            "deps": [],
            "link_libraries": ["m"],
            "platform_deps": {
                "win32": ["ws2_32"],
                "unix": ["pthread"]
            }
        },
        "plugins": {
            "package": True,
            "coverage": True
        }
    }
    
    with open(os.path.join(module_dir, "module.json"), "w", encoding="utf-8") as f:
        json.dump(module_json, f, indent=2, ensure_ascii=False)
    
    cmake_content = f"""cmake_minimum_required(VERSION 3.10)
project({name})

find_package(idcu-module-build REQUIRED)
idcu_build_module()

enable_testing()
add_executable(test_{name} tests/test_{module_name_underscore}.c)
target_link_libraries(test_{name} {name})
add_test(NAME {name}_test COMMAND test_{name})
"""
    with open(os.path.join(module_dir, "CMakeLists.txt"), "w", encoding="utf-8") as f:
        f.write(cmake_content)
    
    header_content = f"""#ifndef {module_name_underscore.upper()}_H
#define {module_name_underscore.upper()}_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {{
#endif

typedef struct {{
    char name[64];
    int32_t value;
}} {module_name_underscore}_data_t;

int {module_name_underscore}_init(void);
int {module_name_underscore}_process({module_name_underscore}_data_t* data);
int {module_name_underscore}_cleanup(void);

#ifdef __cplusplus
}}
#endif

#endif
"""
    with open(os.path.join(include_dir, f"{module_name_underscore}.h"), "w", encoding="utf-8") as f:
        f.write(header_content)
    
    src_content = f'''#include "{module_name_underscore}/{module_name_underscore}.h"
#include <stdio.h>
#include <string.h>

static int g_initialized = 0;

int {module_name_underscore}_init(void)
{{
    if (g_initialized) {{
        return 0;
    }}
    g_initialized = 1;
    printf("[{name}] Initialized\\n");
    return 0;
}}

int {module_name_underscore}_process({module_name_underscore}_data_t* data)
{{
    if (!g_initialized || !data) {{
        return -1;
    }}
    printf("[{name}] Processing: %s = %d\\n", data->name, data->value);
    return 0;
}}

int {module_name_underscore}_cleanup(void)
{{
    if (!g_initialized) {{
        return 0;
    }}
    g_initialized = 0;
    printf("[{name}] Cleaned up\\n");
    return 0;
}}
'''
    with open(os.path.join(src_dir, f"{module_name_underscore}.c"), "w", encoding="utf-8") as f:
        f.write(src_content)
    
    test_content = f'''#include "{module_name_underscore}/{module_name_underscore}.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void)
{{
    printf("Testing {name}...\\n");
    
    int ret = {module_name_underscore}_init();
    assert(ret == 0);
    printf("Test 1 passed: init\\n");
    
    {module_name_underscore}_data_t data;
    strncpy(data.name, "test", sizeof(data.name));
    data.value = 42;
    
    ret = {module_name_underscore}_process(&data);
    assert(ret == 0);
    printf("Test 2 passed: process\\n");
    
    ret = {module_name_underscore}_cleanup();
    assert(ret == 0);
    printf("Test 3 passed: cleanup\\n");
    
    printf("All tests passed!\\n");
    return 0;
}}
'''
    with open(os.path.join(tests_dir, f"test_{module_name_underscore}.c"), "w", encoding="utf-8") as f:
        f.write(test_content)
    
    readme_content = f"""# {name}

自定义项目

## 构建

```bash
cd /path/to/idcu-module-build
python scripts/build.py release --module {name}
```

## 运行测试

```bash
cd build
ctest --output-on-failure
```
"""
    with open(os.path.join(module_dir, "README.md"), "w", encoding="utf-8") as f:
        f.write(readme_content)
    
    print(f"✅ Custom project created at: {module_dir}")


def main():
    parser = argparse.ArgumentParser(
        description="IDCU Module Build - Project Template Generator"
    )
    parser.add_argument(
        "--type",
        required=True,
        choices=["idcu-agent", "library", "custom"],
        help="Project type: idcu-agent, library, or custom"
    )
    parser.add_argument(
        "--name",
        required=True,
        help="Project name (use hyphens for multi-word names)"
    )
    parser.add_argument(
        "--output",
        default=".",
        help="Output directory (default: current directory)"
    )
    
    args = parser.parse_args()
    
    print("=" * 50)
    print("IDCU Module Build - Template Generator")
    print("=" * 50)
    
    if args.type == "idcu-agent":
        generate_idcu_agent_module(args.name, args.output)
    elif args.type == "library":
        generate_library(args.name, args.output)
    elif args.type == "custom":
        generate_custom_project(args.name, args.output)
    
    print("\n🎉 Done! Your project is ready.")


if __name__ == "__main__":
    main()
