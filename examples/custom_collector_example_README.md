# 自定义数据采集模块示例

## 功能说明

本示例演示如何使用IDCU SDK开发一个完整的自定义数据采集模块，包括数据采集、存储、统计和上报功能。

### 主要功能
- 定时数据采集（可配置间隔）
- 循环缓冲区数据存储
- 实时统计计算（最小值、最大值、平均值、总和）
- SDK日志集成
- 内存管理

## 编译方式

### 作为独立模块编译到IDCU Agent

1. 将 `custom_collector_example.c` 复制到 `modules/business/custom_collector/src/` 目录
2. 创建对应的 `CMakeLists.txt`（参考 `modules/business/collect/CMakeLists.txt`）
3. 在根目录的 `CMakeLists.txt` 中添加该模块
4. 运行编译脚本：
   - Windows: `scripts/build.bat`
   - Linux: `scripts/build.sh`

### 作为独立程序运行（演示用）

```bash
gcc -o custom_collector_example custom_collector_example.c -I./modules/core/sdk/include -I./libs/idcu-log/include
./custom_collector_example
```

## 运行步骤

### 在IDCU Agent中运行

1. 确保模块已编译到IDCU Agent中
2. 在 `config/agent.cfg` 中启用模块：
```ini
[modules]
business = ..., custom_collector

enable_custom_collector = true
custom_collector.priority = normal
```
3. 运行IDCU Agent：
   - Windows: `out/idcu_agent.exe`
   - Linux: `out/idcu_agent`

### 查看输出

模块会输出类似以下日志：
```
[custom_collector] Initializing custom collector module
[custom_collector] Custom collector module initialized
[custom_collector] Collected data: 42.56 (total: 1)
[custom_collector] Collected data: 78.12 (total: 2)
...
```

## 核心代码解读

### 1. 数据结构定义

```c
typedef struct {
    int64_t timestamp;
    double value;
} DataPoint;

typedef struct {
    DataPoint data_points[MAX_DATA_POINTS];
    int data_count;
    double sum;
    double min;
    double max;
    int collect_count;
    int64_t last_collect_time;
} CollectorData;
```

- `DataPoint`: 存储单个数据点，包含时间戳和值
- `CollectorData`: 模块状态，包含数据缓冲区和统计信息

### 2. 模块初始化函数

```c
static int collector_init(idcu_SdkContext* ctx) {
    CollectorData* data = (CollectorData*)malloc(sizeof(CollectorData));
    // 初始化数据结构
    // 设置用户数据
    idcu_sdk_set_user_data(ctx, data);
    return IDCU_ERR_OK;
}
```

- 分配并初始化模块数据
- 使用 `idcu_sdk_set_user_data` 存储上下文
- 初始化随机数生成器

### 3. 数据采集函数

```c
static void collect_data(idcu_SdkContext* ctx, CollectorData* data) {
    // 检查采集间隔
    // 生成随机数据
    // 存储到循环缓冲区
    // 更新统计信息
    // 记录日志
}
```

- 使用循环缓冲区存储最新的数据点
- 实时更新统计数据
- 通过SDK记录日志

### 4. 统计计算

```c
static void update_statistics(CollectorData* data, double value) {
    data->sum += value;
    if (data->data_count == 0 || value < data->min) {
        data->min = value;
    }
    if (data->data_count == 0 || value > data->max) {
        data->max = value;
    }
}
```

- 累计总和
- 跟踪最小值和最大值
- 为平均值计算提供基础数据

### 5. 模块注册

```c
IDCU_SDK_MODULE_DEFINE(
    custom_collector,
    "1.0.0",
    "A custom data collector module example",
    collector_init,
    collector_start,
    collector_stop,
    collector_destroy
);
```

使用 `IDCU_SDK_MODULE_DEFINE` 宏注册模块，提供：
- 模块名称
- 版本号
- 描述
- 生命周期函数指针

## 扩展建议

1. **真实数据源**: 替换 `get_random_value()` 函数，从真实传感器或API获取数据
2. **数据持久化**: 添加将数据保存到文件或数据库的功能
3. **数据上报**: 通过HTTP或消息总线将数据发送到远程服务器
4. **配置支持**: 添加配置文件支持，允许动态调整采集间隔
5. **数据导出**: 支持将统计数据导出为JSON或CSV格式

## 依赖关系

- `idcu-core-sdk`: SDK核心功能
- `idcu-log`: 日志系统
- `idcu-common`: 通用工具和错误码
