#ifndef IDCU_MODULE_MODULE_VERSION_H
#define IDCU_MODULE_MODULE_VERSION_H

#include "module_def.h"
#include "error_code.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_VERSION_STR_MAX 64

// 版本比较结果
typedef enum {
    IDCU_VERSION_LESS    = -1,
    IDCU_VERSION_EQUAL   = 0,
    IDCU_VERSION_GREATER = 1
} idcu_VersionCompareResult;

// 解析版本字符串为 idcu_ModuleVersion
// 格式: "major.minor.patch" 或 "major.minor.patch-pre_release"
int idcu_version_parse(const char* version_str, idcu_ModuleVersion* out_version);

// 将 idcu_ModuleVersion 格式化为字符串
int idcu_version_format(const idcu_ModuleVersion* version, char* buffer, size_t buffer_size);

// 比较两个版本
// 返回:
//   IDCU_VERSION_LESS  如果 v1 < v2
//   IDCU_VERSION_EQUAL 如果 v1 == v2
//   IDCU_VERSION_GREATER 如果 v1 > v2
idcu_VersionCompareResult idcu_version_compare(const idcu_ModuleVersion* v1, const idcu_ModuleVersion* v2);

// 检查版本兼容性 (仅主版本号相同则兼容)
int idcu_version_is_compatible(const idcu_ModuleVersion* current, const idcu_ModuleVersion* required);

// 检查依赖版本是否满足要求
// 检查 actual >= min_version 且 actual <= max_version (如果设置了)
int idcu_version_check_dependency(const idcu_ModuleVersion* actual,
                                   const idcu_ModuleDependency* dependency);

// 创建简单的版本初始化辅助函数
idcu_ModuleVersion idcu_version_create(uint16_t major, uint16_t minor, uint16_t patch);
idcu_ModuleVersion idcu_version_create_pre(uint16_t major, uint16_t minor, uint16_t patch, const char* pre_release);

#ifdef __cplusplus
}
#endif

#endif // IDCU_MODULE_MODULE_VERSION_H
