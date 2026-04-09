#ifndef IDCU_COMMON_LINKED_LIST_H
#define IDCU_COMMON_LINKED_LIST_H

#include "error_code.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct idcu_LinkedListNode
{
    struct idcu_LinkedListNode* prev;
    struct idcu_LinkedListNode* next;
    void* data;
} idcu_LinkedListNode;

typedef struct
{
    idcu_LinkedListNode* head;
    idcu_LinkedListNode* tail;
    size_t size;
    size_t element_size;
    void (*element_dtor)(void*);
} idcu_LinkedList;

typedef struct
{
    idcu_LinkedList* list;
    idcu_LinkedListNode* current;
} idcu_LinkedListIterator;

int  idcu_linked_list_init(idcu_LinkedList* list, size_t element_size);
int  idcu_linked_list_init_with_dtor(idcu_LinkedList* list, size_t element_size,
                                       void (*element_dtor)(void*));
void idcu_linked_list_destroy(idcu_LinkedList* list);

int  idcu_linked_list_push_front(idcu_LinkedList* list, const void* element);
int  idcu_linked_list_push_back(idcu_LinkedList* list, const void* element);
int  idcu_linked_list_pop_front(idcu_LinkedList* list, void* out_element);
int  idcu_linked_list_pop_back(idcu_LinkedList* list, void* out_element);

void* idcu_linked_list_front(const idcu_LinkedList* list);
void* idcu_linked_list_back(const idcu_LinkedList* list);

size_t idcu_linked_list_size(const idcu_LinkedList* list);
bool   idcu_linked_list_empty(const idcu_LinkedList* list);
void   idcu_linked_list_clear(idcu_LinkedList* list);

void idcu_linked_list_iterator_init(idcu_LinkedListIterator* iter, idcu_LinkedList* list);
bool idcu_linked_list_iterator_next(idcu_LinkedListIterator* iter, void** out_value);

#define IDCU_LINKED_LIST_FOR_EACH(list, var, iter)                      \
    for (idcu_linked_list_iterator_init(&(iter), (list));               \
         idcu_linked_list_iterator_next(&(iter), (void**)&(var));)

#ifdef __cplusplus
}
#endif

#endif
