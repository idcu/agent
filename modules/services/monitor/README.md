# Monitor 服务模块

## 功能说明

监控服务模块，提供完整的监控和可观测性功能：
- **健康检查** - 模块健康状态检测和管理
- **指标收集** - 性能指标收集、分类和查询
- **告警管理** - 告警规则定义、条件判断和状态管理
- **通知器** - 告警通知发送，支持多渠道
- **Prometheus 导出** - Prometheus 格式指标导出和 HTTP 端点

## 目录结构

```
monitor/
├── include/              # 头文件
│   ├── README.md
│   ├── alert_manager.h
│   ├── health_check.h
│   ├── metrics.h
│   ├── notifier.h
│   └── prometheus_exporter.h
├── src/                  # 源代码
│   ├── alert_manager.c
│   ├── health_check.c
│   ├── metrics.c
│   ├── notifier.c
│   └── prometheus_exporter.c
├── tests/                # 测试
│   ├── CMakeLists.txt
│   ├── test_health_check.c
│   └── test_metrics.c
├── CMakeLists.txt
├── README.md
└── module.json
```

## 依赖关系

- 基础公共库
- 工具库
- 模块系统

## 使用说明

业务模块可以通过 include 目录下的头文件使用监控服务的功能。
