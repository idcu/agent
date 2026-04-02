#ifndef CONTEXT_H
#define CONTEXT_H

#define CONTEXT_BUF 8192

typedef struct {
    char task[2048];
    char output[CONTEXT_BUF];
} Context;

void context_init(Context *ctx, const char *task);

#endif
