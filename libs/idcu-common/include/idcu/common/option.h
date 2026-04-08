#ifndef IDCU_COMMON_OPTION_H
#define IDCU_COMMON_OPTION_H

#include "error_code.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define IDCU_OPTION_TYPE(T) idcu_Option_##T

#define IDCU_OPTION_DEFINE(T)                                                            \
    typedef struct                                                                       \
    {                                                                                    \
        bool has_value;                                                                  \
        T    value;                                                                      \
    } idcu_Option_##T;                                                                   \
                                                                                         \
    static inline idcu_Option_##T idcu_Some_##T(T value)                                 \
    {                                                                                    \
        idcu_Option_##T opt;                                                             \
        opt.has_value = true;                                                            \
        opt.value     = value;                                                           \
        return opt;                                                                      \
    }                                                                                    \
                                                                                         \
    static inline idcu_Option_##T idcu_None_##T(void)                                    \
    {                                                                                    \
        idcu_Option_##T opt;                                                             \
        opt.has_value = false;                                                           \
        return opt;                                                                      \
    }                                                                                    \
                                                                                         \
    static inline bool idcu_Option_is_some_##T(const idcu_Option_##T* opt)               \
    {                                                                                    \
        return opt && opt->has_value;                                                    \
    }                                                                                    \
                                                                                         \
    static inline bool idcu_Option_is_none_##T(const idcu_Option_##T* opt)               \
    {                                                                                    \
        return !opt || !opt->has_value;                                                  \
    }                                                                                    \
                                                                                         \
    static inline T idcu_Option_unwrap_##T(const idcu_Option_##T* opt)                   \
    {                                                                                    \
        return opt->value;                                                               \
    }                                                                                    \
                                                                                         \
    static inline T idcu_Option_unwrap_or_##T(const idcu_Option_##T* opt, T default_val) \
    {                                                                                    \
        return (opt && opt->has_value) ? opt->value : default_val;                       \
    }

    IDCU_OPTION_DEFINE(int)
    IDCU_OPTION_DEFINE(int64_t)
    IDCU_OPTION_DEFINE(uint64_t)
    IDCU_OPTION_DEFINE(double)
    IDCU_OPTION_DEFINE(bool)
    IDCU_OPTION_DEFINE(void*)

#define idcu_Some(T, value)                        idcu_Some_##T(value)
#define idcu_None(T)                               idcu_None_##T()
#define idcu_Option_is_some(T, opt)                idcu_Option_is_some_##T(opt)
#define idcu_Option_is_none(T, opt)                idcu_Option_is_none_##T(opt)
#define idcu_Option_unwrap(T, opt)                 idcu_Option_unwrap_##T(opt)
#define idcu_Option_unwrap_or(T, opt, default_val) idcu_Option_unwrap_or_##T(opt, default_val)

#define IDCU_OPTION_IF_LET(T, var, opt)                              \
    for (int _i = 0; _i < 1 && idcu_Option_is_some(T, &(opt)); _i++) \
        for (T var = idcu_Option_unwrap(T, &(opt)); _i; _i = 0)

#ifdef __cplusplus
}
#endif

#endif
