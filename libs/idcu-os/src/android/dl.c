#include "idcu/os.h"

void* idcu_dlopen(const char* path) { (void)path; return NULL; }
void* idcu_dlsym(void* handle, const char* symbol) { (void)handle; (void)symbol; return NULL; }
int idcu_dlclose(void* handle) { (void)handle; return -1; }
const char* idcu_dlerror(void) { return NULL; }
