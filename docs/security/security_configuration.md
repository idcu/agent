# IDCU Agent 安全配置建议

## 概述

本文档提供了 IDCU Agent 部署和运行时的安全配置建议，帮助您构建更安全的应用程序。

## 构建配置

### 1. 编译器选项

#### 推荐的编译标志

```cmake
# 在 CMakeLists.txt 或构建脚本中使用
if(NOT MSVC)
    # GCC/Clang 编译器
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror  # 将警告视为错误（推荐在开发环境中使用）
        -fstack-protector-strong
        -D_FORTIFY_SOURCE=2
        -O2  # 优化级别，不推荐 -O0 在生产环境
    )
endif()
```

#### MSVC 编译器选项

```cmake
if(MSVC)
    add_compile_options(
        /W4
        /WX  # 将警告视为错误
        /sdl  # 启用安全开发周期检查
        /GS  # 缓冲区安全检查
        /guard:cf  # 控制流保护
    )
endif()
```

### 2. 调试与发布配置

#### 开发环境配置

```bash
# Debug 构建，启用 Sanitizer
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DENABLE_ASAN=ON \
      -DENABLE_UBSAN=ON \
      ..
make
```

#### 测试环境配置

```bash
# Release 构建，但保留调试信息和 Sanitizer
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DENABLE_ASAN=ON \
      -DENABLE_TSAN=ON \
      -DENABLE_UBSAN=ON \
      ..
make
make test
```

#### 生产环境配置

```bash
# Release 构建，优化性能，禁用调试功能
cmake -DCMAKE_BUILD_TYPE=Release \
      -DENABLE_ASAN=OFF \
      -DENABLE_TSAN=OFF \
      -DENABLE_UBSAN=OFF \
      -DENABLE_MSAN=OFF \
      ..
make
```

## 运行时配置

### 1. 内存配置

#### 内存池配置

```c
// 根据应用需求配置内存池
#define POOL_INITIAL_SIZE (10 * 1024 * 1024)  // 10MB
#define POOL_MAX_SIZE (100 * 1024 * 1024)     // 100MB

idcu_memory_pool_t* pool = idcu_memory_pool_create(POOL_INITIAL_SIZE);
if (pool == NULL) {
    // 处理错误
}
```

#### 内存调试配置（调试模式）

```c
// 在调试模式下启用详细的内存跟踪
#ifdef DEBUG
idcu_memory_set_debug_level(IDCU_MEMORY_DEBUG_FULL);
idcu_memory_set_leak_check(true);
#endif
```

### 2. 日志配置

#### 生产环境日志

```c
// 生产环境：只记录重要信息，不记录敏感数据
idcu_log_set_level(IDCU_LOG_LEVEL_WARN);
idcu_log_set_output(IDCU_LOG_OUTPUT_FILE);
idcu_log_set_file_path("/var/log/idcu-agent/app.log");
```

#### 开发环境日志

```c
// 开发环境：记录详细信息
idcu_log_set_level(IDCU_LOG_LEVEL_DEBUG);
idcu_log_set_output(IDCU_LOG_OUTPUT_CONSOLE | IDCU_LOG_OUTPUT_FILE);
```

### 3. 网络配置

#### 连接超时设置

```c
// 设置合理的超时值
#define CONNECT_TIMEOUT 5000   // 5 秒
#define READ_TIMEOUT 30000     // 30 秒
#define WRITE_TIMEOUT 30000    // 30 秒

idcu_network_socket_t sock;
idcu_network_set_timeout(&sock, CONNECT_TIMEOUT, READ_TIMEOUT, WRITE_TIMEOUT);
```

#### 连接限制

```c
// 限制最大连接数
#define MAX_CONNECTIONS 1000

// 实现连接计数器
static volatile int connection_count = 0;

bool can_accept_connection() {
    return connection_count < MAX_CONNECTIONS;
}
```

## 部署配置

### 1. 操作系统安全加固

#### Linux 系统

```bash
# 使用非 root 用户运行
useradd -r -s /sbin/nologin idcu-agent

# 设置适当的文件权限
chown -R idcu-agent:idcu-agent /opt/idcu-agent
chmod -R 750 /opt/idcu-agent
chmod 640 /opt/idcu-agent/config/*.yaml

# 使用 systemd 服务管理
cat > /etc/systemd/system/idcu-agent.service << EOF
[Unit]
Description=IDCU Agent Service
After=network.target

[Service]
Type=simple
User=idcu-agent
Group=idcu-agent
ExecStart=/opt/idcu-agent/bin/idcu-agent
Restart=always
RestartSec=5
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=/opt/idcu-agent/data
ReadWritePaths=/var/log/idcu-agent

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable idcu-agent
```

#### Windows 系统

