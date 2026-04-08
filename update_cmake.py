#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
批量更新库的 CMakeLists.txt 文件
"""

import os
import json

# 定义需要更新的库目录和它们的依赖关系
LIB_DEPENDENCIES = {
    "idcu-cache": ["idcu::common"],
    "idcu-coroutine": ["idcu::common"],
    "idcu-device-collector": ["idcu::common"],
    "idcu-discovery": ["idcu::common", "idcu::network"],
    "idcu-distributed": ["idcu::common", "idcu::network", "idcu::msgbus"],
    "idcu-module-system": ["idcu::common"],
    "idcu-module-isolation": ["idcu::common"],
    "idcu-module-verifier": ["idcu::common"],
    "idcu-msgbus": ["idcu::common", "idcu::log"],
    "idcu-permission": ["idcu::common"],
    "idcu-plugin": ["idcu::common"],
    "idcu-sandbox": ["idcu::common", "idcu::log"],
    "idcu-scheduler": ["idcu::common", "idcu::coroutine", "idcu::msgbus"],
    "idcu-security-enhanced": ["idcu::common"],
    "idcu-server-monitor": ["idcu::common"],
    "idcu-storage": ["idcu::common"],
    "idcu-testframework": ["idcu::common"],
    "idcu-watchdog": ["idcu::common"],
    "idcu-management": ["idcu::common", "idcu::http-server", "idcu::log"]
}

# CMakeLists.txt 模板
CMAKE_TEMPLATE = '''cmake_minimum_required(VERSION 3.14)
project({lib_name} C)

set(CMAKE_C_STANDARD 99)

file(GLOB_RECURSE SOURCES "src/idcu/{lib_suffix}/*.c")

add_library({lib_name} STATIC ${{SOURCES}})

target_include_directories({lib_name} PUBLIC
    $<BUILD_INTERFACE:${{CMAKE_CURRENT_SOURCE_DIR}}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries({lib_name} PUBLIC {dependencies})

add_library(idcu::{lib_alias} ALIAS {lib_name})
'''

def main():
    libs_dir = os.path.join(os.path.dirname(__file__), "libs")
    
    for lib_name, deps in LIB_DEPENDENCIES.items():
        lib_dir = os.path.join(libs_dir, lib_name)
        if not os.path.exists(lib_dir):
            continue
            
        # 从 lib_name 中提取后缀 (去掉 idcu- 前缀)
        lib_suffix = lib_name.replace("idcu-", "")
        lib_alias = lib_suffix
        
        cmake_path = os.path.join(lib_dir, "CMakeLists.txt")
        
        # 生成依赖字符串
        dep_str = " ".join(deps)
        
        # 填充模板
        content = CMAKE_TEMPLATE.format(
            lib_name=lib_name,
            lib_suffix=lib_suffix,
            lib_alias=lib_alias,
            dependencies=dep_str
        )
        
        # 写入文件
        with open(cmake_path, "w", encoding="utf-8") as f:
            f.write(content)
        
        print(f"已更新: {lib_name}/CMakeLists.txt")

if __name__ == "__main__":
    main()
