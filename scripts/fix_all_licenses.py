
import os
import re

def get_license_text(relative_path):
    """根据相对路径返回许可证文本"""
    return f"""

## 许可证

本库采用 [Apache License 2.0]({relative_path}) 许可证。
"""

def fix_readme(file_path, root_dir):
    """修复单个 README.md 文件"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 计算相对路径
        rel_dir = os.path.relpath(root_dir, os.path.dirname(file_path))
        if rel_dir == '.':
            license_path = 'LICENSE'
        else:
            license_path = os.path.join(rel_dir, 'LICENSE').replace('\\', '/')
        
        # 检查是否已有许可证部分
        license_pattern = r'\n*##\s*许可证.*?(?=\n##\s|\Z)'
        has_license = re.search(license_pattern, content, re.DOTALL)
        
        if has_license:
            # 替换现有的许可证部分
            new_content = re.sub(
                license_pattern,
                get_license_text(license_path),
                content,
                flags=re.DOTALL
            )
            if new_content != content:
                print(f'Updating (replacing): {file_path}')
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(new_content)
                return True
        else:
            # 添加新的许可证部分
            print(f'Adding: {file_path}')
            with open(file_path, 'a', encoding='utf-8') as f:
                f.write(get_license_text(license_path))
            return True
            
        return False
    except Exception as e:
        print(f'Error processing {file_path}: {e}')
        return False

def process_directory(root_dir, target_dir):
    """处理指定目录下的所有 README.md"""
    count = 0
    full_target_dir = os.path.join(root_dir, target_dir)
    for dirpath, dirnames, filenames in os.walk(full_target_dir):
        for filename in filenames:
            if filename == 'README.md':
                file_path = os.path.join(dirpath, filename)
                if fix_readme(file_path, root_dir):
                    count += 1
    return count

if __name__ == '__main__':
    root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(root_dir)
    
    print(f'Working directory: {root_dir}')
    print('=' * 60)
    
    print('\nProcessing libs/ directory:')
    libs_count = process_directory(root_dir, 'libs')
    
    print('\nProcessing modules/ directory:')
    modules_count = process_directory(root_dir, 'modules')
    
    print('\n' + '=' * 60)
    print(f'Total files updated: {libs_count + modules_count}')
    print(f'  - libs/: {libs_count}')
    print(f'  - modules/: {modules_count}')
