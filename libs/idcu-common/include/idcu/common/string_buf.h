#ifndef IDCU_COMMON_STRING_BUF_H
#define IDCU_COMMON_STRING_BUF_H

#include "error_code.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} idcu_StringBuf;

int idcu_strbuf_init(idcu_StringBuf* buf, size_t initial_capacity);
void idcu_strbuf_destroy(idcu_StringBuf* buf);

int idcu_strbuf_append(idcu_StringBuf* buf, const char* str);
int idcu_strbuf_append_n(idcu_StringBuf* buf, const char* str, size_t n);
int idcu_strbuf_append_char(idcu_StringBuf* buf, char c);
int idcu_strbuf_append_int(idcu_StringBuf* buf, int64_t value);
int idcu_strbuf_append_double(idcu_StringBuf* buf, double value, int precision);
int idcu_strbuf_append_format(idcu_StringBuf* buf, const char* format, ...);
int idcu_strbuf_append_format_v(idcu_StringBuf* buf, const char* format, va_list args);

int idcu_strbuf_insert(idcu_StringBuf* buf, size_t pos, const char* str);
int idcu_strbuf_remove(idcu_StringBuf* buf, size_t pos, size_t len);

int idcu_strbuf_clear(idcu_StringBuf* buf);
int idcu_strbuf_reserve(idcu_StringBuf* buf, size_t new_capacity);
int idcu_strbuf_resize(idcu_StringBuf* buf, size_t new_size, char fill_char);

const char* idcu_strbuf_data(const idcu_StringBuf* buf);
char* idcu_strbuf_detach(idcu_StringBuf* buf);
size_t idcu_strbuf_size(const idcu_StringBuf* buf);
size_t idcu_strbuf_capacity(const idcu_StringBuf* buf);
int idcu_strbuf_empty(const idcu_StringBuf* buf);

int idcu_strbuf_compare(const idcu_StringBuf* buf, const char* str);
int idcu_strbuf_find(const idcu_StringBuf* buf, const char* substr, size_t start_pos);
int idcu_strbuf_replace(idcu_StringBuf* buf, const char* old_str, const char* new_str);

#ifdef __cplusplus
}
#endif

#endif
