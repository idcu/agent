#include "idcu/common/string_buf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define IDCU_STRING_BUF_DEFAULT_CAPACITY 32
#define IDCU_STRING_BUF_GROWTH_FACTOR 2

int idcu_string_buf_init(idcu_StringBuf* buf, size_t initial_capacity)
{
    if (!buf) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (initial_capacity == 0) {
        initial_capacity = IDCU_STRING_BUF_DEFAULT_CAPACITY;
    }

    buf->data = (char*)malloc(initial_capacity);
    if (!buf->data) {
        return IDCU_ERR_NO_MEMORY;
    }

    buf->data[0] = '\0';
    buf->size = 0;
    buf->capacity = initial_capacity;

    return IDCU_ERR_OK;
}

void idcu_string_buf_destroy(idcu_StringBuf* buf)
{
    if (!buf)
        return;
    free(buf->data);
    buf->data = NULL;
    buf->size = 0;
    buf->capacity = 0;
}

static int idcu_string_buf_ensure_capacity(idcu_StringBuf* buf, size_t required)
{
    if (required + 1 <= buf->capacity) {
        return IDCU_ERR_OK;
    }

    size_t new_capacity = buf->capacity * IDCU_STRING_BUF_GROWTH_FACTOR;
    while (new_capacity < required + 1) {
        new_capacity *= IDCU_STRING_BUF_GROWTH_FACTOR;
    }

    char* new_data = (char*)realloc(buf->data, new_capacity);
    if (!new_data) {
        return IDCU_ERR_NO_MEMORY;
    }

    buf->data = new_data;
    buf->capacity = new_capacity;
    return IDCU_ERR_OK;
}

int idcu_string_buf_append(idcu_StringBuf* buf, const char* str)
{
    if (!buf || !str) {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t len = strlen(str);
    return idcu_string_buf_append_n(buf, str, len);
}

int idcu_string_buf_append_n(idcu_StringBuf* buf, const char* str, size_t len)
{
    if (!buf || (!str && len > 0)) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_string_buf_ensure_capacity(buf, buf->size + len);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    memcpy(buf->data + buf->size, str, len);
    buf->size += len;
    buf->data[buf->size] = '\0';

    return IDCU_ERR_OK;
}

int idcu_string_buf_append_char(idcu_StringBuf* buf, char c)
{
    if (!buf) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_string_buf_ensure_capacity(buf, buf->size + 1);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    buf->data[buf->size++] = c;
    buf->data[buf->size] = '\0';

    return IDCU_ERR_OK;
}

int idcu_string_buf_vappend_format(idcu_StringBuf* buf, const char* format, va_list args)
{
    if (!buf || !format) {
        return IDCU_ERR_INVALID_PARAM;
    }

    va_list args_copy;
    va_copy(args_copy, args);

    int len = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (len < 0) {
        return IDCU_ERR_GENERAL;
    }

    int ret = idcu_string_buf_ensure_capacity(buf, buf->size + (size_t)len);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    vsnprintf(buf->data + buf->size, (size_t)(buf->capacity - buf->size), format, args);
    buf->size += (size_t)len;

    return IDCU_ERR_OK;
}

int idcu_string_buf_append_format(idcu_StringBuf* buf, const char* format, ...)
{
    if (!buf || !format) {
        return IDCU_ERR_INVALID_PARAM;
    }

    va_list args;
    va_start(args, format);
    int ret = idcu_string_buf_vappend_format(buf, format, args);
    va_end(args);

    return ret;
}

int idcu_string_buf_clear(idcu_StringBuf* buf)
{
    if (!buf) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (buf->data) {
        buf->data[0] = '\0';
    }
    buf->size = 0;
    return IDCU_ERR_OK;
}

int idcu_string_buf_reserve(idcu_StringBuf* buf, size_t new_capacity)
{
    if (!buf) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (new_capacity <= buf->capacity) {
        return IDCU_ERR_OK;
    }
    char* new_data = (char*)realloc(buf->data, new_capacity);
    if (!new_data) {
        return IDCU_ERR_NO_MEMORY;
    }
    buf->data = new_data;
    buf->capacity = new_capacity;
    return IDCU_ERR_OK;
}

const char* idcu_string_buf_data(const idcu_StringBuf* buf) { return buf ? buf->data : NULL; }
size_t idcu_string_buf_size(const idcu_StringBuf* buf) { return buf ? buf->size : 0; }
size_t idcu_string_buf_capacity(const idcu_StringBuf* buf) { return buf ? buf->capacity : 0; }
bool idcu_string_buf_empty(const idcu_StringBuf* buf) { return buf ? (buf->size == 0) : true; }
