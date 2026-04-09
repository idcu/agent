#!/usr/bin/env python3
import os
import sys
import argparse
import subprocess
import yaml
from pathlib import Path

def load_config(config_path):
    with open(config_path, 'r', encoding='utf-8') as f:
        return yaml.safe_load(f)

def find_modules(root_dir):
    modules = []
    for path in Path(root_dir).rglob('module.yaml'):
        modules.append(path.parent)
    return sorted(modules)

def build_module(module_dir, build_type='Release'):
    build_dir = module_dir / 'build'
    build_dir.mkdir(exist_ok=True)

    cmake_args = [
        'cmake',
        '-B', str(build_dir),
        '-S', str(module_dir),
        f'-DCMAKE_BUILD_TYPE={build_type}',
    ]

    print(f"Configuring {module_dir.name}...")
    result = subprocess.run(cmake_args)
    if result.returncode != 0:
        return False

    print(f"Building {module_dir.name}...")
    build_args = ['cmake', '--build', str(build_dir), '--config', build_type]
    result = subprocess.run(build_args)
    return result.returncode == 0

def main():
    parser = argparse.ArgumentParser(description='IDCU Module Builder')
    parser.add_argument('--root', default='.', help='Root directory to search for modules')
    parser.add_argument('--config', help='Configuration file path')
    parser.add_argument('--build-type', default='Release', choices=['Debug', 'Release', 'RelWithDebInfo'])
    parser.add_argument('--list', action='store_true', help='List discovered modules')

    args = parser.parse_args()

    root_dir = Path(args.root).resolve()

    if args.config:
        config = load_config(args.config)
        print(f"Loaded config: {config}")

    modules = find_modules(root_dir)

    if args.list:
        print("Discovered modules:")
        for module in modules:
            print(f"  - {module.relative_to(root_dir)}")
        return 0

    print(f"Found {len(modules)} modules")

    success_count = 0
    for module in modules:
        if build_module(module, args.build_type):
            success_count += 1
        else:
            print(f"Failed to build {module.name}", file=sys.stderr)

    print(f"\nSuccessfully built {success_count}/{len(modules)} modules")
    return 0 if success_count == len(modules) else 1

if __name__ == '__main__':
    sys.exit(main())
