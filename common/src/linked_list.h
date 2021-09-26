#ifndef linked_list_h__
#define linked_list_h__

#define DEFINE_LINKED_LIST(TYPE) \
    typedef struct TYPE##Node { \
        TYPE* current; \
        struct TYPE##Node* next; \
        struct TYPE##Node* prev; \
    } TYPE##Node; \
    typedef struct { \
        TYPE##Node* head; \
        TYPE##Node* tail; \
    } TYPE##List; \
    TYPE##List* make_##TYPE##_list(); \
    void add_##TYPE(TYPE##List* list, TYPE * value); \
    void remove_##TYPE(TYPE##List* list, TYPE * value);

#define MAKE_LINKED_LIST(TYPE) \
    TYPE##List* make_##TYPE##_list() { \
        TYPE##List* list = (TYPE##List*) malloc(sizeof(*list)); \
        list->head = NULL; \
        list->tail = NULL; \
        return list; \
    } \
    \
    TYPE##Node* make_##TYPE##_node(TYPE* value) { \
        TYPE##Node* node = (TYPE##Node*) malloc(sizeof(*node)); \
        node->current = value; \
        node->next = NULL; \
        node->prev = NULL; \
        return node; \
    } \
    \
    void add_##TYPE(TYPE##List* list, TYPE * value) { \
        TYPE##Node * node = make_##TYPE##_node(value);\
        if (list->head == NULL) { \
            list->head = node; \
            list->tail = node; \
        } else { \
            node->prev = list->tail; \
            list->tail->next = node; \
            list->tail = node; \
        } \
    } \
    \
    TYPE* pop_##TYPE(TYPE##List* list) { \
        TYPE##Node * node = list->tail; \
        if (node != NULL) { \
            TYPE * val = node->current; \
            node->current = NULL; \
            list->tail = node->prev; \
            if (node->prev == NULL) { \
                list->head = NULL; \
            } else {\
                list->tail->next = NULL;\
            }\
            /*free(node);*/ \
            return val; \
        } \
        return NULL;\
    } \

#endif