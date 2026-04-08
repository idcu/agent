#ifndef IDCU_SDK_MODULE_SANDBOX_H
#define IDCU_SDK_MODULE_SANDBOX_H

#include "idcu/common/error_code.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        IDCU_SANDBOX_RESOURCE_MEMORY           = 0,
        IDCU_SANDBOX_RESOURCE_CPU              = 1,
        IDCU_SANDBOX_RESOURCE_FILE_DESCRIPTORS = 2,
        IDCU_SANDBOX_RESOURCE_THREADS          = 3,
        IDCU_SANDBOX_RESOURCE_NETWORK          = 4
    } idcu_SandboxResourceType;

    typedef struct
    {
        uint64_t memory_limit_bytes;
        uint64_t memory_used_bytes;
        uint32_t cpu_limit_percent;
        uint32_t cpu_used_percent;
        uint32_t fd_limit;
        uint32_t fd_used;
        uint32_t thread_limit;
        uint32_t thread_used;
        bool     network_allowed;
    } idcu_SandboxResourceLimits;

    typedef struct idcu_module_sandbox_context idcu_ModuleSandboxContext;

    typedef void (*idcu_SandboxViolationCallback)(idcu_ModuleSandboxContext* ctx,
                                                  idcu_SandboxResourceType   resource_type,
                                                  const char* violation_details, void* user_data);

    idcu_ModuleSandboxContext* idcu_module_sandbox_create_context(void);
    void                       idcu_module_sandbox_destroy_context(idcu_ModuleSandboxContext* ctx);

    void* idcu_module_sandbox_get_user_data(idcu_ModuleSandboxContext* ctx);
    void  idcu_module_sandbox_set_user_data(idcu_ModuleSandboxContext* ctx, void* user_data);

    void idcu_module_sandbox_set_module_info(idcu_ModuleSandboxContext* ctx,
                                             const char*                module_name);

    int idcu_module_sandbox_set_limits(idcu_ModuleSandboxContext*        ctx,
                                       const idcu_SandboxResourceLimits* limits);

    int idcu_module_sandbox_get_limits(idcu_ModuleSandboxContext*  ctx,
                                       idcu_SandboxResourceLimits* out_limits);

    int idcu_module_sandbox_get_usage(idcu_ModuleSandboxContext*  ctx,
                                      idcu_SandboxResourceLimits* out_usage);

    int idcu_module_sandbox_check_memory(idcu_ModuleSandboxContext* ctx, uint64_t requested_bytes);
    int idcu_module_sandbox_check_fd(idcu_ModuleSandboxContext* ctx);
    int idcu_module_sandbox_check_thread(idcu_ModuleSandboxContext* ctx);
    int idcu_module_sandbox_check_network(idcu_ModuleSandboxContext* ctx);

    void idcu_module_sandbox_set_violation_callback(idcu_ModuleSandboxContext*    ctx,
                                                    idcu_SandboxViolationCallback callback,
                                                    void*                         user_data);

    void idcu_module_sandbox_track_memory_alloc(idcu_ModuleSandboxContext* ctx, uint64_t size);
    void idcu_module_sandbox_track_memory_free(idcu_ModuleSandboxContext* ctx, uint64_t size);
    void idcu_module_sandbox_track_fd_alloc(idcu_ModuleSandboxContext* ctx);
    void idcu_module_sandbox_track_fd_free(idcu_ModuleSandboxContext* ctx);
    void idcu_module_sandbox_track_thread_create(idcu_ModuleSandboxContext* ctx);
    void idcu_module_sandbox_track_thread_destroy(idcu_ModuleSandboxContext* ctx);

    void idcu_module_sandbox_reset_usage(idcu_ModuleSandboxContext* ctx);

#ifdef __cplusplus
}
#endif

#endif
