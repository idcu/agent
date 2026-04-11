#ifndef IDCU_TOOLS_PERFORMANCE_PROFILER_H
#define IDCU_TOOLS_PERFORMANCE_PROFILER_H

#include <idcu/common/config.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* name;
    uint64_t call_count;
    uint64_t total_time_ns;
    uint64_t min_time_ns;
    uint64_t max_time_ns;
    uint64_t start_time_ns;
    int active;
} idcu_PerformanceMetric;

int idcu_performance_profiler_init(void);
void idcu_performance_profiler_destroy(void);

void idcu_performance_start(const char* name);
void idcu_performance_stop(const char* name);

void idcu_performance_profiler_print_report(void);
void idcu_performance_profiler_reset(void);

#ifdef IDCU_PROFILE
#define IDCU_PROFILE_SCOPE(name) \
    idcu_performance_start(name); \
    idcu_performance_stop(name)
#else
#define IDCU_PROFILE_SCOPE(name)
#endif

#ifdef __cplusplus
}
#endif

#endif
