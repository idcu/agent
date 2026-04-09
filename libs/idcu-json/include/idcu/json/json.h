#ifndef IDCU_JSON_JSON_H
#define IDCU_JSON_JSON_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/json/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int  idcu_json_parse(const char* json_str, idcu_JsonValue* result);

void idcu_json_free(idcu_JsonValue* value);

idcu_JsonType idcu_json_get_type(const idcu_JsonValue* value);

int idcu_json_get_bool(const idcu_JsonValue* value, int* out);

int idcu_json_get_int(const idcu_JsonValue* value, int64_t* out);

int idcu_json_get_double(const idcu_JsonValue* value, double* out);

int idcu_json_get_string(const idcu_JsonValue* value, const char** out);

size_t          idcu_json_array_size(const idcu_JsonArray* array);

idcu_JsonValue* idcu_json_array_get(const idcu_JsonArray* array, size_t index);

idcu_JsonValue* idcu_json_object_get(const idcu_JsonObject* object, const char* key);

int             idcu_json_object_has(const idcu_JsonObject* object, const char* key);

int   idcu_json_to_string(const idcu_JsonValue* value, char* buffer, size_t buffer_size);

char* idcu_json_to_string_alloc(const idcu_JsonValue* value);

#ifdef __cplusplus
}
#endif

#endif
