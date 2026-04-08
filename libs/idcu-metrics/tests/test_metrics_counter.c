#include "idcu/metrics/metrics.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    printf("Testing counter metric...\n");

    idcu_MetricsCollector collector;
    idcu_metrics_init(&collector);

    int ret =
        idcu_metrics_register(&collector, "test_counter", "A test counter", IDCU_METRIC_COUNTER);
    assert(ret == IDCU_ERR_OK);
    printf("✓ Counter metric registered\n");

    ret = idcu_metrics_inc(&collector, "test_counter", 1);
    assert(ret == IDCU_ERR_OK);
    uint64_t val = idcu_metrics_get(&collector, "test_counter");
    assert(val == 1);
    printf("✓ Counter incremented to 1\n");

    ret = idcu_metrics_inc(&collector, "test_counter", 5);
    assert(ret == IDCU_ERR_OK);
    val = idcu_metrics_get(&collector, "test_counter");
    assert(val == 6);
    printf("✓ Counter incremented by 5 to 6\n");

    idcu_metrics_destroy(&collector);
    printf("All tests passed!\n");
    return 0;
}
