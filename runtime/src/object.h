#ifndef object_h
#define object_h

#include "../../common/src/linked_list.h"
#include <stdbool.h>
#include <stdint.h>

#define BOOL_VALUE(B) (Value) {.type=V_BOOL, .as.boolean=B}
#define INT_VALUE(I) (Value) {.type=V_INT, .as.integer=I}
#define STR_VALUE(S) (Value) {.type=V_STR, .as.string=S}
#define OBJ_VALUE(O) (Value) {.type=V_OBJ, .as.object=(Obj*) O}

#define AS_ARRAY(v) (ObjArray*) v.as.object

typedef enum {
    V_BOOL,
    V_INT,
    V_STR,
    V_NUM,
    V_OBJ
} ValueType;

typedef enum {
    O_STRUCT,
    O_NATFN,
    O_FN,
    O_ARRAY
} ObjectType;

typedef struct {
    ValueType type;
    char* name;
    char* qualifier;
    bool isArray;
} Type;

typedef struct {
    Type type;
    uint16_t name;
} Field;

DEFINE_LINKED_LIST(Field)

typedef struct {
    uint16_t id;
    FieldList* fields;
} StructDefinition;

typedef struct Obj {
    ObjectType type;
    struct Obj* next;
    bool isMarked;
} Obj;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        long integer;
        double number;
        char* string;
        Obj* object;
    } as;
} Value;

typedef struct {
    uint16_t constantId; // unrelated to variable id!
    uint16_t level;
    Value value;
} Variable;

DEFINE_LINKED_LIST(Variable)

typedef void (*NatFn)(int noArgs, Value* args);

typedef struct {
    Obj obj;
    const char* name;
    NatFn fn;
} ObjNatFn;

typedef struct {
    Obj obj;
    uint32_t address;
    uint16_t id;
    uint8_t argCount;
    FieldList* params;
} ObjFn;

typedef struct {
    Obj obj;
    Type componentType;
    uint16_t size;
    Value* vals;
} ObjArray;

typedef struct {
    uint16_t name;
    Value val;
} FieldValue;

typedef struct {
    Obj obj;
    uint16_t name;
    uint16_t size;
    FieldValue* fvs;
} ObjStruct;

void debug_value(Value v);
void debug_object(Obj* obj);

Value get_field(ObjStruct* str, uint16_t name);
void set_field(ObjStruct* str, uint16_t name, Value val);

#endif