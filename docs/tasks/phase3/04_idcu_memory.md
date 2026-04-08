# 任务 3.4: idcu-memory - 内存池管理库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建高效的内存池管理库，支持多粒度内存块分配（最多 8 个尺寸类别）、线程安全操作、内存使用统计、峰值使用追踪、安全检查（空指针、缓冲区溢出），满足分配/释放延迟 ≤ 100ns、内存碎片率 ≤ 5% 的性能要求。

### 1.2 不做什么
- 不实现可变大小内存块（仅支持固定尺寸类别）
- 不实现内存压缩或垃圾回收
- 不实现跨进程共享内存
- 不实现持久化内存池

### 1.3 输入
- 内存池初始化配置
- 分配大小（字节）
- 要释放的内存指针
- 安全检查参数

### 1.4 输出
- 分配的内存指针
- 统计信息（空闲块数、累计分配、峰值使用等）
- 安全检查结果
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用（提供锁、错误码等）
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **内存池架构**: 多尺寸类别（Size Class）设计，最多 8 个类别
- **空闲链表**: 每个尺寸类别独立的空闲链表
- **线程安全**: 每个尺寸类别独立锁，减少锁竞争
- **安全检查**: 内存保护标记（Guard）、头部魔法数验证

### 2.2 核心逻辑
```
初始化流程：
1. 预配置 8 个尺寸类别（如 16、32、64、128、256、512、1024、2048 字节）
2. 为每个类别分配内存块数组
3. 初始化空闲链表
4. 初始化统计计数器

分配流程：
1. 根据请求大小选择最小满足的尺寸类别
2. 获取该类别的锁
3. 从空闲链表取一个块
4. 标记为使用中
5. 写入块头部信息
6. 返回数据指针（跳过头部）

释放流程：
1. 验证指针有效性（检查魔法数）
2. 获取尺寸类别索引
3. 获取该类别的锁
4. 标记为空闲
5. 插回空闲链表
```

### 2.3 数据结构/接口
```c
// 关键常量
#define IDCU_MEM_POOL_MAX_BLOCKS        1024
#define IDCU_MEM_POOL_MAX_SIZE_CLASSES 8
#define IDCU_MEM_GUARD_SIZE             16
#define IDCU_POOL_HEADER_MAGIC          0x49444355  // "IDCU"

// 内存块头部
typedef struct {
    uint8_t  size_class;
    uint32_t block_idx;
    uint32_t magic;
} idcu_PoolBlockHeader;

// 内存块
typedef struct {
    void*    data;
    uint8_t  in_use;
    uint32_t alloc_size;
    uint8_t  size_class;
} idcu_PoolBlock;

// 尺寸类别
typedef struct {
    uint32_t        block_size;
    uint32_t        block_count;
    idcu_PoolBlock* blocks;
    uint32_t        free_count;
    uint32_t*       free_list;
    uint32_t        free_head;
    idcu_Mutex      class_lock;
} idcu_SizeClass;

// 内存池主结构体
typedef struct {
    idcu_SizeClass size_classes[IDCU_MEM_POOL_MAX_SIZE_CLASSES];
    uint32_t       num_size_classes;
    idcu_Mutex     lock;
    uint64_t       total_allocated;
    uint64_t       total_freed;
    uint64_t       peak_usage;
    uint32_t       null_check_count;
    uint32_t       overflow_check_count;
} idcu_MemoryPool;

// 核心 API
int   idcu_mem_pool_init(idcu_MemoryPool* pool);
void  idcu_mem_pool_destroy(idcu_MemoryPool* pool);
void* idcu_mem_pool_alloc(idcu_MemoryPool* pool, uint32_t size);
void  idcu_mem_pool_free(idcu_MemoryPool* pool, void* ptr);

// 统计 API
uint32_t idcu_mem_pool_get_free_count(idcu_MemoryPool* pool, uint32_t size);
uint64_t idcu_mem_pool_get_total_allocated(idcu_MemoryPool* pool);
uint64_t idcu_mem_pool_get_peak_usage(idcu_MemoryPool* pool);

// 安全检查 API
int idcu_mem_check_null(const void* ptr, const char* context);
int idcu_mem_check_overflow(const void* ptr, size_t size, const char* context);
int idcu_mem_safe_copy(void* dst, size_t dst_size, const void* src, size_t src_size);
int idcu_mem_pool_get_safety_stats(idcu_MemoryPool* pool, uint32_t* null_checks,
                                   uint32_t* overflow_checks);
```

