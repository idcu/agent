#ifndef IDCU_COMMON_STRING_BUF_H
#define IDCU_COMMON_STRING_BUF_H

#include "error_code.h"
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    char* data;
    size_t size;
    size_t capacity;
} idcu_StringBuf;

int  idcu_string_buf_init(idcu_StringBuf* buf, size_t initial_capacity);
void idcu_string_buf_destroy(idcu_StringBuf* buf);

int  idcu_string_buf_append(idcu_StringBuf* buf, const char* str);
int  idcu_string_buf_append_n(idcu_StringBuf* buf, const char* str, size_t len);
int  idcu_string_buf_append_char(idcu_StringBuf* buf, char c);
int  idcu_string_buf_append_format(idcu_StringBuf* buf, const char* format, ...);
int  idcu_string_buf_vappend_format(idcu_StringBuf* buf, const char* format, va_list args);

int  idcu_string_buf_clear(idcu_StringBuf* buf);
int  idcu_string_buf_reserve(idcu_StringBuf* buf, size_t new_capacity);

const char* idcu_string_buf_data(const idcu_StringBuf* buf);
size_t      idcu_string_buf_size(const idcu_StringBuf* buf);
size_t      idcu_string_buf_capacity(const idcu_StringBuf* buf);
bool        idcu_string_buf_empty(const idcu_StringBuf* buf);

#ifdef __cplusplus
}
#endif

#endif
