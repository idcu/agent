#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
IDCU Module Build - Cross-Platform Build Script
通用跨平台构建脚本 - 使用 Python 实现统一入口
"""

import os
import sys
import subprocess
import argparse
import platform


def get_script_dir():
    """获取脚本所在目录"""
    return os.path.dirname(os.path.abspath(__file__))


def get_project_dir():
    """获取项目根目录"""
    return os.path.dirname(get_script_dir())


def is_windows():
    """检测是否为 Windows 系统"""
    return platform.system() == "Windows"


def is_macos():
    """检测是否为 macOS 系统"""
    return platform.system() == "Darwin"


def is_linux():
    """检测是否为 Linux 系统"""
    return platform.system() == "Linux"


def print_header():
    """打印头部信息"""
    print("=" * 40)
    print("IDCU Module Build - Cross-Platform")
    print(f"Platform: {platform.system()} {platform.release()}")
    print(f"Architecture: {platform.machine()}")
    print("=" * 40)


def clean_build(build_dir):
    """清理构建目录"""
    print(f"Cleaning build directory: {build_dir}")
    if os.path.exists(build_dir):
        import shutil
        shutil.rmtree(build_dir)
    print("Clean complete!")
    return 0


def run_command(cmd, cwd=None):
    """运行命令并返回退出码"""
    print(f"Running: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, cwd=cwd, check=True)
        return result.returncode
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        return e.returncode


def main():
    parser = argparse.ArgumentParser(
        description="IDCU Module Build - Cross-Platform Build Script"
    )
    parser.add_argument(
        "action",
        nargs="?",
        default="release",
        choices=["clean", "debug", "release"],
        help="Build action: clean, debug, or release (default: release)"
    )
    parser.add_argument(
        "module_dir",
        nargs="?",
        default="",
        help="Module directory to build (optional)"
    )
    
    args = parser.parse_args()
    
    print_header()
    
    script_dir = get_script_dir()
    project_dir = get_project_dir()
    
    if args.action == "clean":
        build_dir = os.path.join(project_dir, "build")
        return clean_build(build_dir)
    
    # 确定构建类型
    build_type = "Debug" if args.action == "debug" else "Release"
    
    # 根据平台选择对应的脚本
    if is_windows():
        bat_script = os.path.join(script_dir, "build.bat")
        cmd_args = [bat_script, args.action]
        if args.module_dir:
            cmd_args.append(args.module_dir)
        return run_command(cmd_args)
    else:
        sh_script = os.path.join(script_dir, "build.sh")
        # 确保脚本有执行权限
        if not os.access(sh_script, os.X_OK):
            os.chmod(sh_script, 0o755)
        cmd_args = [sh_script, args.action]
        if args.module_dir:
            cmd_args.append(args.module_dir)
        return run_command(cmd_args)


if __name__ == "__main__":
    sys.exit(main())
