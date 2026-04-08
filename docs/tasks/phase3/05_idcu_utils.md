# 任务 3.5: idcu-utils - 工具库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的工具库，提供字符串处理、时间管理、编码（Base64/URL）、哈希（MD5/SHA256）、随机数生成、文件操作、环境变量、命令执行、应用信息等 10 大类工具函数，支持跨平台运行，所有 API 调用延迟 ≤ 1ms，字符串处理性能提升 20% 对比标准库。

### 1.2 不做什么
- 不实现加密解密功能（超出工具库范围）
- 不实现复杂正则表达式（使用 PCRE 库更合适）
- 不实现图形界面相关工具
- 不实现网络协议栈（独立模块）

### 1.3 输入
- 字符串数据、时间戳、文件路径
- 配置参数（如格式化字符串、分隔符）
- 命令字符串、环境变量名
- 随机数范围、哈希输入数据

### 1.4 输出
- 处理后的字符串、解析的时间值
- 编码/解码结果、哈希值
- 文件操作结果（成功/失败、大小）
- 命令输出、应用信息结构
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用
- phase2 已完成
- 标准 C 库可用

---

## 2. 技术实现方案

### 2.1 核心选型
- **字符串处理**: 纯 C 实现，避免依赖外部库
- **时间管理**: 使用系统调用（Windows GetSystemTime, Linux clock_gettime）
- **Base64**: RFC 4648 标准实现
- **URL编码**: RFC 3986 标准实现
- **MD5/SHA256**: 标准加密算法实现，不依赖 OpenSSL
- **随机数**: 使用系统熵源（Windows CryptGenRandom, Linux /dev/urandom）
- **文件操作**: 跨平台封装（Windows CreateFile, Linux open）

### 2.2 核心逻辑
```
初始化（按需）：
1. 初始化随机数生成器种子
2. 无全局状态，所有函数无状态

字符串处理流程：
1. 输入验证（空指针检查）
2. 就地修改或新分配内存
3. 返回状态码或结果指针

哈希计算流程：
1. 初始化上下文
2. 分块更新数据
3. 最终计算并输出摘要

文件操作流程：
1. 路径有效性检查
2. 系统调用
3. 错误码转换为统一错误码
```

### 2.3 数据结构/接口
```c
// 主要头文件：idcu/utils/utils.h

// 字符串工具
int  idcu_str_trim(char* str);
int  idcu_str_trim_left(char* str);
int  idcu_str_trim_right(char* str);
char* idcu_str_dup(const char* str);
char* idcu_str_ndup(const char* str, size_t n);
int  idcu_str_split(const char* str, char delimiter, char** parts, size_t max_parts, size_t* count);

// 时间工具
uint64_t idcu_time_now_ms(void);
uint64_t idcu_time_now_us(void);
uint64_t idcu_time_now_ns(void);
int  idcu_time_sleep_ms(uint64_t ms);
int  idcu_time_format(uint64_t timestamp_ms, const char* format, char* buffer, size_t buffer_size);
int  idcu_time_format_iso8601(uint64_t timestamp_ms, char* buffer, size_t buffer_size);

// 编码工具
int  idcu_base64_encode(const uint8_t* data, size_t data_len, char* output, size_t* output_len);
int  idcu_base64_decode(const char* input, uint8_t* output, size_t* output_len);
int  idcu_url_encode(const char* str, char* output, size_t* output_len);
int  idcu_url_decode(const char* input, char* output, size_t* output_len);

// 哈希工具
typedef struct idcu_MD5Context { ... } idcu_MD5Context;
typedef struct idcu_SHA256Context { ... } idcu_SHA256Context;
int  idcu_md5(const uint8_t* data, size_t len, uint8_t digest[16]);
int  idcu_sha256(const uint8_t* data, size_t len, uint8_t digest[32]);
int  idcu_md5_file(const char* path, uint8_t digest[16]);
int  idcu_sha256_file(const char* path, uint8_t digest[32]);

// 随机数
void idcu_random_seed(uint64_t seed);
uint32_t idcu_random_uint32(void);
int  idcu_random_int(int min, int max);
int  idcu_random_uuid(char* buffer, size_t buffer_size);

// 文件工具
int  idcu_file_exists(const char* path);
int64_t idcu_file_size(const char* path);
int  idcu_file_read(const char* path, char* buffer, size_t buffer_size, size_t* read_len);
int  idcu_file_write(const char* path, const void* data, size_t len);
int  idcu_file_mkdir(const char* path, int recursive);
int  idcu_file_list(const char* path, char** files, size_t max_files, size_t* count);

// 环境变量
int  idcu_env_get(const char* name, char* buffer, size_t buffer_size);
int  idcu_env_set(const char* name, const char* value, int overwrite);
int  idcu_env_get_int(const char* name, int64_t* value, int64_t default_value);

// 命令执行
int  idcu_exec(const char* cmd, char* output, size_t output_size, int* exit_code);
int  idcu_exec_async(const char* cmd, void** handle);
int  idcu_exec_wait(void* handle, char* output, size_t output_size, int* exit_code);

// 应用信息
typedef struct { char path[1024]; char name[256]; ... } idcu_AppInfo;
int  idcu_app_info_init(idcu_AppInfo* info);
int  idcu_app_info_get_exe_path(char* buffer, size_t buffer_size);
int  idcu_app_info_get_data_dir(char* buffer, size_t buffer_size);
```

