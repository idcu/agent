#ifndef ATOMIC_H
#define ATOMIC_H

#include "common/error_code.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t atomic_fetch_add_int32(volatile int32_t* ptr, int32_t value);
int32_t atomic_fetch_sub_int32(volatile int32_t* ptr, int32_t value);
int32_t atomic_fetch_or_int32(volatile int32_t* ptr, int32_t value);
int32_t atomic_fetch_and_int32(volatile int32_t* ptr, int32_t value);
int32_t atomic_fetch_xor_int32(volatile int32_t* ptr, int32_t value);
int32_t atomic_exchange_int32(volatile int32_t* ptr, int32_t value);
int atomic_compare_exchange_int32(volatile int32_t* ptr, int32_t* expected, int32_t desired);
int32_t atomic_load_int32(volatile int32_t* ptr);
void atomic_store_int32(volatile int32_t* ptr, int32_t value);
int32_t atomic_inc_int32(volatile int32_t* ptr);
int32_t atomic_dec_int32(volatile int32_t* ptr);

int64_t atomic_fetch_add_int64(volatile int64_t* ptr, int64_t value);
int64_t atomic_fetch_sub_int64(volatile int64_t* ptr, int64_t value);
int64_t atomic_exchange_int64(volatile int64_t* ptr, int64_t value);
int atomic_compare_exchange_int64(volatile int64_t* ptr, int64_t* expected, int64_t desired);
int64_t atomic_load_int64(volatile int64_t* ptr);
void atomic_store_int64(volatile int64_t* ptr, int64_t value);

#ifdef __cplusplus
}
#endif

#endif
