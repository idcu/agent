#include <idcu/collect_module/collect_module.h>
#include <string.h>

int idcu_collect_module_init(idcu_CollectModule* cm) {
    if (!cm) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(cm, 0, sizeof(idcu_CollectModule));
    cm->initialized = 0;
    return IDCU_ERR_OK;
}

int idcu_collect_module_start(idcu_CollectModule* cm) {
    if (!cm) {
        return IDCU_ERR_INVALID_ARG;
    }

    cm->initialized = 1;
    return IDCU_ERR_OK;
}

int idcu_collect_module_stop(idcu_CollectModule* cm) {
    if (!cm) {
        return IDCU_ERR_INVALID_ARG;
    }

    cm->initialized = 0;
    return IDCU_ERR_OK;
}

void idcu_collect_module_destroy(idcu_CollectModule* cm) {
    if (!cm) {
        return;
    }

    if (cm->initialized) {
        idcu_collect_module_stop(cm);
    }

    memset(cm, 0, sizeof(idcu_CollectModule));
}
