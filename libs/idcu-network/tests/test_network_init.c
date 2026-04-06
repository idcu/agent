#include "idcu/network/network_layer.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("Testing network initialization...\n");

    int ret = idcu_network_init();
    assert(ret == IDCU_ERR_OK);
    printf("✓ Network initialized successfully\n");

    idcu_network_cleanup();
    printf("✓ Network cleanup completed\n");

    printf("All tests passed!\n");
    return 0;
}
