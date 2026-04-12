#!/usr/bin/env python3
"""
完全基于 module.yaml 的构建系统
不需要 CMakeLists.txt 文件
"""
import os
import sys
import argparse
import subprocess
import yaml
import shutil
from pathlib import Path
from typing import Dict, List, Set, Optional


class ModuleInfo:
    def __init__(self, path: Path, config: dict):
        self.path = path
        self.config = config
        
        # 支持两种格式：有module字段和没有module字段
        module_config = config.get('module', config)
        self.name = module_config.get('name', path.name)
        self.version = module_config.get('version', '1.0.0')
        self.description = module_config.get('description', '')
        self.author = module_config.get('author', '')
        self.license = module_config.get('license', 'Apache-2.0')
        self.dependencies = module_config.get('dependencies', [])
        self.build_config = config.get('build', {})
        self.headers = config.get('headers', [])
        self.features = config.get('features', [])
        self.testing_enabled = config.get('testing', {}).get('enabled', False)
        
        # 从 build 配置中获取 targets
        self.targets = self.build_config.get('targets', [self.name])
        
        # 自动发现源文件
        self.sources = self._discover_sources()
        
    def _discover_sources(self) -> List[Path]:
        """自动发现源文件，处理平台特定的源文件"""
        sources = []
        src_dir = self.path / 'src'
        if not src_dir.exists():
            return sources
        
        # 判断当前平台
        import platform
        system = platform.system().lower()
        
        # 所有平台目录列表
        all_platform_dirs = {
            'windows',
            'linux',
            'macos',
            'freebsd',
            'android',
            'harmony',
            'vector',
            'rtthread',
        }
        
        # 平台目录映射（系统名 -> 目录名）
        platform_mapping = {
            'windows': 'windows',
            'linux': 'linux',
            'darwin': 'macos',
            'freebsd': 'freebsd',
        }
        
        target_platform = platform_mapping.get(system, 'linux')
        
        # 查找所有源文件（递归）
        for ext in ['*.c', '*.cpp', '*.cc', '*.cxx']:
            for file in src_dir.rglob(ext):
                # 获取相对于 src_dir 的路径
                rel_path = file.relative_to(src_dir)
                path_parts = list(rel_path.parts)
                
                # 检查是否在平台目录中
                in_platform_dir = False
                is_target_platform = False
                for part in path_parts[:-1]:  # 不包括文件名
                    if part in all_platform_dirs:
                        in_platform_dir = True
                        if part == target_platform:
                            is_target_platform = True
                        break
                
                # 如果不在任何平台目录，或者在目标平台目录，则包含
                if not in_platform_dir or is_target_platform:
                    sources.append(file)
        
        return sorted(sources)
    
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
        # 跳过 build 目录
        if 'build' in module_path.parts:
            continue
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


def generate_compile_commands(module: ModuleInfo, build_dir: Path, include_dirs: List[Path], 
                               defines: List[str], cflags: List[str]) -> List[str]:
    """生成编译命令"""
    commands = []
    obj_dir = build_dir / 'obj'
    obj_dir.mkdir(parents=True, exist_ok=True)
    
    for src in module.sources:
        rel_path = src.relative_to(module.path)
        obj_file = obj_dir / f"{rel_path.with_suffix('.o').name}"
        
        cmd = [
            'gcc',
            '-c', str(src),
            '-o', str(obj_file),
            '-std=c11',
            '-Wall',
            '-Wextra',
        ]
        
        # 添加 include 目录
        for inc_dir in include_dirs:
            cmd.extend(['-I', str(inc_dir)])
        
        # 添加模块自己的 include 目录
        module_inc = module.path / 'include'
        if module_inc.exists():
            cmd.extend(['-I', str(module_inc)])
        
        # 添加 defines
        for define in defines:
            cmd.extend(['-D', define])
        
        # 添加额外的 cflags
        cmd.extend(cflags)
        
        commands.append((cmd, obj_file))
    
    return commands


