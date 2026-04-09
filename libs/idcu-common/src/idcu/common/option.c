#include "idcu/common/option.h"
#include <stdlib.h>
#include <string.h>

int idcu_option_some(idcu_Option* opt, const void* value, size_t value_size)
{
    if (!opt || !value || value_size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    opt->value = malloc(value_size);
    if (!opt->value) {
        return IDCU_ERR_NO_MEMORY;
    }

    memcpy(opt->value, value, value_size);
    opt->value_size = value_size;
    opt->has_value = true;

    return IDCU_ERR_OK;
}

void idcu_option_destroy(idcu_Option* opt)
{
    if (!opt)
        return;

    if (opt->value) {
        free(opt->value);
        opt->value = NULL;
    }

    opt->has_value = false;
    opt->value_size = 0;
}

bool idcu_option_has_value(const idcu_Option* opt)
{
    return opt ? opt->has_value : false;
}

int idcu_option_get(const idcu_Option* opt, void* out_value)
{
    if (!opt || !opt->has_value) {
        return IDCU_ERR_NOT_FOUND;
    }

    if (out_value) {
        memcpy(out_value, opt->value, opt->value_size);
    }

    return IDCU_ERR_OK;
}
