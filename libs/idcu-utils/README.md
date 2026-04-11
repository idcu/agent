# idcu-utils

工具库 for IDCU Agent.

## 功能特性

### 字符串工具
- `idcu_str_trim()` - 去除字符串两端空白字符
- `idcu_str_trim_left()` - 去除字符串左端空白字符
- `idcu_str_trim_right()` - 去除字符串右端空白字符
- `idcu_str_dup()` / `idcu_str_ndup()` - 字符串复制
- `idcu_str_split()` - 字符串分割
- `idcu_str_starts_with()` / `idcu_str_ends_with()` - 前缀/后缀检查
- `idcu_str_replace()` - 字符串替换

### 时间工具
- `idcu_time_now_ms()` / `idcu_time_now_us()` / `idcu_time_now_ns()` - 获取当前时间戳
- `idcu_time_sleep_ms()` - 睡眠指定毫秒
- `idcu_time_format()` / `idcu_time_format_iso8601()` - 时间格式化

### 编码工具
- `idcu_base64_encode()` / `idcu_base64_decode()` - Base64编码/解码
- `idcu_url_encode()` / `idcu_url_decode()` - URL编码/解码

### 哈希工具
- `idcu_md5()` / `idcu_sha256()` - MD5/SHA256哈希计算
- 流式哈希接口（init/update/final）

### 随机数
- `idcu_random_seed()` - 设置随机数种子
- `idcu_random_uint32()` / `idcu_random_int()` - 生成随机数
- `idcu_random_uuid()` - 生成UUID

### 文件工具
- `idcu_file_exists()` - 检查文件是否存在
- `idcu_file_size()` - 获取文件大小
- `idcu_file_read()` / `idcu_file_read_all()` - 读取文件
- `idcu_file_write()` - 写入文件
- `idcu_file_delete()` - 删除文件
- `idcu_file_mkdir()` - 创建目录

### 环境变量
- `idcu_env_get()` / `idcu_env_set()` / `idcu_env_unset()` - 环境变量操作
- `idcu_env_get_int()` - 获取整数型环境变量

### 应用信息
- `idcu_app_info_init()` - 初始化应用信息
- `idcu_app_info_get_exe_path()` / `idcu_app_info_get_exe_name()` - 获取可执行文件信息
- `idcu_app_info_get_data_dir()` - 获取数据目录

## 使用方法

```c
#include <idcu/utils/utils.h>

// 字符串工具
char str[] = "  Hello World!  ";
idcu_str_trim(str);

// 时间工具
uint64_t now = idcu_time_now_ms();

// Base64编码
uint8_t data[] = {0x01, 0x02, 0x03};
char encoded[256];
size_t len = sizeof(encoded);
idcu_base64_encode(data, sizeof(data), encoded, &len);

// 随机UUID
char uuid[IDCU_UUID_STRING_SIZE];
idcu_random_uuid(uuid, sizeof(uuid));
```

## 跨平台支持

- Windows: 使用 Win32 API
- Linux: 使用 POSIX API
- 自动处理路径分隔符差异

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
