#include "idcu/conn_pool/conn_pool.h"
#include "idcu/log/log.h"
#include &lt;stdio.h&gt;

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_ConnectionPool pool;
    int ret = idcu_connection_pool_init(&amp;pool, "test_pool", IDCU_NET_PROTO_TCP);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Test failed: init failed\n");
        return 1;
    }
    
    uint32_t total, in_use, idle;
    ret = idcu_connection_pool_get_stats(&amp;pool, &amp;total, &amp;in_use, &amp;idle);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Test failed: get stats failed\n");
        idcu_connection_pool_destroy(&amp;pool);
        return 1;
    }
    
    if (total != 0) {
        printf("Test failed: total should be 0\n");
        idcu_connection_pool_destroy(&amp;pool);
        return 1;
    }
    
    idcu_connection_pool_destroy(&amp;pool);
    
    printf("Test passed\n");
    return 0;
}
