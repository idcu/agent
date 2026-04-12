#!/usr/bin/env python3
"""
IDCU Agent 主构建脚本
完全基于 module.yaml 的构建系统
"""
import os
import sys
from pathlib import Path

# 添加 idcu-module-build 的 scripts 目录到路径
script_dir = Path(__file__).parent / 'libs' / 'idcu-module-build' / 'scripts'
sys.path.insert(0, str(script_dir))

# 导入并运行 build_from_yaml
from build_from_yaml import main

if __name__ == '__main__':
    sys.exit(main())
