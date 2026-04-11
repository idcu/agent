#ifndef IDCU_MANAGEMENT_MANAGEMENT_H
#define IDCU_MANAGEMENT_MANAGEMENT_H

#include "idcu/common/error_code.h"
#include "idcu/management/types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_mgmt_init(idcu_Management_Context** ctx);
void idcu_mgmt_destroy(idcu_Management_Context* ctx);
int idcu_mgmt_execute(idcu_Management_Context* ctx, idcu_MgmtCommand cmd, const char* args, idcu_MgmtResponse* response);
void idcu_mgmt_response_destroy(idcu_MgmtResponse* response);
int idcu_mgmt_cli_start(int port);
void idcu_mgmt_cli_stop(void);
int idcu_mgmt_http_start(int port);
void idcu_mgmt_http_stop(void);

#ifdef __cplusplus
}
#endif

#endif
