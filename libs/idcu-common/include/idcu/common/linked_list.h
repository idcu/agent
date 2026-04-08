#ifndef IDCU_COMMON_LINKED_LIST_H
#define IDCU_COMMON_LINKED_LIST_H

#include "error_code.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct idcu_ListNode idcu_ListNode;

    struct idcu_ListNode
    {
        void*          data;
        idcu_ListNode* prev;
        idcu_ListNode* next;
    };

    typedef struct
    {
        idcu_ListNode* head;
        idcu_ListNode* tail;
        size_t         size;
        size_t         element_size;
        void (*element_dtor)(void*);
    } idcu_LinkedList;

    typedef struct
    {
        idcu_LinkedList* list;
        idcu_ListNode*   current;
        int              direction;
    } idcu_LinkedListIterator;

    int  idcu_linked_list_init(idcu_LinkedList* list, size_t element_size);
    int  idcu_linked_list_init_with_dtor(idcu_LinkedList* list, size_t element_size,
                                         void (*element_dtor)(void*));
    void idcu_linked_list_destroy(idcu_LinkedList* list);

    int idcu_linked_list_push_front(idcu_LinkedList* list, const void* element);
    int idcu_linked_list_push_back(idcu_LinkedList* list, const void* element);
    int idcu_linked_list_pop_front(idcu_LinkedList* list, void* out_element);
    int idcu_linked_list_pop_back(idcu_LinkedList* list, void* out_element);

    int idcu_linked_list_insert_before(idcu_LinkedList* list, size_t index, const void* element);
    int idcu_linked_list_insert_after(idcu_LinkedList* list, size_t index, const void* element);
    int idcu_linked_list_remove(idcu_LinkedList* list, size_t index);

    void* idcu_linked_list_get(const idcu_LinkedList* list, size_t index);
    void* idcu_linked_list_front(const idcu_LinkedList* list);
    void* idcu_linked_list_back(const idcu_LinkedList* list);

    size_t idcu_linked_list_size(const idcu_LinkedList* list);
    bool   idcu_linked_list_empty(const idcu_LinkedList* list);
    void   idcu_linked_list_clear(idcu_LinkedList* list);

    void idcu_linked_list_iter_init(idcu_LinkedListIterator* iter, idcu_LinkedList* list);
    void idcu_linked_list_iter_init_reverse(idcu_LinkedListIterator* iter, idcu_LinkedList* list);
    bool idcu_linked_list_iter_next(idcu_LinkedListIterator* iter, void** out_element);
    void idcu_linked_list_iter_destroy(idcu_LinkedListIterator* iter);

#define IDCU_LINKED_LIST_FOR_EACH(list, type, var)                                   \
    for (idcu_LinkedListIterator _iter = {0}, *_p_iter = &_iter;                     \
         (idcu_linked_list_iter_init(_p_iter, list), 1);                             \
         idcu_linked_list_iter_destroy(_p_iter), _p_iter = NULL)                     \
        for (type* var = NULL;                                                       \
             idcu_linked_list_iter_next(_p_iter, (void**)&var) && (var = (type*)1);) \
            for (type* var = (type*)1; var; var = NULL)                              \
                if (idcu_linked_list_iter_next(_p_iter, (void**)&var))

#define IDCU_LINKED_LIST_FOR_EACH_REVERSE(list, type, var)                           \
    for (idcu_LinkedListIterator _iter = {0}, *_p_iter = &_iter;                     \
         (idcu_linked_list_iter_init_reverse(_p_iter, list), 1);                     \
         idcu_linked_list_iter_destroy(_p_iter), _p_iter = NULL)                     \
        for (type* var = NULL;                                                       \
             idcu_linked_list_iter_next(_p_iter, (void**)&var) && (var = (type*)1);) \
            for (type* var = (type*)1; var; var = NULL)                              \
                if (idcu_linked_list_iter_next(_p_iter, (void**)&var))

#ifdef __cplusplus
}
#endif

#endif