def build_module(module: ModuleInfo, build_root: Path, all_modules: List[ModuleInfo],
                 build_type: str = 'Release', build_tests: bool = True) -> bool:
    """构建单个模块"""
    print(f"{'='*60}")
    print(f"构建模块: {module.name}")
    print(f"{'='*60}")
    
    module_dir = module.path
    build_dir = build_root / module.name
    build_dir.mkdir(parents=True, exist_ok=True)
    
    # 收集依赖模块的 include 目录
    include_dirs = []
    name_to_module = {m.name: m for m in all_modules}
    
    def collect_includes(mod_name: str):
        if mod_name in name_to_module:
            mod = name_to_module[mod_name]
            inc_dir = mod.path / 'include'
            if inc_dir.exists() and inc_dir not in include_dirs:
                include_dirs.append(inc_dir)
            # 递归收集依赖的依赖
            for dep in mod.dependencies:
                collect_includes(dep)
    
    for dep in module.dependencies:
        collect_includes(dep)
    
    # 定义编译选项
    defines = []
    cflags = []
    
    if build_type == 'Debug':
        defines.append('DEBUG=1')
        cflags.extend(['-g', '-O0'])
    else:
        defines.append('NDEBUG=1')
        cflags.extend(['-O3'])
    
    # 添加平台特定的 defines
    import platform
    system = platform.system().lower()
    if system == 'windows':
        defines.append('WIN32')
        defines.append('_WIN32')
    
    # 生成编译命令
    compile_commands = generate_compile_commands(module, build_dir, include_dirs, defines, cflags)
    
    if not compile_commands:
        print(f"警告: 模块 {module.name} 没有源文件")
        return True
    
    # 编译每个源文件
    success = True
    object_files = []
    
    for cmd, obj_file in compile_commands:
        src_name = Path(cmd[2]).name
        print(f"  编译: {src_name}")
        
        result = subprocess.run(cmd, cwd=module_dir)
        if result.returncode != 0:
            print(f"  [FAIL] 编译失败: {src_name}")
            success = False
        else:
            object_files.append(obj_file)
    
    if not success:
        return False
    
    # 链接静态库
    if object_files:
        lib_name = f"lib{module.name}.a"
        lib_path = build_dir / lib_name
        
        print(f"  链接: {lib_name}")
        
        ar_cmd = ['ar', 'rcs', str(lib_path)] + [str(obj) for obj in object_files]
        result = subprocess.run(ar_cmd, cwd=build_dir)
        
        if result.returncode != 0:
            print(f"  [FAIL] 链接失败")
            return False
        
        print(f"  [OK] 构建成功: {lib_path}")
    
    return True


def main():
    parser = argparse.ArgumentParser(description='IDCU Module Builder (YAML-only)')
    parser.add_argument('--root', default='.', help='Root directory to search for modules')
    parser.add_argument('--build-dir', default='build_yaml', help='Build directory')
    parser.add_argument('--build-type', default='Release', choices=['Debug', 'Release', 'RelWithDebInfo'])
    parser.add_argument('--list', action='store_true', help='List discovered modules')
    parser.add_argument('--no-tests', action='store_true', help='Skip building tests')
    parser.add_argument('--module', help='Build only specific module(s) (comma-separated)')
    parser.add_argument('--clean', action='store_true', help='Clean build directory first')

    args = parser.parse_args()

    root_dir = Path(args.root).resolve()
    build_root = Path(args.build_dir).resolve()

    if args.clean and build_root.exists():
        print(f"清理构建目录: {build_root}")
        shutil.rmtree(build_root)

    print("正在发现模块...")
    all_modules = find_modules(root_dir)
    print(f"发现 {len(all_modules)} 个模块")

    if args.list:
        print("\n发现的模块:")
        for module in all_modules:
            print(f"  - {module.name}")
            print(f"    路径: {module.path.relative_to(root_dir)}")
            print(f"    版本: {module.version}")
            print(f"    描述: {module.description}")
            print(f"    依赖: {', '.join(module.dependencies) if module.dependencies else '无'}")
            print(f"    源文件: {len(module.sources)} 个")
            print(f"    测试: {'启用' if module.testing_enabled else '禁用'}")
        return 0

    # 如果指定了特定模块，只构建这些模块及其依赖
    modules_to_build = all_modules
    if args.module:
        target_names = set(args.module.split(','))
        name_to_module = {m.name: m for m in all_modules}
        
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
        
        modules_to_build = [m for m in all_modules if m.name in all_required]
        print(f"将构建 {len(modules_to_build)} 个模块（包括依赖）")

    # 按依赖顺序排序
    try:
        sorted_modules = topological_sort(modules_to_build)
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
        if build_module(module, build_root, all_modules, args.build_type, not args.no_tests):
            success_count += 1
            print(f"\n[OK] {module.name} 构建成功\n")
        else:
            failed_modules.append(module.name)
            print(f"\n[FAIL] {module.name} 构建失败\n", file=sys.stderr)

    print("=" * 60)
    print(f"构建完成: {success_count}/{len(sorted_modules)} 成功")
    
    if failed_modules:
        print(f"失败的模块: {', '.join(failed_modules)}", file=sys.stderr)
        return 1
    
    print("所有模块构建成功!")
    return 0


if __name__ == '__main__':
    sys.exit(main())
