#include <stdio.h>
#include "idcu/common/vector.h"
#include "idcu/common/string_buf.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/option.h"
#include "idcu/common/linked_list.h"
#include "idcu/common/error_code.h"

typedef struct {
    int id;
    char name[32];
    double price;
} Product;

void example_vector(void)
{
    printf("\n=== Vector Example ===\n");

    idcu_Vector vec;
    int ret = idcu_vector_init(&vec, sizeof(Product), 4);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init vector\n");
        return;
    }

    Product p1 = {1, "Apple", 5.99};
    Product p2 = {2, "Banana", 3.49};
    Product p3 = {3, "Orange", 4.29};

    idcu_vector_push_back(&vec, &p1);
    idcu_vector_push_back(&vec, &p2);
    idcu_vector_push_back(&vec, &p3);

    printf("Vector size: %zu\n", idcu_vector_size(&vec));

    IDCU_VECTOR_FOR_EACH(&vec, Product, product, i) {
        printf("  [%zu] ID: %d, Name: %s, Price: %.2f\n",
               i, product->id, product->name, product->price);
    }

    Product p4 = {4, "Grape", 6.99};
    idcu_vector_insert(&vec, 1, &p4);
    printf("\nAfter inserting at index 1:\n");

    IDCU_VECTOR_FOR_EACH(&vec, Product, product, i) {
        printf("  [%zu] ID: %d, Name: %s, Price: %.2f\n",
               i, product->id, product->name, product->price);
    }

    idcu_vector_destroy(&vec);
}

void example_string_buf(void)
{
    printf("\n=== String Buffer Example ===\n");

    idcu_StringBuf buf;
    int ret = idcu_strbuf_init(&buf, 32);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init string buf\n");
        return;
    }

    idcu_strbuf_append(&buf, "Hello, ");
    idcu_strbuf_append(&buf, "World!");
    idcu_strbuf_append_char(&buf, ' ');
    idcu_strbuf_append_int(&buf, 2024);
    idcu_strbuf_append(&buf, " - ");
    idcu_strbuf_append_double(&buf, 3.14159, 2);

    printf("String: %s\n", idcu_strbuf_data(&buf));
    printf("Size: %zu\n", idcu_strbuf_size(&buf));

    idcu_strbuf_clear(&buf);
    idcu_strbuf_append_format(&buf, "User: %s, Age: %d, Score: %.1f",
                              "Alice", 25, 95.5);
    printf("\nFormatted: %s\n", idcu_strbuf_data(&buf));

    idcu_strbuf_replace(&buf, "Alice", "Bob");
    printf("After replace: %s\n", idcu_strbuf_data(&buf));

    int pos = idcu_strbuf_find(&buf, "Age", 0);
    printf("Found 'Age' at position: %d\n", pos);

    char* detached = idcu_strbuf_detach(&buf);
    printf("Detached string: %s\n", detached);
    free(detached);

    idcu_strbuf_destroy(&buf);
}

void example_hash_map(void)
{
    printf("\n=== Hash Map Example ===\n");

    idcu_HashMap map;
    int ret = idcu_hash_map_init(&map, sizeof(int), 8);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init hash map\n");
        return;
    }

    int age1 = 25;
    int age2 = 30;
    int age3 = 35;

    idcu_hash_map_insert(&map, "Alice", &age1);
    idcu_hash_map_insert(&map, "Bob", &age2);
    idcu_hash_map_insert(&map, "Charlie", &age3);

    printf("Map size: %zu\n", idcu_hash_map_size(&map));

    int age;
    if (idcu_hash_map_get(&map, "Bob", &age) == IDCU_ERR_OK) {
        printf("Bob's age: %d\n", age);
    }

    printf("Contains 'Alice': %s\n", idcu_hash_map_contains(&map, "Alice") ? "yes" : "no");

    printf("\nAll entries:\n");
    idcu_HashMapIterator iter;
    idcu_hash_map_iter_init(&iter, &map);
    const char* key;
    int* value;
    while (idcu_hash_map_iter_next(&iter, &key, (void**)&value)) {
        printf("  %s: %d\n", key, *value);
    }
    idcu_hash_map_iter_destroy(&iter);

    idcu_hash_map_remove(&map, "Charlie");
    printf("\nAfter removing 'Charlie', size: %zu\n", idcu_hash_map_size(&map));

    idcu_hash_map_destroy(&map);
}

