#ifndef IDCU_YAML_YAML_H
#define IDCU_YAML_YAML_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/yaml/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int  idcu_yaml_parse(const char* yaml_str, idcu_YamlValue* result);

void idcu_yaml_free(idcu_YamlValue* value);

idcu_YamlType idcu_yaml_get_type(const idcu_YamlValue* value);

int idcu_yaml_get_bool(const idcu_YamlValue* value, int* out);

int idcu_yaml_get_int(const idcu_YamlValue* value, int64_t* out);

int idcu_yaml_get_double(const idcu_YamlValue* value, double* out);

int idcu_yaml_get_string(const idcu_YamlValue* value, const char** out);

size_t           idcu_yaml_sequence_size(const idcu_YamlSequence* seq);

idcu_YamlValue*  idcu_yaml_sequence_get(const idcu_YamlSequence* seq, size_t index);

idcu_YamlValue*  idcu_yaml_mapping_get(const idcu_YamlMapping* map, const char* key);

int              idcu_yaml_mapping_has(const idcu_YamlMapping* map, const char* key);

int   idcu_yaml_to_string(const idcu_YamlValue* value, char* buffer, size_t buffer_size);

char* idcu_yaml_to_string_alloc(const idcu_YamlValue* value);

int idcu_yaml_to_json(const idcu_YamlValue* yaml_value, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
