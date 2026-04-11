#ifndef IDCU_COMMON_DEADLOCK_DETECTOR_H
#define IDCU_COMMON_DEADLOCK_DETECTOR_H

#include "error_code.h"
#include "lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct idcu_DeadlockDetector idcu_DeadlockDetector;

int idcu_deadlock_detector_init(idcu_DeadlockDetector** detector);
void idcu_deadlock_detector_destroy(idcu_DeadlockDetector* detector);

int idcu_deadlock_detector_register_mutex(idcu_DeadlockDetector* detector, idcu_Mutex* mutex, const char* name);
void idcu_deadlock_detector_unregister_mutex(idcu_DeadlockDetector* detector, idcu_Mutex* mutex);

int idcu_deadlock_detector_on_lock_attempt(idcu_DeadlockDetector* detector, idcu_Mutex* mutex);
int idcu_deadlock_detector_on_lock_acquired(idcu_DeadlockDetector* detector, idcu_Mutex* mutex);
int idcu_deadlock_detector_on_unlock(idcu_DeadlockDetector* detector, idcu_Mutex* mutex);

int idcu_deadlock_detector_check_deadlock(idcu_DeadlockDetector* detector);

void idcu_deadlock_detector_print_report(idcu_DeadlockDetector* detector);

#ifdef __cplusplus
}
#endif

#endif
