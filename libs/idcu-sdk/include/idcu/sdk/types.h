#ifndef IDCU_SDK_TYPES_H
#define IDCU_SDK_TYPES_H

#include <idcu/common/config.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct idcu_SdkContext idcu_SdkContext;

typedef int (*idcu_SdkModuleInit)(idcu_SdkContext* ctx, void* user_data);
typedef int (*idcu_SdkModuleStart)(idcu_SdkContext* ctx, void* user_data);
typedef int (*idcu_SdkModuleStop)(idcu_SdkContext* ctx, void* user_data);
typedef void (*idcu_SdkModuleDestroy)(idcu_SdkContext* ctx, void* user_data);

typedef struct {
    const char* name;
    const char* version;
    const char* description;
    idcu_SdkModuleInit init_fn;
    idcu_SdkModuleStart start_fn;
    idcu_SdkModuleStop stop_fn;
    idcu_SdkModuleDestroy destroy_fn;
    void* user_data;
} idcu_SdkModuleDef;

#ifdef __cplusplus
}
#endif

#endif