void example_option(void)
{
    printf("\n=== Option Type Example ===\n");

    idcu_Option_int age_some = idcu_Some(int, 25);
    idcu_Option_int age_none = idcu_None(int);

    printf("age_some is some: %s\n", idcu_Option_is_some(int, &age_some) ? "yes" : "no");
    printf("age_none is none: %s\n", idcu_Option_is_none(int, &age_none) ? "yes" : "no");

    if (idcu_Option_is_some(int, &age_some)) {
        int age = idcu_Option_unwrap(int, &age_some);
        printf("Unwrapped age: %d\n", age);
    }

    int default_age = idcu_Option_unwrap_or(int, &age_none, 18);
    printf("Default age: %d\n", default_age);

    printf("\nUsing IDCU_OPTION_IF_LET macro:\n");
    IDCU_OPTION_IF_LET(int, age, age_some) {
        printf("Age inside if-let: %d\n", age);
    }

    idcu_Option_double price_some = idcu_Some(double, 19.99);
    double price = idcu_Option_unwrap_or(double, &price_some, 0.0);
    printf("\nPrice: %.2f\n", price);

    idcu_Option_void* ptr_some = idcu_Some(void*, (void*)0x1234);
    void* ptr = idcu_Option_unwrap_or(void*, &ptr_some, NULL);
    printf("Pointer: %p\n", ptr);
}

void example_linked_list(void)
{
    printf("\n=== Linked List Example ===\n");

    idcu_LinkedList list;
    int ret = idcu_linked_list_init(&list, sizeof(int));
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init linked list\n");
        return;
    }

    int val1 = 10;
    int val2 = 20;
    int val3 = 30;

    idcu_linked_list_push_back(&list, &val1);
    idcu_linked_list_push_back(&list, &val2);
    idcu_linked_list_push_back(&list, &val3);

    printf("List size: %zu\n", idcu_linked_list_size(&list));
    printf("Front: %d\n", *(int*)idcu_linked_list_front(&list));
    printf("Back: %d\n", *(int*)idcu_linked_list_back(&list));

    printf("\nForward traversal:\n");
    IDCU_LINKED_LIST_FOR_EACH(&list, int, item) {
        printf("  %d\n", *item);
    }

    int val4 = 5;
    idcu_linked_list_push_front(&list, &val4);
    printf("\nAfter push_front(5):\n");

    IDCU_LINKED_LIST_FOR_EACH(&list, int, item) {
        printf("  %d\n", *item);
    }

    int val5 = 15;
    idcu_linked_list_insert_after(&list, 1, &val5);
    printf("\nAfter insert_after(1, 15):\n");

    IDCU_LINKED_LIST_FOR_EACH(&list, int, item) {
        printf("  %d\n", *item);
    }

    printf("\nReverse traversal:\n");
    IDCU_LINKED_LIST_FOR_EACH_REVERSE(&list, int, item) {
        printf("  %d\n", *item);
    }

    int popped;
    idcu_linked_list_pop_front(&list, &popped);
    printf("\nPopped front: %d\n", popped);

    idcu_linked_list_pop_back(&list, &popped);
    printf("Popped back: %d\n", popped);

    printf("\nFinal list:\n");
    IDCU_LINKED_LIST_FOR_EACH(&list, int, item) {
        printf("  %d\n", *item);
    }

    idcu_linked_list_destroy(&list);
}

void example_combined(void)
{
    printf("\n=== Combined Example ===\n");

    idcu_Vector names;
    idcu_vector_init(&names, sizeof(char*), 4);

    const char* names_arr[] = {"Alice", "Bob", "Charlie", "David"};
    for (int i = 0; i < 4; i++) {
        idcu_vector_push_back(&names, &names_arr[i]);
    }

    idcu_StringBuf html;
    idcu_strbuf_init(&html, 64);

    idcu_strbuf_append(&html, "<ul>\n");
    IDCU_VECTOR_FOR_EACH(&names, char*, name, i) {
        idcu_strbuf_append_format(&html, "  <li>%s</li>\n", *name);
    }
    idcu_strbuf_append(&html, "</ul>");

    printf("Generated HTML:\n%s\n", idcu_strbuf_data(&html));

    idcu_strbuf_destroy(&html);
    idcu_vector_destroy(&names);
}

int main(void)
{
    printf("=== C Productivity Tools Demo ===\n");

    example_vector();
    example_string_buf();
    example_hash_map();
    example_option();
    example_linked_list();
    example_combined();

    printf("\n=== Demo Complete ===\n");
    return 0;
}
