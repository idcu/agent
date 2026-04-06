# IDCU Module Build - 通用模块构建工具入口
# 向后兼容的主入口文件

# 包含通用核心
include("${CMAKE_CURRENT_LIST_DIR}/core/module_build_core.cmake")

# 包含插件系统
include("${CMAKE_CURRENT_LIST_DIR}/plugins/module_build_plugins.cmake")

# 包含 IDCU Agent 适配器
include("${CMAKE_CURRENT_LIST_DIR}/adapters/idcu_agent_adapter.cmake")

# 初始化构建系统
module_build_init()

# 初始化插件系统
init_plugins()

# 加载默认配置
module_build_load_config()

message(STATUS "IDCU Module Build System loaded - ready to use!")
