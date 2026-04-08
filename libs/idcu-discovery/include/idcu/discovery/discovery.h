#ifndef IDCU_DISCOVERY_H
#define IDCU_DISCOVERY_H

#include "idcu/common/error_code.h"
#include "idcu/distributed/distributed.h"
#include "idcu/network/network_layer.h"

#include <stdint.h>

#define IDCU_DISCOVERY_PORT                  9999
#define IDCU_DISCOVERY_MULTICAST_ADDR        "239.255.0.1"
#define IDCU_DISCOVERY_BROADCAST_INTERVAL_MS 3000
#define IDCU_DISCOVERY_TIMEOUT_MS            10000
#define IDCU_DISCOVERY_MAX_PACKET_SIZE       512
#define IDCU_DISCOVERY_MAGIC                 0x49444355  // "IDCU"
#define IDCU_DISCOVERY_VERSION               2
#define IDCU_DISCOVERY_MAX_ETCD_ENDPOINTS    8
#define IDCU_DISCOVERY_ETCD_KEY_PREFIX       "/idcu/discovery/"
#define IDCU_DISCOVERY_ETCD_TTL              15

typedef enum
{
    IDCU_DISCOVERY_PROTOCOL_BROADCAST = 0,
    IDCU_DISCOVERY_PROTOCOL_MDNS,
    IDCU_DISCOVERY_PROTOCOL_ETCD
} idcu_DiscoveryProtocol;

typedef void (*idcu_NodeDiscoveredHandler)(void* user_data, idcu_NodeInfo* node);
typedef void (*idcu_NodeLostHandler)(void* user_data, idcu_NodeInfo* node);

typedef struct
{
    char endpoints[IDCU_DISCOVERY_MAX_ETCD_ENDPOINTS][256];
    int  endpoint_count;
    char username[64];
    char password[64];
    int  use_tls;
    char ca_cert[512];
} idcu_EtcdConfig;

typedef struct
{
    char     service_name[64];
    char     service_domain[64];
    uint16_t service_port;
} idcu_MdnsConfig;

typedef struct
{
    idcu_DiscoveryProtocol protocol;
    idcu_EtcdConfig        etcd;
    idcu_MdnsConfig        mdns;
} idcu_DiscoveryConfig;

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint64_t node_id;
    char     name[IDCU_NODE_NAME_MAX];
    char     address[IDCU_NODE_ADDR_MAX];
    uint16_t port;
    uint64_t timestamp;
    uint64_t load;
    uint32_t status;
} idcu_DiscoveryPacket;

typedef struct
{
    idcu_DistributedNode*      dist_node;
    idcu_NodeDiscoveredHandler discovered_handler;
    void*                      discovered_user_data;
    idcu_NodeLostHandler       lost_handler;
    void*                      lost_user_data;
    idcu_NetworkSocket         socket;
    int                        running;
    uint64_t                   last_broadcast_time;
    idcu_DiscoveryConfig       config;
    void*                      protocol_data;
} idcu_NodeDiscovery;

int  idcu_node_discovery_init(idcu_NodeDiscovery* disc, idcu_DistributedNode* dist_node);
int  idcu_node_discovery_init_with_config(idcu_NodeDiscovery* disc, idcu_DistributedNode* dist_node,
                                          const idcu_DiscoveryConfig* config);
void idcu_node_discovery_destroy(idcu_NodeDiscovery* disc);
int idcu_node_discovery_set_discovered_handler(idcu_NodeDiscovery*        disc,
                                               idcu_NodeDiscoveredHandler handler, void* user_data);
int idcu_node_discovery_set_lost_handler(idcu_NodeDiscovery* disc, idcu_NodeLostHandler handler,
                                         void* user_data);
int idcu_node_discovery_set_protocol(idcu_NodeDiscovery* disc, idcu_DiscoveryProtocol protocol);
int idcu_node_discovery_configure_etcd(idcu_NodeDiscovery* disc, const idcu_EtcdConfig* config);
int idcu_node_discovery_configure_mdns(idcu_NodeDiscovery* disc, const idcu_MdnsConfig* config);
int idcu_node_discovery_start(idcu_NodeDiscovery* disc);
int idcu_node_discovery_stop(idcu_NodeDiscovery* disc);
int idcu_node_discovery_poll(idcu_NodeDiscovery* disc);
void idcu_discovery_config_init(idcu_DiscoveryConfig* config);

#endif
