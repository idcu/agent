#include "idcu/os/os.h"

char* idcu_env_get(const char* name) { (void)name; return NULL; }
int idcu_env_set(const char* name, const char* value) { (void)name; (void)value; return -1; }
int idcu_env_del(const char* name) { (void)name; return -1; }
