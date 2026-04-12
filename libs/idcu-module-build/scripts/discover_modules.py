#!/usr/bin/env python3
import os
import sys
import json
import yaml
from pathlib import Path


def main():
    if len(sys.argv) < 2:
        print("Usage: discover_modules.py <root_dir>", file=sys.stderr)
        return 1

    root_dir = Path(sys.argv[1]).resolve()
    modules = []

    for path in root_dir.rglob('module.yaml'):
        module_path = path.parent
        try:
            with open(path, 'r', encoding='utf-8') as f:
                config = yaml.safe_load(f) or {}
            
            module_config = config.get('module', config)
            name = module_config.get('name', module_path.name)
            
            modules.append({
                'path': str(module_path.relative_to(root_dir)),
                'absolute_path': str(module_path),
                'name': name
            })
        except Exception as e:
            print(f"Warning: Failed to load {path}: {e}", file=sys.stderr)

    print(json.dumps(modules, ensure_ascii=False))
    return 0


if __name__ == '__main__':
    sys.exit(main())
