/**
 * @file example_json.c
 * @brief JSON parsing and serialization example
 */

#include <idcu/json/json.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== JSON Example ===\n\n");

    // Example 1: Parse JSON from string
    printf("1. Parsing JSON from string\n");
    const char* json_str = 
        "{\n"
        "  \"name\": \"John Doe\",\n"
        "  \"age\": 30,\n"
        "  \"is_student\": true,\n"
        "  \"grades\": [90, 85, 95],\n"
        "  \"address\": {\n"
        "    \"city\": \"New York\",\n"
        "    \"zipcode\": \"10001\"\n"
        "  }\n"
        "}";

    idcu_JsonValue* root = NULL;
    int ret = idcu_json_parse(json_str, &root);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to parse JSON\n");
        return 1;
    }

    printf("   JSON parsed successfully\n\n");

    // Example 2: Get values from JSON
    printf("2. Extracting values\n");
    
    // Get string
    const char* name = NULL;
    idcu_json_get_string(idcu_json_object_get(root, "name"), &name);
    printf("   Name: %s\n", name);

    // Get integer
    int64_t age = 0;
    idcu_json_get_int(idcu_json_object_get(root, "age"), &age);
    printf("   Age: %" PRId64 "\n", age);

    // Get boolean
    int is_student = 0;
    idcu_json_get_bool(idcu_json_object_get(root, "is_student"), &is_student);
    printf("   Is student: %s\n", is_student ? "yes" : "no");

    // Get array
    idcu_JsonValue* grades = idcu_json_object_get(root, "grades");
    size_t grades_count = idcu_json_array_size(grades);
    printf("   Grades count: %zu\n", grades_count);
    for (size_t i = 0; i < grades_count; i++) {
        int64_t grade = 0;
        idcu_json_get_int(idcu_json_array_get(grades, i), &grade);
        printf("     Grade %zu: %" PRId64 "\n", i, grade);
    }

    // Get nested object
    idcu_JsonValue* address = idcu_json_object_get(root, "address");
    const char* city = NULL;
    idcu_json_get_string(idcu_json_object_get(address, "city"), &city);
    printf("   City: %s\n", city);

    printf("\n");

    // Example 3: Serialize JSON to string
    printf("3. Serializing JSON\n");
    char* serialized = idcu_json_to_string_pretty_alloc(root);
    if (serialized) {
        printf("   Pretty printed JSON:\n%s\n", serialized);
        free(serialized);
    }

    // Free the JSON value
    idcu_json_free(root);

    // Example 4: Build JSON programmatically
    printf("4. Building JSON programmatically\n");
    
    idcu_JsonValue* new_root = idcu_json_create_object();
    idcu_json_object_set(new_root, "product", idcu_json_create_string("Widget"));
    idcu_json_object_set(new_root, "price", idcu_json_create_double(19.99));
    idcu_json_object_set(new_root, "in_stock", idcu_json_create_bool(1));
    
    idcu_JsonValue* tags = idcu_json_create_array();
    idcu_json_array_push(tags, idcu_json_create_string("electronics"));
    idcu_json_array_push(tags, idcu_json_create_string("gadget"));
    idcu_json_object_set(new_root, "tags", tags);
    
    char* new_json = idcu_json_to_string_pretty_alloc(new_root);
    if (new_json) {
        printf("   Built JSON:\n%s\n", new_json);
        free(new_json);
    }
    
    idcu_json_free(new_root);

    printf("\n=== Example completed ===\n");
    return 0;
}
