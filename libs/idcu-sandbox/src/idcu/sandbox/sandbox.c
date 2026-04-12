#include <idcu/sandbox/sandbox.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void policy_dtor(void* element) {
    (void)element;
}

int idcu_sandbox_init(idcu_Sandbox* sandbox, int enforcing) {
    if (!sandbox) {
        return IDCU_ERR_INVALID_ARG;
    }
    memset(sandbox, 0, sizeof(idcu_Sandbox));
    int ret = idcu_vector_init_with_dtor(&sandbox->policies, sizeof(idcu_SandboxPolicy), 8, policy_dtor);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    ret = idcu_mutex_init(&sandbox->lock);
    if (ret != IDCU_ERR_OK) {
        idcu_vector_destroy(&sandbox->policies);
        return ret;
    }
    sandbox->enforcing = enforcing;
    sandbox->initialized = 1;
    return IDCU_ERR_OK;
}

void idcu_sandbox_destroy(idcu_Sandbox* sandbox) {
    if (!sandbox) {
        return;
    }
    idcu_mutex_lock(&sandbox->lock);
    idcu_vector_destroy(&sandbox->policies);
    idcu_Mutex lock_copy = sandbox->lock;
    sandbox->initialized = 0;
    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);
}

int idcu_sandbox_add_policy(idcu_Sandbox* sandbox, const idcu_SandboxPolicy* policy) {
    if (!sandbox || !sandbox->initialized || !policy) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&sandbox->lock);
    idcu_SandboxPolicy new_policy = *policy;
    new_policy.enabled = 1;
    int ret = idcu_vector_push_back(&sandbox->policies, &new_policy);
    idcu_mutex_unlock(&sandbox->lock);
    return ret;
}

int idcu_sandbox_remove_policy(idcu_Sandbox* sandbox, const char* name) {
    if (!sandbox || !sandbox->initialized || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&sandbox->lock);
    size_t count = idcu_vector_size(&sandbox->policies);
    for (size_t i = 0; i < count; i++) {
        idcu_SandboxPolicy* policy = (idcu_SandboxPolicy*)idcu_vector_at(&sandbox->policies, i);
        if (strcmp(policy->name, name) == 0) {
            idcu_vector_erase(&sandbox->policies, i);
            idcu_mutex_unlock(&sandbox->lock);
            return IDCU_ERR_OK;
        }
    }
    idcu_mutex_unlock(&sandbox->lock);
    return IDCU_ERR_NOT_FOUND;
}

idcu_SandboxPolicy* idcu_sandbox_get_policy(idcu_Sandbox* sandbox, const char* name) {
    if (!sandbox || !sandbox->initialized || !name) {
        return NULL;
    }
    idcu_mutex_lock(&sandbox->lock);
    size_t count = idcu_vector_size(&sandbox->policies);
    for (size_t i = 0; i < count; i++) {
        idcu_SandboxPolicy* policy = (idcu_SandboxPolicy*)idcu_vector_at(&sandbox->policies, i);
        if (strcmp(policy->name, name) == 0) {
            idcu_mutex_unlock(&sandbox->lock);
            return policy;
        }
    }
    idcu_mutex_unlock(&sandbox->lock);
    return NULL;
}

int idcu_sandbox_check_permission(idcu_Sandbox* sandbox, const char* policy_name, idcu_SandboxPermission permission) {
    if (!sandbox || !sandbox->initialized || !policy_name) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&sandbox->lock);
    size_t count = idcu_vector_size(&sandbox->policies);
    for (size_t i = 0; i < count; i++) {
        idcu_SandboxPolicy* policy = (idcu_SandboxPolicy*)idcu_vector_at(&sandbox->policies, i);
        if (strcmp(policy->name, policy_name) == 0 && policy->enabled) {
            if ((policy->permissions & permission) == permission) {
                idcu_mutex_unlock(&sandbox->lock);
                return IDCU_ERR_OK;
            }
            break;
        }
    }
    idcu_mutex_unlock(&sandbox->lock);
    return sandbox->enforcing ? IDCU_ERR_PERM_DENIED : IDCU_ERR_OK;
}

int idcu_sandbox_check_path_access(idcu_Sandbox* sandbox, const char* policy_name, const char* path) {
    if (!sandbox || !sandbox->initialized || !policy_name || !path) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&sandbox->lock);
    size_t count = idcu_vector_size(&sandbox->policies);
    for (size_t i = 0; i < count; i++) {
        idcu_SandboxPolicy* policy = (idcu_SandboxPolicy*)idcu_vector_at(&sandbox->policies, i);
        if (strcmp(policy->name, policy_name) == 0 && policy->enabled) {
            if (strlen(policy->allowed_paths) == 0 || strstr(policy->allowed_paths, path) != NULL) {
                idcu_mutex_unlock(&sandbox->lock);
                return IDCU_ERR_OK;
            }
            break;
        }
    }
    idcu_mutex_unlock(&sandbox->lock);
    return sandbox->enforcing ? IDCU_ERR_PERM_DENIED : IDCU_ERR_OK;
}

int idcu_sandbox_check_network_access(idcu_Sandbox* sandbox, const char* policy_name, const char* host) {
    if (!sandbox || !sandbox->initialized || !policy_name || !host) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&sandbox->lock);
    size_t count = idcu_vector_size(&sandbox->policies);
    for (size_t i = 0; i < count; i++) {
        idcu_SandboxPolicy* policy = (idcu_SandboxPolicy*)idcu_vector_at(&sandbox->policies, i);
        if (strcmp(policy->name, policy_name) == 0 && policy->enabled) {
            if (strlen(policy->allowed_network) == 0 || strstr(policy->allowed_network, host) != NULL) {
                idcu_mutex_unlock(&sandbox->lock);
                return IDCU_ERR_OK;
            }
            break;
        }
    }
    idcu_mutex_unlock(&sandbox->lock);
    return sandbox->enforcing ? IDCU_ERR_PERM_DENIED : IDCU_ERR_OK;
}

int idcu_sandbox_set_enforcing(idcu_Sandbox* sandbox, int enforcing) {
    if (!sandbox || !sandbox->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&sandbox->lock);
    sandbox->enforcing = enforcing;
    idcu_mutex_unlock(&sandbox->lock);
    return IDCU_ERR_OK;
}

int idcu_sandbox_is_enforcing(idcu_Sandbox* sandbox) {
    if (!sandbox || !sandbox->initialized) {
        return 0;
    }
    return sandbox->enforcing;
}
