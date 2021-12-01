#include "natives.h"
#include <stdio.h>

#define SYSTEM_FUNCTIONS 2

void system_print(int argc, Value* args) {
    printf("PRINT: [%d] {", argc);
    for (int i = 0; i < argc; i++) {
        debug_value(args[i]);
    }
    printf("}\n");
}

void initialize_natives(VM* vm) {
    printf("Creating natives\n");
    ObjStruct* str = (ObjStruct*) malloc(sizeof(*str));
    str->name = push_constant_string(vm, "system");
    str->size = SYSTEM_FUNCTIONS;
    str->obj.type = O_STRUCT;
    str->fvs = calloc(sizeof(FieldValue), SYSTEM_FUNCTIONS);
    {
        ObjNatFn* print = (ObjNatFn*) malloc(sizeof(*print));
        print->name = "print";
        print->obj.isMarked = false;
        print->obj.type = O_NATFN;
        print->fn = system_print;
        FieldValue fv = {.name=push_constant_string(vm, "print"), .val=OBJ_VALUE(print)};
        str->fvs[0] = fv;
    }

    Variable* var = (Variable*) malloc(sizeof(*var));
    var->constantId = str->name;
    var->level = 0;
    var->value = OBJ_VALUE(str);
    add_Variable(vm->globalVariables, var);
}