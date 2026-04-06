# idcu-module-build 通用化开发计划（配置自动适配・可落地版）

&#x20;

**核心定位**：从 IDCU Agent 专属构建工具 → **通用跨项目模块构建框架**

**核心设计**：**配置驱动 + 自动适配**，项目无需改代码，仅通过配置文件即可完成适配

**目标**：任何 C 项目（CMake 构建）接入成本 < 5 分钟，零代码侵入

***

## 一、整体架构（自动适配核心）

### 分层架构（解耦 + 通用 + 可扩展）

plaintext

```
idcu-module-build/
├── config/                 # 全局自动适配配置（核心）
│   ├── auto-adapter.yaml   # 项目自动识别适配规则（自动匹配项目类型）
│   └── default.yaml        # 通用默认构建配置
├── cmake/
│   ├── core/               # 通用无依赖核心（自动适配引擎）
│   ├── adapters/           # 内置项目适配模板（Agent/通用/自定义）
│   └── plugins/            # 可插拔扩展（签名/打包/发布）
├── scripts/                # 跨平台构建脚本
├── templates/              # 项目接入模板（module.json/CMakeLists.txt）
└── docs/                   # 通用+专属文档

```

### 自动适配原理（关键）

1. 工具**自动扫描项目根目录**
2. 匹配 `auto-adapter.yaml` 规则（文件 / 目录 / 配置标识）
3. **自动加载对应适配策略**（无需手动指定）
4. 读取项目 `module.json` → 自动完成构建

***

## 二、阶段目标（6 个阶段，可直接排期）

### 阶段 1：基础通用化改造（P0・必做）

**目标**：剥离 IDCU Agent 强耦合，实现通用核心

**周期**：3\~5 天

#### 1.1 核心层重构（无任何项目依赖）

- 拆分 `idcu_module_build.cmake` → 纯通用核心
- 移除所有硬编码路径、宏、专属逻辑
- 提供统一 API：
  - `module_build_init()`
  - `module_build_load_config()`
  - `module_build_auto_detect_project()`
  - `module_build_run()`

#### 1.2 配置驱动体系建立

- 定义 **`module.json`** **通用标准**（跨项目通用）
- 定义 **`auto-adapter.yaml`** **自动适配规则**
- 定义 **项目类型标识**：
  - idcu\_agent
  - c\_shared\_library
  - c\_static\_library
  - c\_executable
  - custom

#### 1.3 最小可用版本（MVP）

- 支持：自动识别项目 + 通用编译链接 + 跨平台
- 可构建：任意 C 动态库 / 静态库

***

### 阶段 2：配置自动适配引擎（核心特性・P0）

**目标**：让工具 “自动认识项目”，无需人工指定适配方式

**周期**：4\~6 天

#### 2.1 `auto-adapter.yaml` 规则引擎开发

yaml

```
# 示例：自动适配规则
adapters:
  - name: idcu_agent
    detect:
      - exists: "modules/core/sdk/"
      - exists: "agent.cfg"
      - file_contains: "CMakeLists.txt" "idcu_agent"
    config:
      type: shared_library
      sdk_path: modules/core/sdk
      output_dir: agent-modules
      c_standard: c11

  - name: generic_c_project
    detect:
      - exists: "CMakeLists.txt"
      - not_exists: "agent.cfg"
    config:
      type: auto
      output_dir: build/modules
      c_standard: c99

```

#### 2.2 自动适配 CMake 函数开发

- `auto_detect_project_type()`
- `auto_load_project_config()`
- `auto_apply_build_strategy()`

#### 2.3 多项目自动适配验证

- 验证 IDCU Agent
- 验证纯 C 通用项目
- 验证自定义项目

***

### 阶段 3：`module.json` 统一配置标准（P0）

**目标**：一个配置文件描述所有模块，跨项目通用

**周期**：2\~3 天

json

```
{
  "module": {
    "name": "demo",
    "version": "1.0.0",
    "type": "shared",
    "auto_adapt": true
  },
  "build": {
    "c_standard": "auto",
    "src": "src/*.c",
    "include": ["inc"],
    "defines": [],
    "deps": []
  },
  "project": {
    "idcu_agent": {},
    "generic": {}
  }
}

```

**自动能力**：

- `auto: true` → 工具自动判断编译类型
- `src: auto` → 自动扫描 src 目录
- `deps: auto` → 自动检测依赖

***

### 阶段 4：跨项目通用构建能力（P1）

**目标**：任何 C 项目都能直接用

**周期**：5\~7 天

#### 4.1 跨平台构建统一

- Linux / Windows /macOS 自动识别
- 自动区分 `.so` / `.dll` / `.dylib`
- 自动处理编译选项差异

#### 4.2 依赖自动管理

- 本地依赖自动查找
- 远程依赖自动拉取（HTTP/Git）
- 版本校验自动执行

#### 4.3 产物标准化输出

plaintext

```
output/
  {module}-{version}-{platform}-{arch}/
    bin/
    lib/
    meta/module.json
    build-info.json

```

***

### 阶段 5：插件化 & 扩展能力（P1）

**目标**：不改动核心，通过配置扩展功能

**周期**：4\~5 天

支持插件（可配置开关）：

- 版本自动生成
- 模块签名
- 代码覆盖率
- 打包（ZIP/TAR）
- 发布到仓库

配置方式：

json

```
"plugins": {
  "sign": true,
  "package": true,
  "coverage": false
}

```

***

### 阶段 6：工程化、文档、示例（P2）

**目标**：开箱即用

**周期**：3\~4 天

- 完善通用 README
- 提供 3 类示例：
  - IDCU Agent 模块
  - 通用 C 动态库
  - 自定义项目
- 提供一键模板生成脚本
- CI 自动构建适配

***

## 三、详细开发任务清单（可直接给开发人员）

### 核心任务（必做）

1. 重构 CMake 核心，解耦 IDCU 专属代码
2. 设计 `module.json` 通用 schema
3. 开发 `auto-adapter.yaml` 规则解析
4. 开发项目自动识别函数
5. 开发自动构建流程
6. 开发跨平台产物规范
7. 兼容旧版 IDCU Agent 构建（不影响原有项目）

### 自动适配任务（核心亮点）

1. 自动识别项目类型
2. 自动选择编译模式
3. 自动配置头文件 / 链接库
4. 自动判断 SDK 路径
5. 自动生成构建信息

### 扩展任务

1. 插件系统
2. 依赖自动拉取
3. 构建缓存
4. 多架构支持（x86/ARM）

***

## 四、接入方式（最终效果・极简）

其他项目**只需两步**：

1. 拷贝 `idcu-module-build` 到项目
2. 创建 `module.json`（可从模板生成）

**然后执行：**

bash

运行

```
./idcu-module-build/scripts/build.sh

```

工具自动完成：

- 识别项目类型
- 加载适配策略
- 扫描源码
- 构建模块
- 输出标准产物

***

## 五、排期建议（总周期：3 周）

- 第 1 周：核心重构 + 配置标准 + 自动适配引擎
- 第 2 周：跨项目构建 + 依赖管理 + 插件化
- 第 3 周：示例 + 文档 + 测试 + 兼容验证

***

## 六、交付物清单

1. `idcu-module-build` 通用化框架
2. `module.json` 通用规范
3. `auto-adapter.yaml` 自动适配规则
4. 自动识别、自动适配、自动构建引擎
5. 跨平台构建脚本
6. 三类可运行示例
7. 完整接入文档
8. IDCU Agent 兼容适配包

