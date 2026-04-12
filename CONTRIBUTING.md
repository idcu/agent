# 贡献指南

感谢您有兴趣为 IDCU Agent 项目做出贡献！

## 行为准则

### 我们的承诺

为了营造开放和友好的环境，我们承诺：
- 尊重不同的观点和经验
- 优雅地接受建设性批评
- 关注对社区最有利的事情

## 如何贡献

### 报告问题

1. 在提交新问题之前，请先搜索现有问题
2. 使用清晰的标题和描述
3. 提供重现步骤
4. 包含预期行为和实际行为
5. 如果可能，提供代码示例或测试用例

### 提交代码

#### 准备工作

1. Fork 仓库
2. 克隆您的 Fork
3. 配置开发环境（见 [快速开始指南](docs/guide/QUICKSTART.md)）

#### 开发流程

1. 从 `main` 分支创建新分支
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. 编写代码
   - 遵循项目的编码规范
   - 添加或更新测试
   - 更新文档（如需要）

3. 确保代码质量
   ```bash
   # 格式化代码
   clang-format -i your-file.c
   
   # 运行测试
   ctest
   
   # 运行静态分析（如配置）
   clang-tidy your-file.c
   ```

4. 提交更改
   ```bash
   git add .
   git commit -m "feat: add your feature description"
   ```
   使用英文 commit message，遵循 Conventional Commits 规范。

5. 推送到您的 Fork
   ```bash
   git push origin feature/your-feature-name
   ```

6. 创建 Pull Request

## 编码规范

### 代码风格

- 使用 4 空格缩进（不要使用 Tab）
- 每行不超过 100 字符
- 大括号不换行
- 运算符前后加空格

示例：
```c
int example_function(int arg1, int arg2)
{
    int result = arg1 + arg2;
    if (result > 0) {
        return result;
    }
    return 0;
}
```

### 命名约定

- 函数名: `idcu_模块名_动作名`，如 `idcu_log_init()`
- 类型名: `idcu_模块名_TypeName`，如 `idcu_log_LogLevel`
- 宏: 全大写，下划线分隔，如 `IDCU_LOG_DEBUG`
- 私有符号: 以下划线开头，如 `_internal_helper()`

### 文档

- 所有公共 API 必须有 Doxygen 文档注释
- 使用中文编写文档和注释
- 示例代码应简洁明了

### 错误处理

- 所有公共 API 应返回 `idcu_ErrorCode`
- 使用 `idcu_err_set_last_error()` 设置详细错误信息
- 检查所有输入参数

## 测试

### 编写测试

- 新功能必须包含测试
- 测试应覆盖正常和错误情况
- 使用 idcu-testframework 编写测试

### 运行测试

```bash
# 构建并运行所有测试
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
```

## Pull Request 指南

### PR 标题

使用 Conventional Commits 格式：
- `feat: 新功能`
- `fix: 修复 bug`
- `docs: 文档更新`
- `style: 代码格式调整`
- `refactor: 重构`
- `test: 测试相关`
- `chore: 构建/工具相关`

### PR 描述

- 清晰描述变更的目的
- 关联相关的 issue（如适用）
- 说明测试方法

### 审核流程

1. 至少一名维护者审核
2. 所有 CI 检查必须通过
3. 解决所有审核意见
4. Squash 合并到 main 分支

## 获取帮助

- 查看 [文档](docs/)
- 提交 Issue
- 发送邮件给维护者

## 许可证

通过贡献代码，您同意您的贡献将根据项目的许可证进行许可。
