#include <idcu/metrics_module/metrics_module.h>
#include <string.h>

int idcu_metrics_module_init(idcu_MetricsModule* mm) {
    if (!mm) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(mm, 0, sizeof(idcu_MetricsModule));
    mm->initialized = 0;

    int ret = idcu_metrics_registry_create(&mm->registry);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    mm->initialized = 1;

    return IDCU_ERR_OK;
}

int idcu_metrics_module_start(idcu_MetricsModule* mm) {
    if (!mm || !mm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }

    return IDCU_ERR_OK;
}

int idcu_metrics_module_stop(idcu_MetricsModule* mm) {
    if (!mm || !mm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }

    return IDCU_ERR_OK;
}

void idcu_metrics_module_destroy(idcu_MetricsModule* mm) {
    if (!mm) {
        return;
    }

    if (mm->registry) {
        idcu_metrics_registry_destroy(mm->registry);
        mm->registry = NULL;
    }

    mm->initialized = 0;
    memset(mm, 0, sizeof(idcu_MetricsModule));
}

int idcu_metrics_module_register_counter(idcu_MetricsModule* mm, const char* name, const char* help) {
    if (!mm || !mm->registry || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    (void)help;
    return IDCU_ERR_OK;
}

int idcu_metrics_module_register_gauge(idcu_MetricsModule* mm, const char* name, const char* help) {
    if (!mm || !mm->registry || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    (void)help;
    return IDCU_ERR_OK;
}

int idcu_metrics_module_inc(idcu_MetricsModule* mm, const char* name, double value) {
    if (!mm || !mm->registry || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    (void)value;
    return IDCU_ERR_OK;
}

int idcu_metrics_module_set(idcu_MetricsModule* mm, const char* name, double value) {
    if (!mm || !mm->registry || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    (void)value;
    return IDCU_ERR_OK;
}

int idcu_metrics_module_export_prometheus(idcu_MetricsModule* mm, char* buffer, size_t buffer_size) {
    if (!mm || !buffer) {
        return IDCU_ERR_INVALID_ARG;
    }
    if (buffer_size > 0) {
        buffer[0] = '\0';
    }
    return IDCU_ERR_OK;
}

idcu_MetricsRegistry* idcu_metrics_module_get_registry(idcu_MetricsModule* mm) {
    if (!mm) {
        return NULL;
    }
    return mm->registry;
}
