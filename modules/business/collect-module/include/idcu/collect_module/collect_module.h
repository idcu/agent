#ifndef IDCU_COLLECT_MODULE_COLLECT_MODULE_H
#define IDCU_COLLECT_MODULE_COLLECT_MODULE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int initialized;
    void* user_data;
} idcu_CollectModule;

int idcu_collect_module_init(idcu_CollectModule* cm);
int idcu_collect_module_start(idcu_CollectModule* cm);
int idcu_collect_module_stop(idcu_CollectModule* cm);
void idcu_collect_module_destroy(idcu_CollectModule* cm);

#ifdef __cplusplus
}
#endif

#endif
