#!/usr/bin/env python3
import os
import sys
import json
import yaml
from pathlib import Path


def main():
    if len(sys.argv) < 2:
        print("Usage: load_module_config.py <module_path>", file=sys.stderr)
        return 1

    module_path = Path(sys.argv[1]).resolve()
    config_path = module_path / 'module.yaml'

    if not config_path.exists():
        print("{}", file=sys.stderr)
        return 1

    try:
        with open(config_path, 'r', encoding='utf-8') as f:
            config = yaml.safe_load(f) or {}
        
        module_config = config.get('module', config)
        
        result = {
            'name': module_config.get('name', module_path.name),
            'version': module_config.get('version', '0.0.0'),
            'description': module_config.get('description', ''),
            'author': module_config.get('author', ''),
            'license': module_config.get('license', ''),
            'use_legacy_build': module_config.get('use_legacy_build', False),
            'dependencies': module_config.get('dependencies', []),
            'build': config.get('build', {}),
            'testing': config.get('testing', {'enabled': True}),
            'features': config.get('features', []),
            'config_files': config.get('config_files', []),
            'scripts': config.get('scripts', []),
            'link_libraries': module_config.get('link_libraries', []),
            'platform_link_libraries': module_config.get('platform_link_libraries', {})
        }
        
        print(json.dumps(result, ensure_ascii=False))
        return 0
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        print("{}")
        return 1


if __name__ == '__main__':
    sys.exit(main())
