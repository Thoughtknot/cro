#ifndef vm_h
#define vm_h
#include "../../common/src/types.h"
#include "../../common/src/linked_list.h"
#include "../../common/src/dynamic_array.h"
#include <stdlib.h>
#include <stdint.h>
#include "object.h"
#include "gc.h"

#define STACK_MAX 256

typedef struct {
    int constantId;
    TypeCode tp;
    Value value;
} Constant;

typedef enum {
    VM_SUCCESS,
    VM_ERROR
} VmStatus;

DEFINE_LINKED_LIST(ObjFn);
DEFINE_DYNAMIC_ARRAY(Constant)

typedef struct {
    uint16_t level;
    VariableList* variableStack;
} Scope;

DEFINE_LINKED_LIST(StructDefinition)

typedef struct {
    Value stack[STACK_MAX];
    Value* sp;
    
    int callsp;
    uint32_t callStack[STACK_MAX];
    int scopep;
    Scope scope[STACK_MAX];

    uint32_t pc;
    uint32_t size;
    char* program;
    VariableList* globalVariables;

    StructDefinitionList* structs;
    ObjFnList* fnTable;
    ConstantArray* constantArray;

    GC gc;
} VM;

void init_vm(VM* vm, char* program);
void call(VM* vm, ObjFn* fn, uint8_t argCount);
uint16_t push_constant_string(VM* vm, char* c);
VmStatus run(VM* vm);

#endif