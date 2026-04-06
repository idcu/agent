#include "idcu/common/string_buf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#define IDCU_STRBUF_DEFAULT_CAPACITY 32
#define IDCU_STRBUF_GROWTH_FACTOR 2

int idcu_strbuf_init(idcu_StringBuf* buf, size_t initial_capacity)
{
    if (!buf) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (initial_capacity == 0) {
        initial_capacity = IDCU_STRBUF_DEFAULT_CAPACITY;
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

void idcu_strbuf_destroy(idcu_StringBuf* buf)
{
    if (!buf) return;
    free(buf->data);
    buf->data = NULL;
    buf->size = 0;
    buf->capacity = 0;
}

static int idcu_strbuf_ensure_capacity(idcu_StringBuf* buf, size_t needed)
{
    if (buf->size + needed < buf->capacity) {
        return IDCU_ERR_OK;
    }

    size_t new_capacity = buf->capacity;
    while (buf->size + needed >= new_capacity) {
        new_capacity *= IDCU_STRBUF_GROWTH_FACTOR;
    }

    char* new_data = (char*)realloc(buf->data, new_capacity);
    if (!new_data) {
        return IDCU_ERR_NO_MEMORY;
    }

    buf->data = new_data;
    buf->capacity = new_capacity;
    return IDCU_ERR_OK;
}

int idcu_strbuf_append(idcu_StringBuf* buf, const char* str)
{
    if (!buf || !str) {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t len = strlen(str);
    int ret = idcu_strbuf_ensure_capacity(buf, len + 1);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    memcpy(buf->data + buf->size, str, len + 1);
    buf->size += len;
    return IDCU_ERR_OK;
}

int idcu_strbuf_append_n(idcu_StringBuf* buf, const char* str, size_t n)
{
    if (!buf || !str) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_strbuf_ensure_capacity(buf, n + 1);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    memcpy(buf->data + buf->size, str, n);
    buf->size += n;
    buf->data[buf->size] = '\0';
    return IDCU_ERR_OK;
}

int idcu_strbuf_append_char(idcu_StringBuf* buf, char c)
{
    if (!buf) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_strbuf_ensure_capacity(buf, 2);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    buf->data[buf->size++] = c;
    buf->data[buf->size] = '\0';
    return IDCU_ERR_OK;
}

int idcu_strbuf_append_int(idcu_StringBuf* buf, int64_t value)
{
    if (!buf) {
        return IDCU_ERR_INVALID_PARAM;
    }

    char temp[32];
    snprintf(temp, sizeof(temp), "%" PRId64, value);
    return idcu_strbuf_append(buf, temp);
}

int idcu_strbuf_append_double(idcu_StringBuf* buf, double value, int precision)
{
    if (!buf) {
        return IDCU_ERR_INVALID_PARAM;
    }

    char temp[64];
    snprintf(temp, sizeof(temp), "%.*f", precision, value);
    return idcu_strbuf_append(buf, temp);
}

int idcu_strbuf_append_format(idcu_StringBuf* buf, const char* format, ...)
{
    if (!buf || !format) {
        return IDCU_ERR_INVALID_PARAM;
    }

    va_list args;
    va_start(args, format);
    int ret = idcu_strbuf_append_format_v(buf, format, args);
    va_end(args);
    return ret;
}

int idcu_strbuf_append_format_v(idcu_StringBuf* buf, const char* format, va_list args)
{
    if (!buf || !format) {
        return IDCU_ERR_INVALID_PARAM;
    }

    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (needed < 0) {
        return IDCU_ERR_GENERAL;
    }

    int ret = idcu_strbuf_ensure_capacity(buf, needed + 1);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    int written = vsnprintf(buf->data + buf->size, buf->capacity - buf->size, format, args);
    if (written < 0) {
        return IDCU_ERR_GENERAL;
    }

    buf->size += written;
    return IDCU_ERR_OK;
}

int idcu_strbuf_insert(idcu_StringBuf* buf, size_t pos, const char* str)
{
    if (!buf || !str || pos > buf->size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t len = strlen(str);
    int ret = idcu_strbuf_ensure_capacity(buf, len + 1);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    memmove(buf->data + pos + len, buf->data + pos, buf->size - pos + 1);
    memcpy(buf->data + pos, str, len);
    buf->size += len;
    return IDCU_ERR_OK;
}

int idcu_strbuf_remove(idcu_StringBuf* buf, size_t pos, size_t len)
{
    if (!buf || pos >= buf->size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (pos + len > buf->size) {
        len = buf->size - pos;
    }

    memmove(buf->data + pos, buf->data + pos + len, buf->size - pos - len + 1);
    buf->size -= len;
    return IDCU_ERR_OK;
}

int idcu_strbuf_clear(idcu_StringBuf* buf)
{
    if (!buf) {
        return IDCU_ERR_INVALID_PARAM;
    }

    buf->size = 0;
    if (buf->data) {
        buf->data[0] = '\0';
    }
    return IDCU_ERR_OK;
}

int idcu_strbuf_reserve(idcu_StringBuf* buf, size_t new_capacity)
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

int idcu_strbuf_resize(idcu_StringBuf* buf, size_t new_size, char fill_char)
{
    if (!buf) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (new_size < buf->size) {
        buf->size = new_size;
        buf->data[buf->size] = '\0';
    } else if (new_size > buf->size) {
        int ret = idcu_strbuf_ensure_capacity(buf, new_size - buf->size);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        for (size_t i = buf->size; i < new_size; ++i) {
            buf->data[i] = fill_char;
        }
        buf->size = new_size;
        buf->data[buf->size] = '\0';
    }
    return IDCU_ERR_OK;
}

const char* idcu_strbuf_data(const idcu_StringBuf* buf)
{
    return buf ? buf->data : NULL;
}

char* idcu_strbuf_detach(idcu_StringBuf* buf)
{
    if (!buf) {
        return NULL;
    }
    char* result = buf->data;
    buf->data = NULL;
    buf->size = 0;
    buf->capacity = 0;
    return result;
}

size_t idcu_strbuf_size(const idcu_StringBuf* buf)
{
    return buf ? buf->size : 0;
}

size_t idcu_strbuf_capacity(const idcu_StringBuf* buf)
{
    return buf ? buf->capacity : 0;
}

int idcu_strbuf_empty(const idcu_StringBuf* buf)
{
    return buf ? (buf->size == 0) : 1;
}

int idcu_strbuf_compare(const idcu_StringBuf* buf, const char* str)
{
    if (!buf || !str) {
        return (buf ? 1 : -1);
    }
    return strcmp(buf->data, str);
}

int idcu_strbuf_find(const idcu_StringBuf* buf, const char* substr, size_t start_pos)
{
    if (!buf || !substr || start_pos >= buf->size) {
        return -1;
    }

    char* found = strstr(buf->data + start_pos, substr);
    if (!found) {
        return -1;
    }
    return (int)(found - buf->data);
}

int idcu_strbuf_replace(idcu_StringBuf* buf, const char* old_str, const char* new_str)
{
    if (!buf || !old_str || !new_str) {
        return IDCU_ERR_INVALID_PARAM;
    }

    size_t old_len = strlen(old_str);
    size_t new_len = strlen(new_str);
    int pos = 0;
    int count = 0;

    while ((pos = idcu_strbuf_find(buf, old_str, pos)) >= 0) {
        int ret = idcu_strbuf_remove(buf, pos, old_len);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        ret = idcu_strbuf_insert(buf, pos, new_str);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
        pos += new_len;
        count++;
    }

    return count;
}
