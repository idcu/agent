#include "module_version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int idcu_version_parse(const char* version_str, idcu_ModuleVersion* out_version) {
    if (!version_str || !out_version) {
        return IDCU_ERROR_INVALID_PARAM;
    }

    memset(out_version, 0, sizeof(idcu_ModuleVersion));

    char* copy = strdup(version_str);
    if (!copy) {
        return IDCU_ERROR_MEMORY;
    }

    char* dash = strchr(copy, '-');
    if (dash) {
        *dash = '\0';
        out_version->pre_release = strdup(dash + 1);
    }

    char* token = strtok(copy, ".");
    if (token) {
        out_version->major = (uint16_t)atoi(token);
        token = strtok(NULL, ".");
        if (token) {
            out_version->minor = (uint16_t)atoi(token);
            token = strtok(NULL, ".");
            if (token) {
                out_version->patch = (uint16_t)atoi(token);
            }
        }
    }

    free(copy);
    return IDCU_SUCCESS;
}

int idcu_version_format(const idcu_ModuleVersion* version, char* buffer, size_t buffer_size) {
    if (!version || !buffer) {
        return IDCU_ERROR_INVALID_PARAM;
    }

    int written;
    if (version->pre_release) {
        written = snprintf(buffer, buffer_size, "%u.%u.%u-%s",
                          version->major, version->minor, version->patch,
                          version->pre_release);
    } else {
        written = snprintf(buffer, buffer_size, "%u.%u.%u",
                          version->major, version->minor, version->patch);
    }

    return (written >= 0 && (size_t)written < buffer_size) ? IDCU_SUCCESS : IDCU_ERROR_BUFFER_TOO_SMALL;
}

idcu_VersionCompareResult idcu_version_compare(const idcu_ModuleVersion* v1, const idcu_ModuleVersion* v2) {
    if (!v1 || !v2) {
        return IDCU_VERSION_EQUAL;
    }

    if (v1->major < v2->major) return IDCU_VERSION_LESS;
    if (v1->major > v2->major) return IDCU_VERSION_GREATER;

    if (v1->minor < v2->minor) return IDCU_VERSION_LESS;
    if (v1->minor > v2->minor) return IDCU_VERSION_GREATER;

    if (v1->patch < v2->patch) return IDCU_VERSION_LESS;
    if (v1->patch > v2->patch) return IDCU_VERSION_GREATER;

    // 预发布版本处�? 有预发布标签的版本比没有的版本旧
    if (v1->pre_release && !v2->pre_release) return IDCU_VERSION_LESS;
    if (!v1->pre_release && v2->pre_release) return IDCU_VERSION_GREATER;

    // 两个都有预发布标签，简单比较字符串
    if (v1->pre_release && v2->pre_release) {
        int cmp = strcmp(v1->pre_release, v2->pre_release);
        if (cmp < 0) return IDCU_VERSION_LESS;
        if (cmp > 0) return IDCU_VERSION_GREATER;
    }

    return IDCU_VERSION_EQUAL;
}

int idcu_version_is_compatible(const idcu_ModuleVersion* current, const idcu_ModuleVersion* required) {
    if (!current || !required) {
        return 0;
    }
    return current->major == required->major;
}

int idcu_version_check_dependency(const idcu_ModuleVersion* actual, const idcu_ModuleDependency* dependency) {
    if (!actual || !dependency) {
        return IDCU_ERROR_INVALID_PARAM;
    }

    idcu_VersionCompareResult min_result = idcu_version_compare(actual, &dependency->min_version);
    if (min_result == IDCU_VERSION_LESS) {
        return IDCU_ERROR_VERSION_TOO_OLD;
    }

    if (dependency->has_max_version) {
        idcu_VersionCompareResult max_result = idcu_version_compare(actual, &dependency->max_version);
        if (max_result == IDCU_VERSION_GREATER) {
            return IDCU_ERROR_VERSION_TOO_NEW;
        }
    }

    if (!idcu_version_is_compatible(actual, &dependency->min_version)) {
        return IDCU_ERROR_VERSION_INCOMPATIBLE;
    }

    return IDCU_SUCCESS;
}

idcu_ModuleVersion idcu_version_create(uint16_t major, uint16_t minor, uint16_t patch) {
    idcu_ModuleVersion v = { major, minor, patch, NULL };
    return v;
}

idcu_ModuleVersion idcu_version_create_pre(uint16_t major, uint16_t minor, uint16_t patch, const char* pre_release) {
    idcu_ModuleVersion v = { major, minor, patch, pre_release };
    return v;
}
