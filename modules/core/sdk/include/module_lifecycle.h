#ifndef IDCU_SDK_MODULE_LIFECYCLE_H
#define IDCU_SDK_MODULE_LIFECYCLE_H

#include "idcu/common/error_code.h"
#include "module_def.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct idcu_module_lifecycle_context idcu_ModuleLifecycleContext;

    typedef struct
    {
        int (*on_pre_init)(idcu_ModuleLifecycleContext* ctx);
        int (*on_init)(idcu_ModuleLifecycleContext* ctx);
        int (*on_post_init)(idcu_ModuleLifecycleContext* ctx);
        int (*on_pre_start)(idcu_ModuleLifecycleContext* ctx);
        int (*on_start)(idcu_ModuleLifecycleContext* ctx);
        int (*on_post_start)(idcu_ModuleLifecycleContext* ctx);
        int (*on_pre_stop)(idcu_ModuleLifecycleContext* ctx);
        int (*on_stop)(idcu_ModuleLifecycleContext* ctx);
        int (*on_post_stop)(idcu_ModuleLifecycleContext* ctx);
        void (*on_pre_destroy)(idcu_ModuleLifecycleContext* ctx);
        void (*on_destroy)(idcu_ModuleLifecycleContext* ctx);
        void (*on_post_destroy)(idcu_ModuleLifecycleContext* ctx);
    } idcu_ModuleLifecycleCallbacks;

    typedef struct
    {
        idcu_ModuleLifecycleCallbacks callbacks;
        void*                         user_data;
    } idcu_ModuleLifecycleConfig;

    idcu_ModuleLifecycleContext* idcu_module_lifecycle_create_context(void);
    void idcu_module_lifecycle_destroy_context(idcu_ModuleLifecycleContext* ctx);

    void* idcu_module_lifecycle_get_user_data(idcu_ModuleLifecycleContext* ctx);
    void  idcu_module_lifecycle_set_user_data(idcu_ModuleLifecycleContext* ctx, void* user_data);

    void idcu_module_lifecycle_set_config(idcu_ModuleLifecycleContext*      ctx,
                                          const idcu_ModuleLifecycleConfig* config);

    int  idcu_module_lifecycle_pre_init(idcu_ModuleLifecycleContext* ctx);
    int  idcu_module_lifecycle_init(idcu_ModuleLifecycleContext* ctx);
    int  idcu_module_lifecycle_post_init(idcu_ModuleLifecycleContext* ctx);
    int  idcu_module_lifecycle_pre_start(idcu_ModuleLifecycleContext* ctx);
    int  idcu_module_lifecycle_start(idcu_ModuleLifecycleContext* ctx);
    int  idcu_module_lifecycle_post_start(idcu_ModuleLifecycleContext* ctx);
    int  idcu_module_lifecycle_pre_stop(idcu_ModuleLifecycleContext* ctx);
    int  idcu_module_lifecycle_stop(idcu_ModuleLifecycleContext* ctx);
    int  idcu_module_lifecycle_post_stop(idcu_ModuleLifecycleContext* ctx);
    void idcu_module_lifecycle_pre_destroy(idcu_ModuleLifecycleContext* ctx);
    void idcu_module_lifecycle_destroy(idcu_ModuleLifecycleContext* ctx);
    void idcu_module_lifecycle_post_destroy(idcu_ModuleLifecycleContext* ctx);

    idcu_ModuleState idcu_module_lifecycle_get_state(idcu_ModuleLifecycleContext* ctx);
    void idcu_module_lifecycle_set_state(idcu_ModuleLifecycleContext* ctx, idcu_ModuleState state);

#ifdef __cplusplus
}
#endif

#endif
