# IDCU Agent 管理 API 文档

## 概述

HTTP 管理 API 提供了 RESTful 接口，用于管理 IDCU Agent 的模块和系统状态。

**默认监听地址**: `http://localhost:8080`

## API 端点

### 健康检查

#### GET /health

检查系统健康状态。

**响应示例**:
```json
{
  "status": "healthy"
}
```

### 模块管理

#### GET /api/v1/modules

获取所有已注册模块的列表。

**响应示例**:
```json
[
  {
    "id": 1,
    "name": "core_module",
    "version": "1.0.0",
    "state": "running"
  },
  {
    "id": 2,
    "name": "log_module",
    "version": "1.0.0",
    "state": "running"
  }
]
```

**模块状态**:
- `uninitialized` - 未初始化
- `initialized` - 已初始化
- `running` - 运行中
- `stopped` - 已停止
- `error` - 错误状态

## 错误响应

所有 API 端点在出错时返回以下格式的 JSON 响应:

```json
{
  "error": "错误描述信息"
}
```

**HTTP 状态码**:
- `200 OK` - 请求成功
- `400 Bad Request` - 请求参数无效
- `404 Not Found` - 资源不存在
- `500 Internal Server Error` - 服务器内部错误

## 使用示例

### 使用 curl

#### 检查健康状态
```bash
curl http://localhost:8080/health
```

#### 获取模块列表
```bash
curl http://localhost:8080/api/v1/modules
```
