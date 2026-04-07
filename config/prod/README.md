# 生产环境配置

本目录包含 IDCU Agent 的生产环境配置文件。

## 环境说明

生产环境配置适用于：
- 正式生产部署
- 高可用性要求的场景
- 需要监控和告警的环境

## 配置特点

- **精简日志**：`log_level = warn`，只输出警告及以上级别，减少 I/O
- **配置验证**：`config_validate = true`，确保配置正确
- **完整告警**：启用所有告警通道（邮件、短信、钉钉等）
- **资源严格**：较低的资源限制，防止资源滥用
- **性能优先**：优化配置，追求最佳性能和稳定性

## 使用方式

### 方式 1：系统服务

```bash
# 使用 systemd 管理（Linux）
cat > /etc/systemd/system/idcu-agent.service << EOF
[Unit]
Description=IDCU Agent
After=network.target

[Service]
Type=simple
User=idcu
Group=idcu
ExecStart=/opt/idcu/bin/idcu_agent --config /opt/idcu/config/prod/agent.cfg
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable idcu-agent
systemctl start idcu-agent
```

### 方式 2：容器部署

```dockerfile
# Dockerfile 示例
FROM alpine:latest
COPY build/idcu_agent /app/idcu_agent
COPY config/prod/agent.cfg /app/config/agent.cfg
CMD ["/app/idcu_agent", "--config", "/app/config/agent.cfg"]
```

### 方式 3：环境变量替换

生产环境敏感信息应通过环境变量注入：

```bash
# 设置敏感信息
export SMS_ACCESS_KEY_ID="your-access-key-id"
export SMS_ACCESS_KEY_SECRET="your-access-key-secret"
export DINGTALK_TOKEN="your-dingtalk-token"
export DINGTALK_SECRET="your-dingtalk-secret"

# 启动 Agent
./idcu_agent --config config/prod/agent.cfg
```

## 部署建议

1. **配置管理**：使用配置管理工具（如 Ansible、Chef）管理配置
2. **版本控制**：配置文件应纳入版本控制
3. **机密管理**：使用密钥管理系统（如 Vault）管理敏感信息
4. **监控告警**：配置完整的监控和告警，及时发现问题
5. **备份恢复**：定期备份配置，便于快速恢复
6. **灰度发布**：先在小规模部署验证，再全量发布
7. **回滚方案**：准备好回滚方案，应对突发问题

## 安全建议

1. **文件权限**：配置文件权限应为 600，仅所有者可读写
2. **网络隔离**：Agent 应部署在专用网络区域
3. **访问控制**：限制管理 API 的访问来源
4. **TLS 加密**：启用 TLS 加密通信
5. **定期更新**：及时更新 Agent 和依赖库

## 相关配置

- [开发环境配置](../dev/README.md)
- [测试环境配置](../test/README.md)
