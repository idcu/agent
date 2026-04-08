#ifndef IDCU_INTEGRATIONS_BASIC_LIBS_H
#define IDCU_INTEGRATIONS_BASIC_LIBS_H

#include "idcu/common/error_code.h"
#include "idcu/config/config.h"
#include "idcu/healthcheck/healthcheck.h"
#include "idcu/http_client/http_client.h"
#include "idcu/http_server/http_server.h"
#include "idcu/json/json.h"
#include "idcu/log/log.h"
#include "idcu/metrics/metrics.h"
#include "idcu/network/network_layer.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int           initialized;
        char          config_file[1024];
        char          log_file[256];
        idcu_LogLevel log_level;
    } idcu_BasicLibs;

    int  idcu_basic_libs_init(idcu_BasicLibs* libs, const char* config_file, const char* log_file,
                              idcu_LogLevel log_level);
    void idcu_basic_libs_destroy(idcu_BasicLibs* libs);

    int idcu_basic_libs_enable_config(int enable);
    int idcu_basic_libs_enable_log(int enable);
    int idcu_basic_libs_enable_json(int enable);
    int idcu_basic_libs_enable_metrics(int enable);
    int idcu_basic_libs_enable_healthcheck(int enable);
    int idcu_basic_libs_enable_http_server(int enable);
    int idcu_basic_libs_enable_http_client(int enable);
    int idcu_basic_libs_enable_network(int enable);

    int idcu_basic_libs_config_reload(void);
    int idcu_basic_libs_config_save(const char* file_path);

    int           idcu_basic_libs_log_set_level(idcu_LogLevel level);
    idcu_LogLevel idcu_basic_libs_log_get_level(void);

#ifdef __cplusplus
}
#endif

#endif  // IDCU_INTEGRATIONS_BASIC_LIBS_H
