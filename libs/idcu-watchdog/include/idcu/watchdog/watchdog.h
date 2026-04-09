#ifndef IDCU_WATCHDOG_WATCHDOG_H
#define IDCU_WATCHDOG_WATCHDOG_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/watchdog/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_watchdog_init(idcu_Watchdog_Context** ctx);
void idcu_watchdog_destroy(idcu_Watchdog_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