```powershell
# 创建服务用户
New-LocalUser -Name "IDCUAgent" -NoPassword

# 设置文件权限
$acl = Get-Acl "C:\Program Files\IDCU Agent"
$rule = New-Object System.Security.AccessControl.FileSystemAccessRule(
    "IDCUAgent",
    "ReadAndExecute",
    "ContainerInherit,ObjectInherit",
    "None",
    "Allow"
)
$acl.SetAccessRule($rule)
Set-Acl "C:\Program Files\IDCU Agent" $acl

# 使用 Windows 服务
# 可以使用 sc.exe 或 New-Service 创建服务
```

### 2. 容器安全配置

#### Dockerfile 安全最佳实践

```dockerfile
# 使用多阶段构建
FROM debian:bookworm-slim AS builder

# 安装构建依赖
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# 构建应用
WORKDIR /build
COPY . .
RUN cmake -DCMAKE_BUILD_TYPE=Release -B build && \
    cmake --build build -j$(nproc)

# 运行时镜像
FROM debian:bookworm-slim

# 创建非 root 用户
RUN useradd -r -s /sbin/nologin idcu-agent

# 只复制必要的文件
COPY --from=builder /build/build/bin /opt/idcu-agent/bin
COPY --from=builder /build/config /opt/idcu-agent/config

# 设置权限
RUN chown -R idcu-agent:idcu-agent /opt/idcu-agent && \
    chmod -R 750 /opt/idcu-agent

# 切换到非 root 用户
USER idcu-agent

# 健康检查
HEALTHCHECK --interval=30s --timeout=3s \
    CMD curl -f http://localhost:8080/health || exit 1

# 暴露端口
EXPOSE 8080

# 启动命令
WORKDIR /opt/idcu-agent
CMD ["./bin/idcu-agent"]
```

#### Docker Compose 配置

```yaml
version: '3.8'

services:
  idcu-agent:
    build: .
    restart: unless-stopped
    user: "1000:1000"
    read_only: true
    tmpfs:
      - /tmp
      - /opt/idcu-agent/data
    volumes:
      - ./config:/opt/idcu-agent/config:ro
      - ./logs:/opt/idcu-agent/logs:rw
    ports:
      - "8080:8080"
    security_opt:
      - no-new-privileges:true
    cap_drop:
      - ALL
    deploy:
      resources:
        limits:
          cpus: '0.5'
          memory: 256M
```

## 监控和审计配置

### 1. 日志监控

#### 使用日志分析工具

```c
// 启用结构化日志
idcu_log_set_format(IDCU_LOG_FORMAT_JSON);

// 集成到日志收集系统
// 例如：ELK Stack, Promtail + Loki, etc.
```

### 2. 健康检查配置

```c
// 配置健康检查
idcu_healthcheck_add_check(
    "memory",
    check_memory_usage,
    30  // 30 秒间隔
);

idcu_healthcheck_add_check(
    "network",
    check_network_connectivity,
    60  // 60 秒间隔
);

// 健康检查 HTTP 端点
// GET /health 返回 200 OK 或 503 Service Unavailable
```

### 3. 指标收集

```c
// 配置 Prometheus 指标导出
idcu_metrics_register_counter("requests_total", "Total number of requests");
idcu_metrics_register_gauge("memory_usage_bytes", "Current memory usage");
idcu_metrics_register_histogram("request_duration_seconds", "Request duration");

// HTTP 端点导出指标
// GET /metrics 返回 Prometheus 格式的指标
```

## 配置文件安全

### 1. 配置文件权限

```bash
# 设置配置文件权限
chmod 600 config/production.yaml
chown idcu-agent:idcu-agent config/production.yaml
```

### 2. 敏感信息处理

#### 使用环境变量

```bash
# 不要在配置文件中存储敏感信息
export IDCU_DATABASE_PASSWORD="your-secure-password"
export IDCU_API_KEY="your-api-key"
```

#### 使用密钥管理系统

```c
// 从密钥管理系统获取敏感信息
// 例如：HashiCorp Vault, AWS Secrets Manager, etc.
char* db_password = get_secret_from_vault("db_password");
```

## 定期维护

### 1. 依赖更新

```bash
# 定期检查和更新依赖
# 使用包管理器或依赖扫描工具
```

### 2. 安全审计

```bash
# 定期运行安全扫描
# 使用 Sanitizer, 静态分析工具, 漏洞扫描器
```

### 3. 备份策略

```bash
# 配置自动备份
# 备份配置文件、数据和日志
```

## 总结

这些安全配置建议涵盖了从构建到部署的整个生命周期。根据您的具体需求和环境，选择适当的配置选项，并定期审查和更新安全配置。

安全是一个持续的过程，需要持续关注和改进。

