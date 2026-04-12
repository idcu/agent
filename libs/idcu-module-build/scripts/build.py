#!/usr/bin/env python3
import os
import sys
import argparse
import subprocess
import yaml
from pathlib import Path
from typing import Dict, List, Set


class ModuleInfo:
    def __init__(self, path: Path, config: dict):
        self.path = path
        self.config = config
        
        # 支持两种格式：有module字段和没有module字段
        module_config = config.get('module', config)
        self.name = module_config.get('name', path.name)
        self.dependencies = module_config.get('dependencies', [])
        self.build_config = config.get('build', {})
        self.testing_enabled = config.get('testing', {}).get('enabled', True)

    def __repr__(self):
        return f"ModuleInfo(name={self.name}, path={self.path})"


def load_module_config(module_path: Path) -> ModuleInfo:
    """加载模块的配置文件"""
    config_path = module_path / 'module.yaml'
    with open(config_path, 'r', encoding='utf-8') as f:
        config = yaml.safe_load(f) or {}
    return ModuleInfo(module_path, config)


def find_modules(root_dir: Path) -> List[ModuleInfo]:
    """查找所有模块并加载它们的配置"""
    modules = []
    for path in root_dir.rglob('module.yaml'):
        module_path = path.parent
        try:
            module_info = load_module_config(module_path)
            modules.append(module_info)
        except Exception as e:
            print(f"警告: 无法加载模块 {module_path} 的配置: {e}", file=sys.stderr)
    return modules


def topological_sort(modules: List[ModuleInfo]) -> List[ModuleInfo]:
    """对模块进行拓扑排序，确保依赖项先被构建"""
    name_to_module = {m.name: m for m in modules}
    visited = set()
    temp = set()
    result = []

    def visit(module: ModuleInfo):
        if module.name in temp:
            raise ValueError(f"检测到循环依赖: {module.name}")
        if module.name in visited:
            return

        temp.add(module.name)
        for dep_name in module.dependencies:
            if dep_name in name_to_module:
                visit(name_to_module[dep_name])
        temp.remove(module.name)
        visited.add(module.name)
        result.append(module)

    for module in modules:
        if module.name not in visited:
            visit(module)

    return result


def build_module(module_info: ModuleInfo, build_type: str = 'Release', build_tests: bool = True) -> bool:
    """构建单个模块"""
    module_dir = module_info.path
    build_dir = module_dir / 'build'
    build_dir.mkdir(exist_ok=True)

    cmake_args = [
        'cmake',
        '-B', str(build_dir),
        '-S', str(module_dir),
        f'-DCMAKE_BUILD_TYPE={build_type}',
    ]

    # 根据module.yaml的配置添加编译选项
    if build_tests and module_info.testing_enabled:
        cmake_args.append('-DBUILD_TESTS=ON')
    else:
        cmake_args.append('-DBUILD_TESTS=OFF')

    print(f"配置模块 {module_info.name}...")
    result = subprocess.run(cmake_args, cwd=module_dir)
    if result.returncode != 0:
        return False

    print(f"构建模块 {module_info.name}...")
    build_args = ['cmake', '--build', str(build_dir), '--config', build_type]
    result = subprocess.run(build_args, cwd=module_dir)
    return result.returncode == 0


def main():
    parser = argparse.ArgumentParser(description='IDCU Module Builder')
    parser.add_argument('--root', default='.', help='Root directory to search for modules')
    parser.add_argument('--config', help='Configuration file path')
    parser.add_argument('--build-type', default='Release', choices=['Debug', 'Release', 'RelWithDebInfo'])
    parser.add_argument('--list', action='store_true', help='List discovered modules')
    parser.add_argument('--no-tests', action='store_true', help='Skip building tests')
    parser.add_argument('--module', help='Build only specific module(s) (comma-separated)')

    args = parser.parse_args()

    root_dir = Path(args.root).resolve()

    if args.config:
        with open(args.config, 'r', encoding='utf-8') as f:
            config = yaml.safe_load(f)
        print(f"加载配置: {config}")

    print("正在发现模块...")
    modules = find_modules(root_dir)
    print(f"发现 {len(modules)} 个模块")

    if args.list:
        print("\n发现的模块:")
        for module in modules:
            print(f"  - {module.name}")
            print(f"    路径: {module.path.relative_to(root_dir)}")
            print(f"    依赖: {', '.join(module.dependencies) if module.dependencies else '无'}")
            print(f"    测试: {'启用' if module.testing_enabled else '禁用'}")
        return 0

    # 如果指定了特定模块，只构建这些模块及其依赖
    if args.module:
        target_names = set(args.module.split(','))
        name_to_module = {m.name: m for m in modules}
        
        # 收集目标模块及其依赖
        def collect_deps(name: str, collected: Set[str]):
            if name in collected:
                return
            collected.add(name)
            if name in name_to_module:
                for dep in name_to_module[name].dependencies:
                    collect_deps(dep, collected)
        
        all_required = set()
        for name in target_names:
            collect_deps(name, all_required)
        
        modules = [m for m in modules if m.name in all_required]
        print(f"将构建 {len(modules)} 个模块（包括依赖）")

    # 按依赖顺序排序
    try:
        sorted_modules = topological_sort(modules)
    except ValueError as e:
        print(f"错误: {e}", file=sys.stderr)
        return 1

    print("\n构建顺序:")
    for i, module in enumerate(sorted_modules, 1):
        print(f"  {i}. {module.name}")

    print()
    success_count = 0
    failed_modules = []

    for module in sorted_modules:
        if build_module(module, args.build_type, not args.no_tests):
            success_count += 1
            print(f"[OK] {module.name} 构建成功\n")
        else:
            failed_modules.append(module.name)
            print(f"[FAIL] {module.name} 构建失败\n", file=sys.stderr)

    print("=" * 50)
    print(f"构建完成: {success_count}/{len(sorted_modules)} 成功")
    
    if failed_modules:
        print(f"失败的模块: {', '.join(failed_modules)}", file=sys.stderr)
        return 1
    
    print("所有模块构建成功!")
    return 0


if __name__ == '__main__':
    sys.exit(main())