### 2.4 跨平台适配
- **Windows**: 使用 Win32 API（GetSystemTime, CreateFile, CryptGenRandom）
- **Linux**: 使用 POSIX API（clock_gettime, open, /dev/urandom）
- **路径分隔符**: 自动处理 \ 和 /
- **命令执行**: Windows 使用 CreateProcess，Linux 使用 fork/exec

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 字符串工具（trim, split, join, format）全部可用
- [ ] 时间工具（获取当前时间、格式化、睡眠）正常工作
- [ ] Base64 和 URL 编码/解码正确
- [ ] MD5 和 SHA256 哈希计算正确（与 OpenSSL 结果一致）
- [ ] 随机数生成（整数、UUID）分布均匀
- [ ] 文件工具（存在检查、读写、目录操作）正常
- [ ] 环境变量读取和设置正常
- [ ] 命令执行（同步和异步）正常工作
- [ ] 应用信息获取（exe路径、数据目录）正确
- [ ] 所有 API 正确处理 NULL 指针，无崩溃

### 3.2 性能验收
- 单次字符串操作（trim）≤ 100ns（1KB 字符串）
- Base64 编码吞吐量 ≥ 100MB/s
- SHA256 哈希吞吐量 ≥ 200MB/s
- 随机数生成 ≥ 10,000,000 UUID/s
- 文件读取吞吐量 ≥ 500MB/s
- 跨平台调用开销 ≤ 5%

### 3.3 异常验收
- [ ] 传入 NULL 指针返回明确错误码
- [ ] 缓冲区溢出时截断并返回错误
- [ ] 文件不存在时返回正确错误码
- [ ] 命令执行失败时返回 exit code 和错误信息
- [ ] 编码无效输入时返回错误而非崩溃

---

## 4. 执行计划

### 4.1 工期
6 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和数据结构（1 小时）
- D1-60: 完成字符串工具和时间工具（1.5 小时）
- D1-150: 完成编码、哈希、随机数工具（1.5 小时）
- D1-240: 完成文件、环境变量、命令执行、应用信息（1 小时）
- D1-300: 完成单元测试（30 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 跨平台开发）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释
- 内部函数使用 static 修饰

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 每个工具类别至少 3 个测试用例
- 性能测试验证吞吐量和延迟
- 跨平台测试（Windows + Linux）

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::utils)`
- Windows 需链接 advapi32 和 shell32

---

## 6. 风险与应对

### 6.1 风险1
描述：跨平台行为差异导致兼容性问题  
应对：编写完整的跨平台测试用例，持续集成验证

### 6.2 风险2
描述：随机数生成器安全性不足  
应对：使用系统级熵源，不依赖 rand()，提供安全种子设置

---

## 7. 详细实现步骤

### 1. 创建目录结构
```bash
mkdir -p libs/idcu-utils/include/idcu/utils
mkdir -p libs/idcu-utils/src/idcu/utils
mkdir -p libs/idcu-utils/tests
mkdir -p libs/idcu-utils/examples
```

### 2. 创建工具头文件 (utils.h)
创建 `libs/idcu-utils/include/idcu/utils/utils.h`，包含所有 API 声明。

### 3. 创建 CMakeLists.txt
创建 `libs/idcu-utils/CMakeLists.txt`。

### 4. 创建模块配置文件 (module.yaml)
创建 `libs/idcu-utils/module.yaml`。

### 5. 创建实现文件
按功能模块创建实现文件：
- string.c: 字符串工具
- time.c: 时间工具
- base64.c: Base64 编码
- md5.c: MD5 哈希
- sha256.c: SHA256 哈希
- random.c: 随机数
- file.c: 文件工具
- env.c: 环境变量
- exec.c: 命令执行
- utils.c: 应用信息

### 6. 创建 README.md
创建 `libs/idcu-utils/README.md` 文档。

---

## 8. 验证检查清单

- [ ] 工具头文件已创建
- [ ] 所有工具实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 字符串工具测试通过
- [ ] 时间工具测试通过
- [ ] Base64/URL 编码测试通过
- [ ] MD5/SHA256 哈希测试通过
- [ ] 随机数生成测试通过
- [ ] 文件工具测试通过
- [ ] 环境变量工具测试通过
- [ ] 命令执行测试通过
- [ ] 应用信息获取测试通过
- [ ] 跨平台测试通过（Windows + Linux）
- [ ] 性能测试达标
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-utils/
git commit -m "feat: add idcu-utils library

- Add string utilities (trim, split, join, format)
- Add time utilities (timestamp, format, sleep)
- Add Base64 and URL encoding/decoding
- Add MD5 and SHA256 hashing
- Add random number and UUID generation
- Add file utilities (read, write, directory operations)
- Add environment variable utilities
- Add command execution (sync/async)
- Add application information utilities
- Add CMake build configuration
- Add module.yaml metadata
- Add comprehensive unit tests"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 字符串处理错误 | 缓冲区溢出 | 使用足够大的缓冲区，检查返回值 |
| 时间解析错误 | 格式不匹配 | 确保使用正确的格式字符串 |
| 文件操作失败 | 权限问题 | 检查文件权限和路径有效性 |
| 随机数不安全 | 使用了错误的种子 | 使用 idcu_random_seed() 或让库自动初始化 |
| 命令执行无输出 | 缓冲区太小 | 增大输出缓冲区大小 |
| 跨平台路径错误 | 路径分隔符问题 | 使用 idcu_file_join_path() 处理路径拼接 |
