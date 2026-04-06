# idcu-permission

IDCU 项目的独立权限管理库，提供模块级别的权限控制功能。

## 特性

- 跨平台支持（Windows、Linux、macOS）
- 模块级别的权限管理
- 支持通配符权限（如 "file.*）
- 线程安全操作
- 权限检查、授权和撤销

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
find_package(idcu-permission REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::permission)
```

## API 文档

### 初始化权限管理器

```c
#include &lt;idcu/permission/permission.h&gt;

int ret = idcu_permission_manager_init();
if (ret == IDCU_ERR_SUCCESS) {
    // 使用权限管理器
    idcu_permission_manager_shutdown();
}
```

### 授权和检查权限

```c
// 授权
idcu_permission_grant("my_module", "file.read");
idcu_permission_grant("my_module", "network.connect");

// 检查权限
if (idcu_permission_check("my_module", "file.read")) {
    // 有权限
}

// 检查任一权限
const char* perms[] = {"file.read", "file.write"};
if (idcu_permission_check_any("my_module", perms, 2)) {
    // 有任一权限
}

// 检查所有权限
if (idcu_permission_check_all("my_module", perms, 2)) {
    // 有所有权限
}
```

### 获取权限列表

```c
// 获取模块权限
char* perms[64];
int count;
idcu_permission_get_module_permissions("my_module", perms, &amp;count);

// 列出所有模块
char* modules[64];
int module_count;
idcu_permission_list_modules(modules, &amp;module_count);
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [基本权限管理示例](examples/example_basic.c) - 演示权限基本操作
- [通配符权限示例](examples/example_wildcard.c) - 演示通配符权限使用

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

./example_basic
./example_wildcard
```

## 依赖

- idcu-common - IDCU 基础通用组件库

## 许可证

详见项目根目录的 LICENSE 文件。
