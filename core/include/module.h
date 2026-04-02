#ifndef MODULE_H
#define MODULE_H

#include "context.h"

#define MAX_MODULES 64

typedef struct Module {
    const char *name;
    int (*init)(void);
    int (*exec)(Context *ctx, void *arg);
    void *handle;
} Module;

void module_register(Module *m);
Module *module_find(const char *name);
int module_load_by_getter(const char *getter_name);

#endif
