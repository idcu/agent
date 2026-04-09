#ifndef IDCU_COMMON_SECURITY_H
#define IDCU_COMMON_SECURITY_H

#include "error_code.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void idcu_secure_zero(void* ptr, size_t size);
int  idcu_memcmp_constant(const void* a, const void* b, size_t size);

uint64_t idcu_random_uint64(void);
void     idcu_random_bytes(uint8_t* buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif
