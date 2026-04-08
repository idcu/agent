#!/usr/bin/env python3
import os
import re

def complete_document(file_path, reference_doc):
    """补全不完整的文档"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        with open(reference_doc, 'r', encoding='utf-8') as f:
            ref_content = f.read()
        
        # 提取参考文档的各个章节
        ref_sections = {}
        section_pattern = re.compile(r'##\s+(\d+)\.\s+([^\n]+)\n([\s\S]*?)(?=##\s+\d+\.\s+|$)')
        for match in section_pattern.finditer(ref_content):
            section_num = match.group(1)
            section_title = match.group(2)
            section_content = match.group(3)
            ref_sections[f"{section_num}.{section_title}"] = section_content
        
        # 检查当前文档有哪些章节
        current_sections = set()
        for match in re.finditer(r'##\s+(\d+)\.\s+([^\n]+)', content):
            current_sections.add(f"{match.group(1)}.{match.group(2)}")
        
        # 补全缺失的章节
        updated_content = content
        
        # 检查是否有第5章（工程化要求）
        if not any(s.startswith('5.') for s in current_sections):
            # 添加工程化要求章节
            engineering_section = '''
## 5. 工程化要求

### 5.1 编码规范
- 对齐 .clang-format 规范
- 函数名小写+下划线
- 结构体前缀 idcu_

### 5.2 测试要求
- 单元测试覆盖率 ≥ 70%
- 测试覆盖正常和异常场景

### 5.3 部署指引
- 编译命令：cmake --build build
- 模块路径：libs/idcu-xxx/

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：核心功能实现复杂度高  
应对：分步骤实现，先完成核心功能，再优化

### 6.2 风险2
描述：性能不满足预期  
应对：进行性能基准测试，优化关键路径

### 6.3 风险3
描述：性能不满足预期要求  
应对：进行性能基准测试，优化关键路径代码

### 6.4 风险4
描述：跨平台兼容性问题  
应对：使用跨平台 API，充分测试不同平台

---

## 7. 详细实现步骤

（根据具体模块实现详细步骤）

---

## 8. 验证检查清单

- [ ] 头文件已创建
- [ ] 实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] 代码可以正常编译
- [ ] 单元测试通过
- [ ] 代码已格式化（clang-format）
- [ ] 静态分析通过（clang-tidy）
- [ ] 符合工程化标准要求
- [ ] 性能测试满足指标要求
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-xxx/
git commit -m "feat: add idcu-xxx library

- Add core functionality
- Add CMake build configuration
- Add unit tests
- Add documentation"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 功能异常 | 实现有误 | 检查核心实现逻辑 |
| 性能不达标 | 算法效率低 | 优化算法和数据结构 |
'''
            # 在第4章之后插入
            updated_content = re.sub(
                r'(## 4\. 执行计划[\s\S]*?)(---\s*$)',
                r'\1' + engineering_section,
                updated_content
            )
        
        # 如果已经有部分章节，检查是否需要补全
        if '## 5. 工程化要求' in updated_content and '### 5.4 相关参考文档' not in updated_content:
            # 添加 5.4 相关参考文档
            ref_docs = '''
### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

'''
            updated_content = re.sub(
                r'(### 5\.3 部署指引[\s\S]*?)(---\s*## 6\.)',
                r'\1' + ref_docs + r'\2',
                updated_content
            )
        
        if '## 6. 风险与应对' in updated_content:
            # 检查是否有 6.3 和 6.4
            if '### 6.3 风险3' not in updated_content:
                additional_risks = '''
### 6.3 风险3
描述：性能不满足预期要求  
应对：进行性能基准测试，优化关键路径代码

### 6.4 风险4
描述：跨平台兼容性问题  
应对：使用跨平台 API，充分测试不同平台

'''
                updated_content = re.sub(
                    r'(### 6\.2 风险2[\s\S]*?)(---\s*## 7\.)',
                    r'\1' + additional_risks + r'\2',
                    updated_content
                )
        
        if '## 8. 验证检查清单' in updated_content:
            # 检查是否有工程化相关的检查项
            if '符合工程化标准' not in updated_content:
                additional_checks = '''
- [ ] 代码已格式化（clang-format）
- [ ] 静态分析通过（clang-tidy）
- [ ] 符合工程化标准要求
- [ ] 性能测试满足指标要求

'''
                updated_content = re.sub(
                    r'(## 8\. 验证检查清单[\s\S]*?)(---\s*## 9\.)',
                    r'\1' + additional_checks + r'\2',
                    updated_content
                )
        
        # 写回文件
        if updated_content != content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(updated_content)
            print(f"  补全成功: {file_path}")
            return True
        else:
            print(f"  无需补全: {file_path}")
            return False
            
    except Exception as e:
        print(f"  处理失败 {file_path}: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    base_dir = '/workspace/docs/tasks'
    reference_doc = '/workspace/docs/tasks/phase3/01_idcu_log.md'
    
    # 需要补全的文档
    incomplete_docs = [
        'phase3/22_idcu_plugin.md',
        'phase3/13_idcu_msgbus.md',
        'phase3/21_idcu_permission.md',
        'phase3/29_idcu_module_verifier.md',
        'phase4/06_metrics_integration.md'
    ]
    
    total_updated = 0
    
    for doc_path in incomplete_docs:
        full_path = os.path.join(base_dir, doc_path)
        if os.path.exists(full_path):
            print(f"\n处理 {doc_path}...")
            if complete_document(full_path, reference_doc):
                total_updated += 1
    
    print(f"\n补全完成！共更新 {total_updated} 个文档")

if __name__ == '__main__':
    main()