### 2.4 跨平台适配
- 使用 idcu-common 中的跨平台互斥锁
- 内存对齐使用标准 C 库函数
- 所有代码使用标准 C 库，无平台特定 API

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以初始化和销毁内存池
- [ ] 可以分配和释放不同大小的内存块
- [ ] 自动选择合适的尺寸类别
- [ ] 线程安全：多线程并发分配/释放不崩溃
- [ ] 内存保护标记正确工作
- [ ] 空指针检查和缓冲区溢出检查正常工作
- [ ] 统计信息（空闲块数、累计分配、峰值使用）正确
- [ ] idcu_mem_pool_destroy 正确释放所有资源，无内存泄漏

### 3.2 性能验收
- 单次分配延迟 ≤ 100ns（平均值）
- 单次释放延迟 ≤ 100ns（平均值）
- 内存碎片率 ≤ 5%（长时间运行后）
- 多线程（4 线程）吞吐量 ≥ 1,000,000 次/秒
- 初始化时间 ≤ 10ms

### 3.3 异常验收
- [ ] 分配超过最大块大小时返回 NULL
- [ ] 释放无效指针时安全处理（不崩溃）
- [ ] 内存池已满时返回 NULL
- [ ] 传入 NULL 池指针时返回明确错误码

---

## 4. 执行计划

### 4.1 工期
4 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和数据结构（45 分钟）
- D1-45: 完成初始化/销毁和核心分配/释放逻辑（1.5 小时）
- D1-135: 完成统计和安全检查功能（45 分钟）
- D1-180: 完成单元测试（30 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 内存管理 + 多线程编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，结构体前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释

### 5.2 测试要求
- 单元测试覆盖率 ≥ 85%
- 测试用例覆盖：各种尺寸分配、多线程、异常场景、安全检查
- 性能测试用例验证分配/释放延迟和吞吐量

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::memory)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：多线程环境下锁竞争导致性能下降  
应对：每个尺寸类别使用独立锁，减少锁竞争范围

### 6.2 风险2
描述：内存碎片问题  
应对：使用多个尺寸类别，每个请求选择最小满足的类别

---

## 7. 详细实现步骤

### 步骤 1: 确认目录结构
```bash
# 目录结构已存在
libs/idcu-memory/
├── include/idcu/memory/
├── src/idcu/memory/
├── tests/
├── examples/
├── CMakeLists.txt
├── README.md
└── module.json
```

### 步骤 2: 确认头文件 memory_pool.h
确认 libs/idcu-memory/include/idcu/memory/memory_pool.h 中的 API 定义完整。

### 步骤 3: 确认实现文件 memory_pool.c
确认 libs/idcu-memory/src/idcu/memory/memory_pool.c 中的实现完整。

### 步骤 4: 确认 CMakeLists.txt
确认 libs/idcu-memory/CMakeLists.txt 配置正确，使用 idcu-module-build。

### 步骤 5: 确认 module.json
确认 libs/idcu-memory/module.json 元数据完整。

### 步骤 6: 确认 README.md
确认 libs/idcu-memory/README.md 文档完整。

---

## 8. 验证检查清单

- [ ] 头文件 memory_pool.h 已存在且 API 完整
- [ ] 实现文件 memory_pool.c 已存在且实现完整
- [ ] CMakeLists.txt 已存在且配置正确
- [ ] module.json 已存在
- [ ] README.md 已存在
- [ ] 可以正常编译通过
- [ ] 单元测试通过率 100%
- [ ] 性能测试达标（分配/释放 ≤ 100ns）
- [ ] 内存泄漏检测通过（Valgrind/AddressSanitizer）
- [ ] 多线程测试通过
- [ ] 跨平台测试通过（Windows + Linux）
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-memory/
git commit -m "feat: add idcu-memory library

- Add multi-size-class memory pool
- Add fast allocation and deallocation (≤100ns)
- Add thread-safe operations with per-class locks
- Add memory usage statistics (peak usage, free count)
- Add safety checks (null pointer, buffer overflow)
- Add memory guard for corruption detection
- Add CMake build configuration
- Add unit tests with 85%+ coverage"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 分配返回 NULL | 内存池已满或请求过大 | 检查尺寸类别配置，增加块数量 |
| 释放崩溃 | 释放无效指针或双重释放 | 使用安全检查函数验证指针 |
| 性能不达标 | 锁竞争严重 | 确认使用了多个尺寸类别，减少锁粒度 |
| 内存泄漏 | 忘记调用 destroy | 确保所有内存池都被正确销毁 |
| 内存碎片 | 尺寸类别不匹配 | 调整尺寸类别配置，更好地匹配分配模式 |
