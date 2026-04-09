#ifndef IDCU_COMMON_OPTION_H
#define IDCU_COMMON_OPTION_H

#include "error_code.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    bool has_value;
    void* value;
    size_t value_size;
} idcu_Option;

#define IDCU_OPTION_NONE ((idcu_Option){false, NULL, 0})

int  idcu_option_some(idcu_Option* opt, const void* value, size_t value_size);
void idcu_option_destroy(idcu_Option* opt);

bool idcu_option_has_value(const idcu_Option* opt);
int  idcu_option_get(const idcu_Option* opt, void* out_value);

#ifdef __cplusplus
}
#endif

#endif
