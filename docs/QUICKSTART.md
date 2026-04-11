# IDCU Agent 快速入门指南

本指南将帮助您快速开始使用 IDCU Agent 项目。

## 前提条件

- CMake 3.15 或更高版本
- 一个 C 编译器（GCC、Clang 或 MSVC）
- Git（可选，用于克隆仓库）

## 构建项目

### 步骤 1：克隆或下载项目

```bash
git clone <repository-url>
cd idcu-agent
```

### 步骤 2：创建构建目录

```bash
mkdir build
cd build
```

### 步骤 3：使用 CMake 配置

#### Windows (MSVC)
```bash
cmake .. -G "Visual Studio 17 2022" -A x64
```

#### Windows (MinGW)
```bash
cmake .. -G "MinGW Makefiles"
```

#### Linux (GCC)
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

#### Linux (Clang)
```bash
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Release
```

### 步骤 4：构建

```bash
# Windows (Visual Studio)
cmake --build . --config Release

# Linux/Mac (Make)
make -j4
```

### 步骤 5：运行应用程序

```bash
# 构建成功后
./app/idcu-agent --version
```

## 项目结构

```
idcu-agent/
├── app/                    # 主应用程序
├── libs/                   # 核心库
│   ├── idcu-common/       # 通用工具
│   ├── idcu-log/          # 日志系统
│   ├── idcu-module-system/# 模块系统
│   ├── idcu-coroutine/    # 协程调度器
│   ├── idcu-msgbus/       # 消息总线
│   ├── idcu-microkernel/  # 微内核
│   ├── idcu-network/      # 网络库
│   ├── idcu-http-server/  # HTTP 服务器
│   ├── idcu-http-client/  # HTTP 客户端
│   ├── idcu-config/       # 配置
│   ├── idcu-metrics/      # 指标
│   ├── idcu-healthcheck/  # 健康检查
│   └── ...                # 更多库
├── modules/                # 业务模块
├── tests/                  # 测试
├── docs/                   # 文档
└── CMakeLists.txt          # 主构建文件
```

## 第一个程序：使用日志库

创建文件 `hello_log.c`：

```c
#include <idcu/log/log.h>

int main() {
    // 初始化日志
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    IDCU_LOG_INFO("你好, IDCU Agent!");
    IDCU_LOG_WARN("这是一个警告");
    
    // 清理
    idcu_log_shutdown();
    
    return 0;
}
```

## 构建您自己的程序

创建 `CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(my_program)

# 查找或包含 IDCU 库
add_executable(my_program hello_log.c)
target_link_libraries(my_program PRIVATE idcu::log)
```

## 使用配置

```c
#include <idcu/config/config.h>

int main() {
    idcu_Config* config = NULL;
    idcu_config_init(&config, "app.ini");
    
    const char* host = idcu_config_get_string(config, "database", "host", "localhost");
    int port = idcu_config_get_int(config, "database", "port", 5432);
    
    printf("连接到 %s:%d\n", host, port);
    
    idcu_config_destroy(config);
    return 0;
}
```

## 使用指标

```c
#include <idcu/metrics/metrics.h>

int main() {
    idcu_MetricsRegistry* registry = NULL;
    idcu_metrics_registry_init(&registry);
    
    // 创建一个计数器
    idcu_Metric* requests = idcu_metrics_counter_create(
        registry, "http_requests_total", "总 HTTP 请求数"
    );
    
    idcu_metrics_counter_inc(requests);
    
    // 导出为 Prometheus 格式
    char* prom = idcu_metrics_export_prometheus(registry);
    printf("%s", prom);
    free(prom);
    
    idcu_metrics_registry_destroy(registry);
    return 0;
}
```

## 运行测试

构建和运行测试：

```bash
cd build
cmake .. -DBUILD_TESTS=ON
cmake --build .
ctest --output-on-failure
```

## 下一步

- 阅读 [API 文档](api/README.md) 了解详细的库使用方法
- 查看 [examples](examples/) 目录获取更多代码示例
- 查看 [开发计划](DEVELOPMENT_PLAN.md) 了解项目路线图
- 查看 [任务](tasks/README.md) 了解贡献指南

## 获取帮助

- 查看 `docs/` 目录中的文档
- 查看 `libs/*/include/` 中的头文件了解 API 定义
- 查看 `libs/*/src/` 中的源代码了解实现细节

## 许可证

IDCU Agent 采用 MIT 许可证发布。
