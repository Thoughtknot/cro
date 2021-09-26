#ifndef dynamic_array_h__
#define dynamic_array_h__

#include "stdlib.h"

#define DEFINE_DYNAMIC_ARRAY(TYPE) \
    typedef struct {\
        int count;\
        int capacity;\
        TYPE* data;\
    } TYPE##Array;\
    TYPE##Array* make_array_##TYPE();\
    void write_##TYPE(TYPE##Array* arr, TYPE data);\

#define MAKE_DYNAMIC_ARRAY(TYPE) \
    \
    TYPE##Array* make_array_##TYPE() { \
        TYPE##Array* arr = malloc(sizeof(*arr)); \
        arr->count = 0; \
        arr->capacity = 0; \
        arr->data = NULL; \
        return arr; \
    } \
    \
    void free_array_##TYPE(TYPE##Array* arr) { \
        arr->count = 0; \
        arr->capacity = 0; \
        free(arr->data); \
        arr->data = NULL; \
    } \
    \
    void write_##TYPE(TYPE##Array* arr, TYPE data) { \
        if (arr->capacity < arr->count + 1) { \
            int old_cap = arr->capacity; \
            arr->capacity = old_cap < 8 ? 8 : (old_cap * 2); \
            arr->data = realloc(arr->data, sizeof(TYPE) * arr->capacity); \
        } \
        arr->data[arr->count] = data; \
        arr->count++; \
    }

#endif