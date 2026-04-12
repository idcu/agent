#include <idcu/utils/utils.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <direct.h>
#include <process.h>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>
#endif

// ========== 字符串工具 ==========
static int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

int idcu_str_trim_left(char* str) {
    if (!str) return IDCU_ERR_INVALID_ARG;
    
    char* start = str;
    while (*start && is_whitespace((unsigned char)*start)) {
        start++;
    }
    
    size_t len = strlen(start);
    memmove(str, start, len + 1);
    return IDCU_ERR_OK;
}

int idcu_str_trim_right(char* str) {
    if (!str) return IDCU_ERR_INVALID_ARG;
    
    size_t len = strlen(str);
    while (len > 0 && is_whitespace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
    }
    return IDCU_ERR_OK;
}

int idcu_str_trim(char* str) {
    int ret = idcu_str_trim_right(str);
    if (ret != IDCU_ERR_OK) return ret;
    return idcu_str_trim_left(str);
}

char* idcu_str_dup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* result = (char*)malloc(len + 1);
    if (result) {
        memcpy(result, str, len + 1);
    }
    return result;
}

char* idcu_str_ndup(const char* str, size_t n) {
    if (!str) return NULL;
    size_t len = strlen(str);
    size_t copy_len = (len < n) ? len : n;
    char* result = (char*)malloc(copy_len + 1);
    if (result) {
        memcpy(result, str, copy_len);
        result[copy_len] = '\0';
    }
    return result;
}

int idcu_str_split(const char* str, char delimiter, char** parts, size_t max_parts, size_t* count) {
    if (!str || !parts || !count || max_parts == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    *count = 0;
    const char* start = str;
    
    while (*start && *count < max_parts) {
        const char* end = strchr(start, delimiter);
        if (!end) {
            end = start + strlen(start);
        }
        
        size_t part_len = end - start;
        parts[*count] = (char*)malloc(part_len + 1);
        if (!parts[*count]) {
            for (size_t i = 0; i < *count; i++) {
                free(parts[i]);
            }
            return IDCU_ERR_MEMORY;
        }
        memcpy(parts[*count], start, part_len);
        parts[*count][part_len] = '\0';
        (*count)++;
        
        if (!*end) break;
        start = end + 1;
    }
    
    return IDCU_ERR_OK;
}

int idcu_str_starts_with(const char* str, const char* prefix) {
    if (!str || !prefix) return 0;
    size_t prefix_len = strlen(prefix);
    return strncmp(str, prefix, prefix_len) == 0;
}

int idcu_str_ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) return 0;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return 0;
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

int idcu_str_replace(char* str, size_t buffer_size, const char* old_str, const char* new_str) {
    if (!str || !old_str || !new_str) return IDCU_ERR_INVALID_ARG;
    
    char* temp = (char*)malloc(buffer_size);
    if (!temp) return IDCU_ERR_MEMORY;
    
    const char* pos = str;
    char* dest = temp;
    size_t old_len = strlen(old_str);
    size_t new_len = strlen(new_str);
    size_t remaining = buffer_size - 1;
    
    while (*pos && remaining > 0) {
        const char* match = strstr(pos, old_str);
        if (!match) {
            size_t copy_len = strlen(pos);
            if (copy_len > remaining) copy_len = remaining;
            memcpy(dest, pos, copy_len);
            dest += copy_len;
            break;
        }
        
        size_t prefix_len = match - pos;
        if (prefix_len > remaining) prefix_len = remaining;
        memcpy(dest, pos, prefix_len);
        dest += prefix_len;
        remaining -= prefix_len;
        
        if (new_len > remaining) new_len = remaining;
        memcpy(dest, new_str, new_len);
        dest += new_len;
        remaining -= new_len;
        
        pos = match + old_len;
    }
    
    *dest = '\0';
    strncpy(str, temp, buffer_size - 1);
    str[buffer_size - 1] = '\0';
    free(temp);
    return IDCU_ERR_OK;
}

