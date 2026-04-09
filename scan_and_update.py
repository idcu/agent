#!/usr/bin/env python3
import os
import re

def check_document_structure(file_path):
    """检查文档的结构完整性"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        has_section_5 = bool(re.search(r'##\s+\d+\.\s+工程化要求', content))
        has_section_6 = bool(re.search(r'##\s+\d+\.\s+风险与应对', content))
        has_section_8 = bool(re.search(r'##\s+\d+\.\s+验证检查清单', content))
        has_ref_docs = '相关参考文档' in content
        
        return {
            'file': file_path,
            'has_section_5': has_section_5,
            'has_section_6': has_section_6,
            'has_section_8': has_section_8,
            'has_ref_docs': has_ref_docs,
            'needs_update': not has_ref_docs
        }
    except Exception as e:
        return {
            'file': file_path,
            'error': str(e)
        }

def main():
    base_dir = '/workspace/docs/tasks'
    phases = ['phase1', 'phase2', 'phase3', 'phase4', 'phase5']
    
    all_docs = []
    
    for phase in phases:
        phase_dir = os.path.join(base_dir, phase)
        if not os.path.exists(phase_dir):
            continue
            
        print(f"\n扫描 {phase}...")
        
        for filename in os.listdir(phase_dir):
            if filename.endswith('.md') and not filename.startswith('00_'):
                file_path = os.path.join(phase_dir, filename)
                result = check_document_structure(file_path)
                all_docs.append(result)
    
    print(f"\n{'='*80}")
    print("文档状态统计：")
    print(f"{'='*80}")
    
    needs_update = [d for d in all_docs if d.get('needs_update', False)]
    already_updated = [d for d in all_docs if not d.get('needs_update', False) and 'error' not in d]
    errors = [d for d in all_docs if 'error' in d]
    
    print(f"\n总文档数: {len(all_docs)}")
    print(f"已更新: {len(already_updated)}")
    print(f"需要更新: {len(needs_update)}")
    print(f"有错误: {len(errors)}")
    
    if needs_update:
        print(f"\n需要更新的文档:")
        for d in needs_update:
            print(f"  - {d['file']}")
    
    if errors:
        print(f"\n有错误的文档:")
        for d in errors:
            print(f"  - {d['file']}: {d['error']}")

if __name__ == '__main__':
    main()
