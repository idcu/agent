# idcu-sandbox

IDCU 项目的独立沙箱安全库，提供模块级别的安全沙箱和权限管理功能。

## 特性

- 跨平台支持（Windows、Linux、macOS）
- 模块级别的沙箱管理
- 位掩码权限系统
- 沙箱注册中心
- 配额管理
- 线程安全操作

## 快速开始

### 构建

```bash
mkdir build &amp;&amp; cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-common REQUIRED)
find_package(idcu-sandbox REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::sandbox)
```

## API 文档

### 创建沙箱

```c
#include &lt;idcu/sandbox/sandbox.h&gt;

idcu_Sandbox sb;
int ret = idcu_sandbox_init(&amp;sb, 123, IDCU_PERM_FILE | IDCU_PERM_NETWORK);
if (ret == IDCU_ERR_OK) {
    // 使用沙箱
    idcu_sandbox_destroy(&amp;sb);
}
```

### 权限管理

```c
// 添加权限
idcu_sandbox_add_perm(&amp;sb, IDCU_PERM_RUN);

// 移除权限
idcu_sandbox_remove_perm(&amp;sb, IDCU_PERM_HW);

// 设置权限
idcu_sandbox_set_perm(&amp;sb, IDCU_PERM_ALL);

// 检查权限
if (idcu_sandbox_perm_check(&amp;sb, IDCU_PERM_FILE) == IDCU_ERR_OK) {
    // 有权限
}
```

### 配额管理

```c
// 设置配额
idcu_sandbox_set_quota(&amp;sb, 1024);

// 获取配额
uint32_t quota = idcu_sandbox_get_quota(&amp;sb);
```

### 沙箱注册中心

```c
// 初始化注册中心
idcu_sandbox_registry_init();

// 注册沙箱
idcu_sandbox_registry_add(&amp;sb);

// 获取沙箱
idcu_Sandbox* sb_ptr = idcu_sandbox_registry_get(123);

// 检查权限
if (idcu_sandbox_registry_check_perm(123, IDCU_PERM_FILE) == IDCU_ERR_OK) {
    // 有权限
}

// 移除沙箱
idcu_sandbox_registry_remove(123);

// 销毁注册中心
idcu_sandbox_registry_destroy();
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [基本沙箱示例](examples/example_basic.c) - 演示沙箱基本操作
- [沙箱注册示例](examples/example_registry.c) - 演示沙箱注册中心使用

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

./example_basic
./example_registry
```

## 权限常量

- `IDCU_PERM_SEND` - 发送权限
- `IDCU_PERM_RECV` - 接收权限
- `IDCU_PERM_RUN` - 运行权限
- `IDCU_PERM_HW` - 硬件访问权限
- `IDCU_PERM_FILE` - 文件访问权限
- `IDCU_PERM_NETWORK` - 网络访问权限
- `IDCU_PERM_ALL` - 所有权限

## 依赖

- idcu-common - IDCU 基础通用组件库

## 许可证

详见项目根目录的 LICENSE 文件。
