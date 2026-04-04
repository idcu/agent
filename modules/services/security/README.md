# Security 服务模块

## 功能说明

安全服务模块，提供模块隔离和安全机制：
- **沙箱基础版** - 权限控制（发送、接收、运行、硬件访问等），内存区域保护，资源限制（CPU、内存、文件描述符）
- **沙箱增强版** - 系统调用白名单，增强的权限检查，更严格的资源限制

## 目录结构

```
security/
├── include/              # 头文件
│   ├── README.md
│   ├── sandbox.h
│   └── sandbox_enhanced.h
├── src/                  # 源代码
│   ├── sandbox.c
│   └── sandbox_enhanced.c
├── tests/                # 测试
│   ├── CMakeLists.txt
│   ├── test_sandbox.c
│   └── test_sandbox_enhanced.c
├── CMakeLists.txt
├── README.md
└── module.json
```

## 依赖关系

- 基础公共库
- 工具库
- 模块系统

## 使用说明

业务模块可以通过 include 目录下的头文件使用安全服务的功能。
