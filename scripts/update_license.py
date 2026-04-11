
import os

license_text = """

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
"""

def update_readme(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    if '## 许可证' not in content:
        print(f'Updating: {file_path}')
        with open(file_path, 'a', encoding='utf-8') as f:
            f.write(license_text)

def update_directory(root_dir):
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename == 'README.md':
                file_path = os.path.join(dirpath, filename)
                update_readme(file_path)

if __name__ == '__main__':
    print('Updating README files in libs/')
    update_directory('libs')
    
    print('\nUpdating README files in modules/')
    update_directory('modules')
    
    print('\nLicense update completed!')
