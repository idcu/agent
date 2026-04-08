#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
快速恢复所有库的简洁 CMakeLists.txt
"""

import os

# 简单的 CMakeLists.txt 模板
SIMPLE_CMAKE = '''cmake_minimum_required(VERSION 3.10)

include("${CMAKE_SOURCE_DIR}/libs/idcu-module-build/cmake/idcu_module_build.cmake")
idcu_build_module()
'''

def main():
    libs_dir = os.path.join(os.path.dirname(__file__), "libs")
    
    # 遍历 libs 目录下的所有子目录
    for item in os.listdir(libs_dir):
        lib_dir = os.path.join(libs_dir, item)
        if not os.path.isdir(lib_dir):
            continue
            
        # 跳过 idcu-module-build 本身
        if item == "idcu-module-build":
            continue
            
        cmake_path = os.path.join(lib_dir, "CMakeLists.txt")
        if not os.path.exists(cmake_path):
            continue
            
        # 写入简洁的 CMakeLists.txt
        with open(cmake_path, "w", encoding="utf-8") as f:
            f.write(SIMPLE_CMAKE)
        
        print(f"已恢复: {item}/CMakeLists.txt")

if __name__ == "__main__":
    main()
