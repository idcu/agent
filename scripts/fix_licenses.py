
import os
import re
import yaml

def fix_module_license(file_path):
    """修复 module.yaml 文件中的许可证"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 检查是否包含 MIT 许可证
        if 'license: MIT' not in content:
            return False
        
        # 替换 MIT 为 Apache-2.0
        new_content = re.sub(r'license:\s*MIT', 'license: Apache-2.0', content)
        
        if new_content != content:
            print(f'Fixing: {file_path}')
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(new_content)
            return True
        
        return False
    except Exception as e:
        print(f'Error processing {file_path}: {e}')
        return False

def process_directory(root_dir):
    """处理指定目录下的所有 module.yaml 文件"""
    count = 0
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename == 'module.yaml':
                file_path = os.path.join(dirpath, filename)
                if fix_module_license(file_path):
                    count += 1
    return count

if __name__ == '__main__':
    root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(root_dir)
    
    print(f'Working directory: {root_dir}')
    print('=' * 60)
    
    # 处理所有目录
    total_count = process_directory(root_dir)
    
    print('\n' + '=' * 60)
    print(f'Total module.yaml files updated: {total_count}')
