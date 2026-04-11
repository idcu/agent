/**
 * @file example_yaml.c
 * @brief YAML parsing and serialization example
 */

#include <idcu/yaml/yaml.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== YAML Example ===\n\n");

    // Example 1: Parse YAML from string
    printf("1. Parsing YAML from string\n");
    const char* yaml_str = 
        "name: John Doe\n"
        "age: 30\n"
        "is_student: true\n"
        "grades:\n"
        "  - 90\n"
        "  - 85\n"
        "  - 95\n"
        "address:\n"
        "  city: New York\n"
        "  zipcode: \"10001\"\n";

    idcu_YamlValue* root = NULL;
    int ret = idcu_yaml_parse(yaml_str, &root);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to parse YAML\n");
        return 1;
    }

    printf("   YAML parsed successfully\n\n");

    // Example 2: Get values from YAML
    printf("2. Extracting values\n");
    
    // Get string
    const char* name = NULL;
    idcu_yaml_get_string(idcu_yaml_mapping_get(root, "name"), &name);
    printf("   Name: %s\n", name);

    // Get integer
    int64_t age = 0;
    idcu_yaml_get_int(idcu_yaml_mapping_get(root, "age"), &age);
    printf("   Age: %" PRId64 "\n", age);

    // Get boolean
    int is_student = 0;
    idcu_yaml_get_bool(idcu_yaml_mapping_get(root, "is_student"), &is_student);
    printf("   Is student: %s\n", is_student ? "yes" : "no");

    // Get sequence
    idcu_YamlValue* grades = idcu_yaml_mapping_get(root, "grades");
    size_t grades_count = idcu_yaml_sequence_size(grades);
    printf("   Grades count: %zu\n", grades_count);
    for (size_t i = 0; i < grades_count; i++) {
        int64_t grade = 0;
        idcu_yaml_get_int(idcu_yaml_sequence_get(grades, i), &grade);
        printf("     Grade %zu: %" PRId64 "\n", i, grade);
    }

    // Get nested mapping
    idcu_YamlValue* address = idcu_yaml_mapping_get(root, "address");
    const char* city = NULL;
    idcu_yaml_get_string(idcu_yaml_mapping_get(address, "city"), &city);
    printf("   City: %s\n", city);

    printf("\n");

    // Example 3: Serialize YAML to string
    printf("3. Serializing YAML\n");
    char* serialized = idcu_yaml_to_string_alloc(root);
    if (serialized) {
        printf("   Serialized YAML:\n%s\n", serialized);
        free(serialized);
    }

    // Example 4: Convert YAML to JSON
    printf("4. Converting YAML to JSON\n");
    char json_buffer[2048];
    ret = idcu_yaml_to_json(root, json_buffer, sizeof(json_buffer));
    if (ret == IDCU_ERR_OK) {
        printf("   YAML to JSON:\n%s\n", json_buffer);
    }

    // Free the YAML value
    idcu_yaml_free(root);

    printf("\n=== Example completed ===\n");
    return 0;
}
