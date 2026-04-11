#ifndef IDCU_SANDBOX_SANDBOX_H
#define IDCU_SANDBOX_SANDBOX_H

#include <idcu/common/error_code.h>
#include <idcu/common/vector.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_SANDBOX_PERM_READ = 1 << 0,
    IDCU_SANDBOX_PERM_WRITE = 1 << 1,
    IDCU_SANDBOX_PERM_EXECUTE = 1 << 2,
    IDCU_SANDBOX_PERM_NETWORK = 1 << 3,
    IDCU_SANDBOX_PERM_FILESYSTEM = 1 << 4,
    IDCU_SANDBOX_PERM_PROCESS = 1 << 5
} idcu_SandboxPermission;

typedef struct
{
    char name[128];
    idcu_SandboxPermission permissions;
    char allowed_paths[2048];
    char allowed_network[2048];
    int enabled;
} idcu_SandboxPolicy;

typedef struct
{
    idcu_Vector policies;
    idcu_Mutex lock;
    int initialized;
    int enforcing;
} idcu_Sandbox;

int  idcu_sandbox_init(idcu_Sandbox* sandbox, int enforcing);
void idcu_sandbox_destroy(idcu_Sandbox* sandbox);

int  idcu_sandbox_add_policy(idcu_Sandbox* sandbox, const idcu_SandboxPolicy* policy);
int  idcu_sandbox_remove_policy(idcu_Sandbox* sandbox, const char* name);
idcu_SandboxPolicy* idcu_sandbox_get_policy(idcu_Sandbox* sandbox, const char* name);

int  idcu_sandbox_check_permission(idcu_Sandbox* sandbox, const char* policy_name, idcu_SandboxPermission permission);
int  idcu_sandbox_check_path_access(idcu_Sandbox* sandbox, const char* policy_name, const char* path);
int  idcu_sandbox_check_network_access(idcu_Sandbox* sandbox, const char* policy_name, const char* host);

int  idcu_sandbox_set_enforcing(idcu_Sandbox* sandbox, int enforcing);
int  idcu_sandbox_is_enforcing(idcu_Sandbox* sandbox);

#ifdef __cplusplus
}
#endif

#endif