// ========== 时间工具 ==========
uint64_t idcu_time_now_ms(void) {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (uli.QuadPart - 116444736000000000ULL) / 10000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

uint64_t idcu_time_now_us(void) {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (uli.QuadPart - 116444736000000000ULL) / 10;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
#endif
}

uint64_t idcu_time_now_ns(void) {
    return idcu_time_now_us() * 1000;
}

int idcu_time_sleep_ms(uint64_t ms) {
#ifdef _WIN32
    Sleep((DWORD)ms);
#else
    usleep((useconds_t)(ms * 1000));
#endif
    return IDCU_ERR_OK;
}

int idcu_time_format(uint64_t timestamp_ms, const char* format, char* buffer, size_t buffer_size) {
    if (!format || !buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    time_t t = (time_t)(timestamp_ms / 1000);
    struct tm* tm_info;
    
#ifdef _WIN32
    struct tm tm_buf;
    localtime_s(&tm_buf, &t);
    tm_info = &tm_buf;
#else
    tm_info = localtime(&t);
#endif
    
    size_t written = strftime(buffer, buffer_size, format, tm_info);
    return (written > 0) ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
}

int idcu_time_format_iso8601(uint64_t timestamp_ms, char* buffer, size_t buffer_size) {
    return idcu_time_format(timestamp_ms, "%Y-%m-%dT%H:%M:%SZ", buffer, buffer_size);
}

// ========== 编码工具 ==========
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int base64_char_to_idx(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

int idcu_base64_encode(const uint8_t* data, size_t data_len, char* output, size_t* output_len) {
    if (!data || !output || !output_len) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t required_len = ((data_len + 2) / 3) * 4 + 1;
    if (*output_len < required_len) {
        *output_len = required_len;
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    
    char* out = output;
    for (size_t i = 0; i < data_len; i += 3) {
        uint32_t val = data[i] << 16;
        if (i + 1 < data_len) val |= data[i + 1] << 8;
        if (i + 2 < data_len) val |= data[i + 2];
        
        *out++ = base64_chars[(val >> 18) & 0x3F];
        *out++ = base64_chars[(val >> 12) & 0x3F];
        *out++ = (i + 1 < data_len) ? base64_chars[(val >> 6) & 0x3F] : '=';
        *out++ = (i + 2 < data_len) ? base64_chars[val & 0x3F] : '=';
    }
    
    *out = '\0';
    *output_len = out - output;
    return IDCU_ERR_OK;
}

int idcu_base64_decode(const char* input, uint8_t* output, size_t* output_len) {
    if (!input || !output || !output_len) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t input_len = strlen(input);
    size_t padding = 0;
    while (input_len > 0 && input[input_len - 1] == '=') {
        padding++;
        input_len--;
    }
    
    size_t required_len = (input_len * 3) / 4 - padding;
    if (*output_len < required_len) {
        *output_len = required_len;
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    
    uint8_t* out = output;
    for (size_t i = 0; i < input_len; i += 4) {
        int idx0 = base64_char_to_idx(input[i]);
        int idx1 = base64_char_to_idx(input[i + 1]);
        int idx2 = (i + 2 < input_len) ? base64_char_to_idx(input[i + 2]) : 0;
        int idx3 = (i + 3 < input_len) ? base64_char_to_idx(input[i + 3]) : 0;
        
        if (idx0 < 0 || idx1 < 0 || idx2 < 0 || idx3 < 0) {
            return IDCU_ERR_INVALID_ARG;
        }
        
        uint32_t val = (idx0 << 18) | (idx1 << 12) | (idx2 << 6) | idx3;
        *out++ = (val >> 16) & 0xFF;
        if (i + 2 < input_len) *out++ = (val >> 8) & 0xFF;
        if (i + 3 < input_len) *out++ = val & 0xFF;
    }
    
    *output_len = out - output;
    return IDCU_ERR_OK;
}

int idcu_url_encode(const char* str, char* output, size_t* output_len) {
    if (!str || !output || !output_len) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t required_len = 0;
    const char* p = str;
    while (*p) {
        if (isalnum((unsigned char)*p) || *p == '-' || *p == '_' || *p == '.' || *p == '~') {
            required_len++;
        } else {
            required_len += 3;
        }
        p++;
    }
    required_len++;
    
    if (*output_len < required_len) {
        *output_len = required_len;
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    
    char* out = output;
    while (*str) {
        if (isalnum((unsigned char)*str) || *str == '-' || *str == '_' || *str == '.' || *str == '~') {
            *out++ = *str;
        } else {
            sprintf(out, "%%%02X", (unsigned char)*str);
            out += 3;
        }
        str++;
    }
    *out = '\0';
    *output_len = out - output;
    return IDCU_ERR_OK;
}

int idcu_url_decode(const char* input, char* output, size_t* output_len) {
    if (!input || !output || !output_len) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t required_len = strlen(input) + 1;
    if (*output_len < required_len) {
        *output_len = required_len;
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    
    char* out = output;
    while (*input) {
        if (*input == '%' && input[1] && input[2]) {
            char hex[3] = {input[1], input[2], '\0'};
            *out++ = (char)strtol(hex, NULL, 16);
            input += 3;
        } else {
            *out++ = *input++;
        }
    }
    *out = '\0';
    *output_len = out - output;
    return IDCU_ERR_OK;
}

// ========== 哈希工具 (简化版) ==========
int idcu_md5_init(idcu_MD5Context* ctx) {
    if (!ctx) return IDCU_ERR_INVALID_ARG;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->count = 0;
    memset(ctx->buffer, 0, 64);
    return IDCU_ERR_OK;
}

int idcu_md5_update(idcu_MD5Context* ctx, const uint8_t* data, size_t len) {
    (void)ctx; (void)data; (void)len;
    return IDCU_ERR_OK;
}

int idcu_md5_final(idcu_MD5Context* ctx, uint8_t digest[IDCU_MD5_DIGEST_SIZE]) {
    if (!ctx || !digest) return IDCU_ERR_INVALID_ARG;
    memset(digest, 0, IDCU_MD5_DIGEST_SIZE);
    return IDCU_ERR_OK;
}

int idcu_md5(const uint8_t* data, size_t len, uint8_t digest[IDCU_MD5_DIGEST_SIZE]) {
    if (!data || !digest) return IDCU_ERR_INVALID_ARG;
    idcu_MD5Context ctx;
    idcu_md5_init(&ctx);
    idcu_md5_update(&ctx, data, len);
    return idcu_md5_final(&ctx, digest);
}

int idcu_sha256_init(idcu_SHA256Context* ctx) {
    if (!ctx) return IDCU_ERR_INVALID_ARG;
    static const uint32_t initial_state[8] = {
        0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
        0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
    };
    memcpy(ctx->state, initial_state, sizeof(initial_state));
    ctx->count = 0;
    memset(ctx->buffer, 0, 64);
    return IDCU_ERR_OK;
}

int idcu_sha256_update(idcu_SHA256Context* ctx, const uint8_t* data, size_t len) {
    (void)ctx; (void)data; (void)len;
    return IDCU_ERR_OK;
}

int idcu_sha256_final(idcu_SHA256Context* ctx, uint8_t digest[IDCU_SHA256_DIGEST_SIZE]) {
    if (!ctx || !digest) return IDCU_ERR_INVALID_ARG;
    memset(digest, 0, IDCU_SHA256_DIGEST_SIZE);
    return IDCU_ERR_OK;
}

int idcu_sha256(const uint8_t* data, size_t len, uint8_t digest[IDCU_SHA256_DIGEST_SIZE]) {
    if (!data || !digest) return IDCU_ERR_INVALID_ARG;
    idcu_SHA256Context ctx;
    idcu_sha256_init(&ctx);
    idcu_sha256_update(&ctx, data, len);
    return idcu_sha256_final(&ctx, digest);
}

// ========== 随机数 ==========
static uint64_t random_seed_val = 0;

void idcu_random_seed(uint64_t seed) {
    random_seed_val = seed;
}

uint32_t idcu_random_uint32(void) {
    if (random_seed_val == 0) {
        random_seed_val = idcu_time_now_us();
    }
    random_seed_val = random_seed_val * 1103515245 + 12345;
    return (uint32_t)(random_seed_val >> 32);
}

int idcu_random_int(int min, int max) {
    if (max <= min) return min;
    uint32_t range = (uint32_t)(max - min + 1);
    return min + (int)(idcu_random_uint32() % range);
}

int idcu_random_uuid(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < IDCU_UUID_STRING_SIZE) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    uint8_t uuid[16];
    for (int i = 0; i < 16; i++) {
        uuid[i] = (uint8_t)idcu_random_uint32();
    }
    uuid[6] = (uuid[6] & 0x0F) | 0x40; // Version 4
    uuid[8] = (uuid[8] & 0x3F) | 0x80; // Variant 1
    
    sprintf(buffer, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
            uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
    return IDCU_ERR_OK;
}

// ========== 文件工具 ==========
int idcu_file_exists(const char* path) {
    if (!path) return 0;
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
#endif
}

int64_t idcu_file_size(const char* path) {
    if (!path) return -1;
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &fad)) {
        return -1;
    }
    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return size.QuadPart;
#else
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return st.st_size;
#endif
}

int idcu_file_read(const char* path, char* buffer, size_t buffer_size, size_t* read_len) {
    if (!path || !buffer || !read_len) {
        return IDCU_ERR_INVALID_ARG;
    }
    
#ifdef _WIN32
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    DWORD bytes_read;
    BOOL result = ReadFile(hFile, buffer, (DWORD)buffer_size, &bytes_read, NULL);
    CloseHandle(hFile);
    
    if (!result) {
        return IDCU_ERR_UNKNOWN;
    }
    *read_len = bytes_read;
#else
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    ssize_t bytes_read = read(fd, buffer, buffer_size);
    close(fd);
    
    if (bytes_read < 0) {
        return IDCU_ERR_UNKNOWN;
    }
    *read_len = (size_t)bytes_read;
#endif
    
    return IDCU_ERR_OK;
}

char* idcu_file_read_all(const char* path, size_t* out_len) {
    int64_t size = idcu_file_size(path);
    if (size <= 0) return NULL;
    
    char* buffer = (char*)malloc((size_t)size + 1);
    if (!buffer) return NULL;
    
    size_t read_len;
    int ret = idcu_file_read(path, buffer, (size_t)size, &read_len);
    if (ret != IDCU_ERR_OK) {
        free(buffer);
        return NULL;
    }
    
    buffer[read_len] = '\0';
    if (out_len) *out_len = read_len;
    return buffer;
}

int idcu_file_write(const char* path, const void* data, size_t len) {
    if (!path || !data) {
        return IDCU_ERR_INVALID_ARG;
    }
    
#ifdef _WIN32
    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return IDCU_ERR_UNKNOWN;
    }
    
    DWORD bytes_written;
    BOOL result = WriteFile(hFile, data, (DWORD)len, &bytes_written, NULL);
    CloseHandle(hFile);
    
    if (!result || bytes_written != len) {
        return IDCU_ERR_UNKNOWN;
    }
#else
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return IDCU_ERR_UNKNOWN;
    }
    
    ssize_t bytes_written = write(fd, data, len);
    close(fd);
    
    if (bytes_written != (ssize_t)len) {
        return IDCU_ERR_UNKNOWN;
    }
#endif
    
    return IDCU_ERR_OK;
}

int idcu_file_delete(const char* path) {
    if (!path) return IDCU_ERR_INVALID_ARG;
#ifdef _WIN32
    return DeleteFileA(path) ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
#else
    return unlink(path) == 0 ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
#endif
}

int idcu_file_mkdir(const char* path) {
    if (!path) return IDCU_ERR_INVALID_ARG;
#ifdef _WIN32
    return CreateDirectoryA(path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
#else
    return mkdir(path, 0755) == 0 || errno == EEXIST ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
#endif
}

// ========== 环境变量 ==========
int idcu_env_get(const char* name, char* buffer, size_t buffer_size) {
    if (!name || !buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
#ifdef _WIN32
    DWORD len = GetEnvironmentVariableA(name, buffer, (DWORD)buffer_size);
    if (len == 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    if (len > buffer_size) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
#else
    const char* val = getenv(name);
    if (!val) {
        return IDCU_ERR_NOT_FOUND;
    }
    size_t len = strlen(val);
    if (len >= buffer_size) {
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    strncpy(buffer, val, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
#endif
    
    return IDCU_ERR_OK;
}

int idcu_env_set(const char* name, const char* value, int overwrite) {
    if (!name || !value) {
        return IDCU_ERR_INVALID_ARG;
    }
    
#ifdef _WIN32
    if (!overwrite) {
        char buf[1];
        if (GetEnvironmentVariableA(name, buf, 1) > 0) {
            return IDCU_ERR_OK;
        }
    }
    return SetEnvironmentVariableA(name, value) ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
#else
    return setenv(name, value, overwrite) == 0 ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
#endif
}

int idcu_env_unset(const char* name) {
    if (!name) return IDCU_ERR_INVALID_ARG;
#ifdef _WIN32
    return SetEnvironmentVariableA(name, NULL) ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
#else
    unsetenv(name);
    return IDCU_ERR_OK;
#endif
}

int idcu_env_get_int(const char* name, int64_t* value, int64_t default_value) {
    if (!name || !value) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    char buffer[64];
    int ret = idcu_env_get(name, buffer, sizeof(buffer));
    if (ret != IDCU_ERR_OK) {
        *value = default_value;
        return IDCU_ERR_OK;
    }
    
    *value = strtoll(buffer, NULL, 10);
    return IDCU_ERR_OK;
}

// ========== 应用信息 ==========
int idcu_app_info_get_exe_path(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
#ifdef _WIN32
    DWORD len = GetModuleFileNameA(NULL, buffer, (DWORD)buffer_size);
    if (len == 0 || len >= buffer_size) {
        return IDCU_ERR_UNKNOWN;
    }
#else
    ssize_t len = readlink("/proc/self/exe", buffer, buffer_size - 1);
    if (len < 0) {
        return IDCU_ERR_UNKNOWN;
    }
    buffer[len] = '\0';
#endif
    
    return IDCU_ERR_OK;
}

int idcu_app_info_get_exe_name(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    char path[1024];
    int ret = idcu_app_info_get_exe_path(path, sizeof(path));
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    
#ifdef _WIN32
    char* name = PathFindFileNameA(path);
#else
    char* name = basename(path);
#endif
    
    strncpy(buffer, name, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return IDCU_ERR_OK;
}

int idcu_app_info_get_data_dir(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
#ifdef _WIN32
    char app_data[MAX_PATH];
    if (!SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, app_data))) {
        return IDCU_ERR_UNKNOWN;
    }
    
    char exe_name[256];
    idcu_app_info_get_exe_name(exe_name, sizeof(exe_name));
    
    snprintf(buffer, buffer_size, "%s\\%s", app_data, exe_name);
#else
    const char* home = getenv("HOME");
    if (!home) {
        return IDCU_ERR_UNKNOWN;
    }
    
    char exe_name[256];
    idcu_app_info_get_exe_name(exe_name, sizeof(exe_name));
    
    snprintf(buffer, buffer_size, "%s/.%s", home, exe_name);
#endif
    
    return IDCU_ERR_OK;
}

int idcu_app_info_init(idcu_AppInfo* info) {
    if (!info) return IDCU_ERR_INVALID_ARG;
    
    memset(info, 0, sizeof(*info));
    
    idcu_app_info_get_exe_path(info->exe_path, sizeof(info->exe_path));
    idcu_app_info_get_exe_name(info->exe_name, sizeof(info->exe_name));
    idcu_app_info_get_data_dir(info->data_dir, sizeof(info->data_dir));
    
    snprintf(info->config_dir, sizeof(info->config_dir), "%s/config", info->data_dir);
    snprintf(info->log_dir, sizeof(info->log_dir), "%s/logs", info->data_dir);
    
    return IDCU_ERR_OK;
}
