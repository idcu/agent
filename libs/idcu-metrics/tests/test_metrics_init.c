#include "idcu/metrics/metrics.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    printf("Testing metrics initialization...\n");

    idcu_MetricsCollector collector;
    int ret = idcu_metrics_init(&collector);
    assert(ret == IDCU_ERR_OK);
    printf("✓ Metrics collector initialized successfully\n");

    idcu_metrics_destroy(&collector);
    printf("✓ Metrics collector destroyed successfully\n");

    printf("All tests passed!\n");
    return 0;
}
