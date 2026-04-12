
import os
import re

def fix_license_in_file(file_path):
    """修复文件中的 MIT 许可证为 Apache-2.0"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 检查是否包含 MIT 许可证
        if 'MIT' not in content:
            return False
        
        # 替换各种形式的 MIT 许可证
        new_content = content
        new_content = re.sub(r'"license":\s*"MIT"', '"license": "Apache-2.0"', new_content)
        new_content = re.sub(r'license:\s*MIT', 'license: Apache-2.0', new_content)
        
        if new_content != content:
            print(f'Fixing: {file_path}')
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(new_content)
            return True
        
        return False
    except Exception as e:
        print(f'Error processing {file_path}: {e}')
        return False

def process_directory(root_dir, target_dir):
    """处理指定目录下的所有文件"""
    count = 0
    full_target_dir = os.path.join(root_dir, target_dir)
    for dirpath, dirnames, filenames in os.walk(full_target_dir):
        for filename in filenames:
            if filename.endswith('.md'):
                file_path = os.path.join(dirpath, filename)
                if fix_license_in_file(file_path):
                    count += 1
    return count

if __name__ == '__main__':
    root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(root_dir)
    
    print(f'Working directory: {root_dir}')
    print('=' * 60)
    
    # 处理 docs 目录
    print('\nProcessing docs/ directory:')
    docs_count = process_directory(root_dir, 'docs')
    
    print('\n' + '=' * 60)
    print(f'Total files updated: {docs_count}')
