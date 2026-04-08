# 常见问题 FAQ

本页面收集了 IDCU Agent 项目开发和使用过程中的常见问题及解决方案。

## 目录
- [编译相关问题](#编译相关问题)
- [运行相关问题](#运行相关问题)
- [模块开发问题](#模块开发问题)
- [配置相关问题](#配置相关问题)
- [网络相关问题](#网络相关问题)
- [性能相关问题](#性能相关问题)
- [其他问题](#其他问题)

---

## 编译相关问题

### Q: 在 Windows 上编译时出现 "找不到 gcc 命令" 错误
**A:** 
1. 确保已安装 MinGW-w64 或 TDM-GCC
2. 将 MinGW 的 bin 目录添加到系统 PATH 环境变量
3. 重启命令提示符或 PowerShell
4. 验证安装：运行 `gcc --version`

### Q: CMake 报错 "Could not find CMake"
**A:**
1. 下载并安装 CMake：https://cmake.org/download/
2. 安装时选择 "Add CMake to the system PATH"
3. 重启终端
4. 验证：运行 `cmake --version`

### Q: 编译时出现大量 undefined reference 错误
**A:**
1. 检查库的链接顺序，确保依赖库在被依赖库之前
2. 确保所有必需的库都已正确链接
3. 检查 CMakeLists.txt 中的 target_link_libraries 设置
4. 确保库的编译类型（静态/动态）匹配

**示例：**
```cmake
# 正确的顺序：先依赖库，后被依赖库
target_link_libraries(your_target PRIVATE
    idcu::common    # 基础库
    idcu::log       # 依赖 common
    idcu::network   # 依赖 common 和 log
)
```

### Q: Windows 上使用 MinGW 编译时出现 Winsock 相关错误
**A:**
确保链接了 `ws2_32` 库：
```cmake
if(WIN32)
    target_link_libraries(your_target PRIVATE ws2_32)
endif()
```

---

## 运行相关问题

### Q: 程序启动后立即退出，没有任何输出
**A:**
1. **Windows 用户：** 不要双击运行 exe，使用命令行运行
   ```cmd
   cd out
   idcu_agent.exe
   ```
2. 检查配置文件是否存在且格式正确
3. 设置日志级别为 debug 查看详细输出：
   ```ini
   [general]
   log_level = debug
   ```
4. 使用调试器运行查看崩溃位置

### Q: 模块加载失败，提示 "module initialization failed"
**A:**
1. 检查模块的 init 函数返回值
2. 查看日志输出中的详细错误信息
3. 确认模块依赖的其他模块已正确加载
4. 验证模块版本兼容性
5. 检查模块是否有内存分配失败

### Q: 程序运行一段时间后崩溃
**A:**
1. 使用 AddressSanitizer 检测内存问题：
   ```bash
   ./scripts/build_with_asan.sh
   ```
2. 检查日志中的错误信息
3. 使用调试器获取崩溃时的调用栈
4. 查看是否有资源泄漏

---

## 模块开发问题

### Q: 如何创建一个新模块？
**A:**
1. 使用提供的模块创建脚本：
   ```bash
   # Windows
   scripts\create-module.bat my_module
   
   # Linux
   ./scripts/create-module.sh my_module
   ```
2. 或者参考 [快速开始指南](./quick_start.md#创建你的第一个模块) 中的步骤
3. 查看现有模块作为示例：`modules/business/`

### Q: 模块之间如何通信？
**A:**
使用消息总线（idcu-msgbus）：
```c
// 发送消息
idcu_msg_send(bus, src_module_id, dst_module_id, 
               IDCU_MSG_PRIO_NORMAL, &context);

// 接收消息
idcu_msg_recv(bus, my_module_id, &msg);

// 广播消息
idcu_msg_broadcast(bus, src_module_id, 
                    IDCU_MSG_PRIO_NORMAL, &context);
```

### Q: 模块的 init、run、stop 函数有什么区别？
**A:**
- **init：** 模块初始化时调用一次，用于分配资源、初始化数据
- **run：** 主循环函数，会被频繁调用，执行模块的主要逻辑
- **stop：** 模块停止时调用一次，用于释放资源、清理数据

### Q: 如何调试自定义模块？
**A:**
1. 在模块代码中添加详细的日志
2. 使用 `IDCU_LOG_DEBUG` 输出调试信息
3. 在配置文件中启用 debug 日志级别
4. 使用调试器在模块函数中设置断点

---

## 配置相关问题

### Q: 配置文件修改后不生效
**A:**
1. 确保修改的是正确的配置文件（`config/agent.cfg` 或 `out/agent.cfg`）
2. 重启程序使配置生效
3. 或者使用配置热重载功能：
   ```c
   idcu_config_reload();
   ```
4. 检查配置文件格式是否正确（INI 格式）

### Q: 如何使用不同环境的配置？
**A:**
项目提供了多环境配置：
```
config/
├── dev/           # 开发环境
├── test/          # 测试环境
└── prod/          # 生产环境
```

使用方法：
```bash
# 开发环境
./idcu_agent --config=config/dev/agent.cfg

# 生产环境
./idcu_agent --config=config/prod/agent.cfg
```

### Q: 配置项的值支持哪些数据类型？
**A:**
idcu-config 支持以下数据类型：
- 字符串：`key = value`
- 整数：`port = 8080`
- 浮点数：`ratio = 0.75`
- 布尔值：`enabled = true` 或 `enabled = false`

---

## 网络相关问题

### Q: TCP 连接总是超时
**A:**
1. 检查服务器是否正常运行
2. 确认防火墙允许连接
3. 验证 IP 地址和端口号是否正确
4. 调整连接超时时间：
   ```c
   idcu_NetworkConfig config = {
       .connect_timeout_ms = 10000  // 10 秒超时
   };
   idcu_network_socket_set_config(&sock, &config);
   ```

### Q: UDP 接收不到数据
**A:**
1. 确认绑定的地址和端口正确
2. 检查防火墙设置
3. 验证发送方的目标地址和端口
4. 使用 Wireshark 等工具抓包分析

### Q: 如何处理网络断开和重连？
**A:**
使用自动重连功能：
```c
// 设置重连配置
idcu_NetworkConfig config = {
    .max_retries = 5,
    .reconnect_delay_ms = 2000
};
idcu_network_socket_set_config(&sock, &config);

// 手动重连
idcu_network_socket_reconnect(&sock);
```

---

## 性能相关问题

### Q: 程序运行时 CPU 占用过高
**A:**
1. 检查模块的 run 函数是否有忙等待
2. 在 run 函数中添加适当的 sleep：
   ```c
   #include <unistd.h>
   
   static int my_module_run(void) {
       // 处理逻辑
       // ...
       
       // 避免 100% CPU 占用
       usleep(1000);  // 休眠 1 毫秒
       return 0;
   }
   ```
3. 检查是否有死循环
4. 使用性能分析工具找出热点

### Q: 消息处理延迟过高
**A:**
1. 使用更高的消息优先级：
   ```c
   idcu_msg_send(bus, src, dst, IDCU_MSG_PRIO_REALTIME, &ctx);
   ```
2. 增加消息队列大小
3. 优化消息处理函数
4. 检查是否有阻塞操作

### Q: 内存占用持续增长
**A:**
1. 使用 AddressSanitizer 检测内存泄漏：
   ```bash
   ./scripts/build_with_asan.sh
   ```
2. 检查模块是否正确释放资源
3. 确认每个 malloc/calloc 都有对应的 free
4. 使用 Valgrind 进行详细分析

---

## 其他问题

### Q: 如何获取帮助？
**A:**
1. 查看文档：
   - [快速开始指南](./quick_start.md)
   - [调试指南](./debugging_guide.md)
   - [模块开发手册](./module_development.md)
2. 检查 GitHub Issues
3. 提交新的 Issue，包含：
   - 详细的错误描述
   - 复现步骤
   - 日志输出
   - 系统环境信息

### Q: 如何贡献代码？
**A:**
1. Fork 项目仓库
2. 创建特性分支
3. 提交更改
4. 推送到分支
5. 创建 Pull Request

详见 [贡献指南](../CONTRIBUTING.md)

### Q: 项目支持哪些平台？
**A:**
目前支持：
- Windows 10/11 (MinGW 或 MSVC)
- Linux (Ubuntu, Debian, CentOS 等)
- macOS (实验性支持)

### Q: 如何报告安全漏洞？
**A:**
请不要在公开的 Issue 中报告安全漏洞。请通过以下方式联系：
- 发送邮件到 security@idcu-project.org
- 详细描述漏洞和复现步骤
- 我们会尽快评估并修复

---

## 找不到答案？

如果以上 FAQ 没有解决你的问题：

1. 仔细阅读相关文档
2. 搜索现有的 GitHub Issues
3. 提交新的 Issue，提供尽可能详细的信息

**提问时请包含：**
- 操作系统和版本
- 编译器和版本
- 项目版本或 commit hash
- 详细的错误描述
- 复现步骤
- 相关的日志输出
- 你已经尝试过的解决方法

---

**希望这些能帮到你！** 💡

如果你有其他常见问题，欢迎提交 PR 补充到这个文档中。
