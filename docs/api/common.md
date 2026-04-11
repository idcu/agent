# idcu-common API Documentation

Common utilities and data structures.

## Error Code System

### Error Codes

```c
typedef enum {
    IDCU_ERR_OK = 0,
    IDCU_ERR_UNKNOWN = -1,
    IDCU_ERR_MEMORY = -2,
    IDCU_ERR_INVALID_ARG = -3,
    IDCU_ERR_NOT_FOUND = -4,
    IDCU_ERR_ALREADY_EXISTS = -5,
    IDCU_ERR_PERMISSION_DENIED = -6,
    IDCU_ERR_TIMEOUT = -7,
    IDCU_ERR_IO = -8,
    IDCU_ERR_NETWORK = -9,
    IDCU_ERR_BUSY = -10,
    IDCU_ERR_CANCELLED = -11,
    IDCU_ERR_NOT_IMPLEMENTED = -12,
    IDCU_ERR_INTERNAL = -13
} idcu_ErrorCode;
```

### Error Message

```c
const char* idcu_error_message(idcu_ErrorCode code);
```

Returns a human-readable error message for the given error code.

## Vector (Dynamic Array)

### Vector Type

```c
typedef struct idcu_Vector idcu_Vector;
```

### Vector Functions

```c
idcu_ErrorCode idcu_vector_init(idcu_Vector** vector, size_t element_size);
void idcu_vector_destroy(idcu_Vector* vector);

idcu_ErrorCode idcu_vector_push(idcu_Vector* vector, const void* element);
idcu_ErrorCode idcu_vector_pop(idcu_Vector* vector, void* out_element);
void* idcu_vector_get(idcu_Vector* vector, size_t index);
idcu_ErrorCode idcu_vector_set(idcu_Vector* vector, size_t index, const void* element);
size_t idcu_vector_size(idcu_Vector* vector);
idcu_ErrorCode idcu_vector_reserve(idcu_Vector* vector, size_t capacity);
void idcu_vector_clear(idcu_Vector* vector);
```

### Vector Example

```c
idcu_Vector* vec = NULL;
idcu_vector_init(&vec, sizeof(int));

int value = 42;
idcu_vector_push(vec, &value);

value = 100;
idcu_vector_push(vec, &value);

int* retrieved = (int*)idcu_vector_get(vec, 0);
printf("First element: %d\n", *retrieved);

idcu_vector_destroy(vec);
```

## Hash Map

### Hash Map Type

```c
typedef struct idcu_HashMap idcu_HashMap;
```

### Hash Map Functions

```c
idcu_ErrorCode idcu_hash_map_init(idcu_HashMap** map);
void idcu_hash_map_destroy(idcu_HashMap* map);

idcu_ErrorCode idcu_hash_map_set(idcu_HashMap* map, const char* key, void* value);
idcu_ErrorCode idcu_hash_map_get(idcu_HashMap* map, const char* key, void** out_value);
idcu_ErrorCode idcu_hash_map_remove(idcu_HashMap* map, const char* key);
int idcu_hash_map_contains(idcu_HashMap* map, const char* key);
size_t idcu_hash_map_size(idcu_HashMap* map);
void idcu_hash_map_clear(idcu_HashMap* map);
```

### Hash Map Example

```c
idcu_HashMap* map = NULL;
idcu_hash_map_init(&map);

int value1 = 100;
int value2 = 200;

idcu_hash_map_set(map, "key1", &value1);
idcu_hash_map_set(map, "key2", &value2);

void* result = NULL;
idcu_hash_map_get(map, "key1", &result);
printf("key1: %d\n", *(int*)result);

idcu_hash_map_destroy(map);
```

## Mutex

### Mutex Type

```c
typedef struct idcu_Mutex idcu_Mutex;
```

### Mutex Functions

```c
idcu_ErrorCode idcu_mutex_init(idcu_Mutex** mutex);
void idcu_mutex_destroy(idcu_Mutex* mutex);

idcu_ErrorCode idcu_mutex_lock(idcu_Mutex* mutex);
idcu_ErrorCode idcu_mutex_unlock(idcu_Mutex* mutex);
idcu_ErrorCode idcu_mutex_trylock(idcu_Mutex* mutex, int* out_acquired);
```

### Mutex Example

```c
idcu_Mutex* mutex = NULL;
idcu_mutex_init(&mutex);

idcu_mutex_lock(mutex);
// Critical section
idcu_mutex_unlock(mutex);

idcu_mutex_destroy(mutex);
```

## Option Type

### Option Type

```c
typedef struct {
    int has_value;
    union {
        int int_value;
        double double_value;
        void* ptr_value;
    } value;
} idcu_Option;
```

### Option Functions

```c
idcu_Option idcu_option_none(void);
idcu_Option idcu_option_some_int(int value);
idcu_Option idcu_option_some_double(double value);
idcu_Option idcu_option_some_ptr(void* value);

int idcu_option_is_some(idcu_Option option);
int idcu_option_is_none(idcu_Option option);
```

### Option Example

```c
idcu_Option opt = idcu_option_some_int(42);

if (idcu_option_is_some(opt)) {
    printf("Value: %d\n", opt.value.int_value);
}

idcu_Option none = idcu_option_none();
if (idcu_option_is_none(none)) {
    printf("No value\n");
}
```
