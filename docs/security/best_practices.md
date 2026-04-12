# IDCU Agent 安全最佳实践

## 概述

本文档提供了在使用 IDCU Agent 开发和部署应用程序时的安全最佳实践指南。

## 内存安全

### 1. 使用内存池

- 优先使用 `idcu-memory` 库提供的内存池进行内存管理
- 内存池提供了更好的内存跟踪和泄漏检测功能
- 在调试版本中，内存池会记录分配信息便于排查问题

```c
// 推荐做法
idcu_memory_pool_t* pool = idcu_memory_pool_create(1024 * 1024);
void* ptr = idcu_memory_pool_alloc(pool, 256);
// 使用 ptr
idcu_memory_pool_free(pool, ptr);
idcu_memory_pool_destroy(pool);
```

### 2. 内存泄漏检测

- 在调试模式下启用内存泄漏检测
- 程序退出时检查未释放的内存
- 使用内存调试工具 (`idcu-memory-debugger`) 进行实时监控

### 3. 缓冲区安全

- 始终检查缓冲区边界
- 使用安全的字符串操作函数
- 避免使用不安全的函数（如 `strcpy`, `sprintf`）

```c
// 安全做法
char dest[256];
snprintf(dest, sizeof(dest), "Format string: %s", src);
```

## 并发安全

### 1. 互斥锁使用

- 使用 `idcu-common` 库提供的互斥锁
- 在调试模式下启用死锁检测
- 遵循锁的获取顺序以避免死锁

```c
idcu_mutex_t lock;
idcu_mutex_init(&lock);

idcu_mutex_lock(&lock);
// 临界区代码
idcu_mutex_unlock(&lock);

idcu_mutex_destroy(&lock);
```

### 2. 线程安全 API

- 查阅文档确认 API 的线程安全属性
- 对于非线程安全的 API，需要外部同步
- 使用消息总线进行线程间通信而非共享数据

### 3. 数据竞争检测

- 使用 ThreadSanitizer (TSAN) 检测数据竞争
- 在 CI/CD 流程中定期运行 TSAN 检查
- 避免在多个线程中无保护地访问共享数据

## 输入验证

### 1. 参数检查

- 所有公共 API 都应验证输入参数
- 检查空指针
- 验证参数范围

```c
idcu_error_t some_api(const char* str, size_t len) {
    if (str == NULL) {
        return IDCU_ERROR_NULL_POINTER;
    }
    if (len == 0 || len > MAX_LEN) {
        return IDCU_ERROR_INVALID_ARGUMENT;
    }
    // 处理逻辑
    return IDCU_SUCCESS;
}
```

### 2. 数据解析

- 验证所有外部输入数据
- 使用安全的解析器（`idcu-json`, `idcu-yaml`）
- 处理解析错误并提供有意义的错误信息

### 3. 整数溢出防护

- 使用安全的算术运算
- 检查计算结果是否溢出
- 验证数组索引范围

## 错误处理

### 1. 错误码检查

- 始终检查 API 返回的错误码
- 不要忽略错误
- 提供合理的错误恢复机制

### 2. 错误传播

- 正确传播错误给调用者
- 提供足够的错误上下文信息
- 记录错误以便调试

## 配置安全

### 1. 配置验证

- 验证所有配置值的有效性
- 为配置项提供默认值
- 记录配置变更

### 2. 敏感信息

- 不要在日志中记录敏感信息
- 安全存储凭据和密钥
- 使用环境变量传递敏感配置

## 网络安全

### 1. 连接管理

- 设置合理的连接超时
- 验证服务器证书（如使用 TLS）
- 限制连接频率防止滥用

### 2. 数据传输

- 考虑使用加密传输敏感数据
- 验证接收到的数据完整性
- 处理网络错误和重连逻辑

## 测试安全

### 1. 安全测试

- 运行单元测试验证安全功能
- 使用模糊测试发现潜在漏洞
- 定期进行安全审计

### 2. Sanitizer 使用

- 在开发过程中使用 AddressSanitizer
- 使用 ThreadSanitizer 检测并发问题
- 使用 UndefinedBehaviorSanitizer 检测未定义行为

```bash
# 使用 ASAN 构建
cmake -DENABLE_ASAN=ON ..
make
make test

# 使用 TSAN 构建
cmake -DENABLE_TSAN=ON ..
make
make test

# 使用 UBSAN 构建
cmake -DENABLE_UBSAN=ON ..
make
make test
```

## 代码审查

### 1. 安全关注点

- 内存管理是否正确
- 并发访问是否安全
- 输入是否经过验证
- 错误处理是否完善

### 2. 审查清单

- [ ] 所有内存分配是否有对应的释放
- [ ] 共享数据访问是否有适当的同步
- [ ] 外部输入是否经过验证
- [ ] 错误码是否被检查和处理
- [ ] 是否存在缓冲区溢出风险
- [ ] 是否存在整数溢出风险

## 持续集成

### 1. CI 安全检查

- 在 CI 中运行 Sanitizer 检查
- 运行静态分析工具
- 确保所有测试通过

### 2. 自动化安全测试

- 定期运行模糊测试
- 自动化安全扫描
- 监控依赖项的安全更新

## 总结

遵循这些安全最佳实践可以显著提高 IDCU Agent 应用程序的安全性和可靠性。安全是一个持续的过程，需要在开发生命周期的每个阶段都给予关注。

