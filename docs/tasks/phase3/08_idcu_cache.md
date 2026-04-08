# 任务 3.8: idcu-cache - 内存缓存库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建内存缓存库，提供 LRU/LFU/FIFO/随机 4 种淘汰策略、TTL 过期支持、最大容量和内存限制、命中率统计、线程安全操作，满足缓存读写延迟 ≤ 100ns、支持 100000+ 缓存项、命中率 ≥ 80% 的性能要求。

### 1.2 不做什么
- 不实现持久化缓存（由 storage 模块处理）
- 不实现分布式缓存
- 不实现缓存预热
- 不实现压缩存储

### 1.3 输入
- 缓存配置（策略、容量、TTL）
- 缓存键值对
- TTL 时间（毫秒）
- 淘汰策略

### 1.4 输出
- 缓存读取结果
- 统计信息（命中/未命中/淘汰计数、命中率）
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **淘汰策略**: LRU（默认）、LFU、FIFO、随机
- **数据结构**: 双向链表 + 哈希表（LRU）
- **过期管理**: 按 TTL 时间戳 + 惰性过期检查
- **线程安全**: 互斥锁保护
- **统计**: 原子计数器

### 2.2 核心逻辑
```
缓存初始化：
1. 根据配置初始化数据结构
2. 初始化互斥锁
3. 清零统计计数器
4. 设置默认淘汰策略

缓存写入：
1. 获取锁
2. 检查键是否已存在（更新）
3. 容量满时按策略淘汰
4. 插入或更新缓存项
5. 更新访问时间/计数
6. 释放锁

缓存读取：
1. 获取锁
2. 查找键
3. 检查是否过期
4. 更新访问时间/计数
5. 更新统计（命中/未命中）
6. 释放锁
```

### 2.3 数据结构/接口
```c
// 主要头文件：idcu/cache/cache.h

typedef enum {
    IDCU_CACHE_POLICY_LRU = 0,
    IDCU_CACHE_POLICY_LFU,
    IDCU_CACHE_POLICY_FIFO,
    IDCU_CACHE_POLICY_RANDOM
} idcu_CachePolicy;

typedef struct idcu_CacheEntry {
    char* key;
    void* value;
    size_t value_size;
    uint64_t created_at;
    uint64_t last_accessed_at;
    uint64_t access_count;
    uint64_t expire_at;
    struct idcu_CacheEntry* prev;
    struct idcu_CacheEntry* next;
} idcu_CacheEntry;

typedef struct {
    idcu_HashMap entries;
    idcu_CacheEntry* head;
    idcu_CacheEntry* tail;
    idcu_Mutex lock;
    idcu_CachePolicy policy;
    size_t max_entries;
    size_t current_entries;
    size_t max_memory;
    size_t current_memory;
    uint64_t default_ttl_ms;
    uint64_t hit_count;
    uint64_t miss_count;
    uint64_t evict_count;
} idcu_Cache;

int  idcu_cache_init(idcu_Cache* cache, const idcu_CacheConfig* config);
void idcu_cache_destroy(idcu_Cache* cache);
int  idcu_cache_put(idcu_Cache* cache, const char* key, const void* value, size_t value_size);
int  idcu_cache_get(idcu_Cache* cache, const char* key, void* buffer, size_t buffer_size, size_t* value_size);
int  idcu_cache_remove(idcu_Cache* cache, const char* key);
int  idcu_cache_clear(idcu_Cache* cache);
double idcu_cache_get_hit_rate(const idcu_Cache* cache);
```

### 2.4 跨平台适配
- 使用 idcu-common 中的跨平台互斥锁和哈希表
- 时间获取使用跨平台 API

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] LRU/LFU/FIFO/随机策略全部正常工作
- [ ] TTL 过期正确执行
- [ ] 容量和内存限制正确生效
- [ ] 命中率统计准确
- [ ] 多线程并发安全

### 3.2 性能验收
- 缓存读写延迟 ≤ 100ns（平均）
- 支持 100000+ 缓存项
- 命中率 ≥ 80%（正常工作负载）
- 内存占用可控

### 3.3 异常验收
- [ ] 传入 NULL 指针返回明确错误码
- [ ] 容量满时正确淘汰

---

## 4. 执行计划

### 4.1 工期
4 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义（30 分钟）
- D1-30: 完成核心数据结构和 LRU（1 小时）
- D1-90: 完成其他策略和 TTL（1 小时）
- D1-150: 完成统计和线程安全（45 分钟）
- D1-195: 完成单元测试（45 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 数据结构）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，前缀 idcu_

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试覆盖所有 4 种策略

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::cache)`

---

## 6. 风险与应对

### 6.1 风险1
描述：锁争用导致多线程性能下降  
应对：考虑分段锁或无锁数据结构

### 6.2 风险2
描述：内存碎片  
应对：使用内存池或固定大小分配

---

## 7. 详细实现步骤

### 1. 创建目录结构
```bash
mkdir -p libs/idcu-cache/include/idcu/cache
mkdir -p libs/idcu-cache/src/idcu/cache
mkdir -p libs/idcu-cache/tests
mkdir -p libs/idcu-cache/examples
```

### 2. 创建头文件和实现
- cache.h: 头文件定义
- cache.c: 核心实现

### 3. 创建 CMakeLists.txt 和 module.yaml

### 4. 创建 README.md

---

## 8. 验证检查清单

- [ ] 缓存头文件已创建
- [ ] 缓存实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 已创建
- [ ] README.md 已创建
- [ ] 所有策略测试通过
- [ ] TTL 功能正常
- [ ] 统计功能正常
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-cache/
git commit -m "feat: add idcu-cache library

- Add LRU/LFU/FIFO/Random cache policies
- Add TTL (time-to-live) support
- Add max capacity and memory limits
- Add hit/miss/evict statistics
- Add thread-safe operations
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 缓存频繁失效 | TTL 设置太短 | 增加 TTL 时间 |
| 内存占用过高 | max_memory 设置太大 | 减少 max_memory 或 max_entries |
| 命中率低 | 缓存策略不适合 | 尝试不同的缓存策略 |
