#include "idcu/conn_pool/conn_pool.h"
#include "idcu/log/log.h"
#include &lt; stdio.h & gt;

int main() {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    idcu_ConnectionPool pool;
    idcu_connection_pool_init(&amp; pool, "example_pool", IDCU_NET_PROTO_TCP);

    printf("Connection pool initialized\n");

    uint32_t total, in_use, idle;
    idcu_connection_pool_get_stats(&amp; pool, &amp; total, &amp; in_use, &amp; idle);
    printf("Stats: total=%u, in_use=%u, idle=%u\n", total, in_use, idle);

    idcu_connection_pool_destroy(&amp; pool);
    printf("Connection pool destroyed\n");

    return 0;
}
