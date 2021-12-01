#include "vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "natives.h"

#define BINARY(OP) \
    Value l = pop_stack(vm); \
    Value r = pop_stack(vm); \
    if (l.type != r.type) \
        vm_error(vm, "Binary operation on non-matching types!"); \
    if (l.type == V_INT) \
        push_stack(vm, INT_VALUE(l.as.integer OP r.as.integer)); \

#define UNARY(OP) \
    Value l = pop_stack(vm); \
    if (l.type == V_INT) \
        push_stack(vm, INT_VALUE(OP l.as.integer)); \

#define CMP(OP) \
    Value l = pop_stack(vm); \
    Value r = pop_stack(vm); \
    if (l.type != r.type) \
        vm_error(vm, "Binary operation on non-matching types!"); \
    if (l.type == V_INT) \
        push_stack(vm, BOOL_VALUE(l.as.integer OP r.as.integer)); \
    else if (l.type == V_BOOL) \
        push_stack(vm, BOOL_VALUE(l.as.boolean OP r.as.boolean)); \


Constant get_constant(VM* vm, int id);
MAKE_DYNAMIC_ARRAY(Constant)
MAKE_LINKED_LIST(StructDefinition)
MAKE_LINKED_LIST(ObjFn)
MAKE_LINKED_LIST(Variable)

void init_vm(VM* vm, char* program) {
    vm->callsp = 0;
    vm->sp = vm->stack;
    vm->program = program;
    vm->size = strlen(program);
    vm->pc = 0;
    vm->fnTable = make_ObjFn_list();
    vm->constantArray = make_array_Constant();
    vm->structs = make_StructDefinition_list();
    init_gc(&vm->gc);
    vm->scopep = 0;
    vm->scope[vm->scopep] = (Scope) {.level = 0, .variableStack = make_Variable_list()};
    vm->globalVariables = make_Variable_list();
}

void print_type(Type type) {
    if (type.qualifier != NULL) {
        printf("%s ", type.qualifier);
    }
    switch (type.type) {
        case V_INT:
            printf("int");
            break;
        case V_NUM:
            printf("num");
            break;
        case V_BOOL:
            printf("bool");
            break;
        case V_STR:
            printf("str");
            break;
        case V_OBJ:
            printf("%s", type.name);
            break;
        default:
            printf("Unknown type %d", type.type);
            break;
    }
    if (type.isArray) printf("[]");
}

void debug_vm(VM* vm) {
    printf("VM[pc: %x]\n", vm->pc);
    printf("Constants: {\n");
    for (int i = 0; i < vm->constantArray->count; i++) {
        Constant val = vm->constantArray->data[i];
        printf("  {%d: tp %d, val ", val.constantId, val.tp);
        debug_value(val.value);
        printf("}\n");
    }
    printf("}\n");
    printf("Globals: {\n");
    VariableNode* gnode = vm->globalVariables->head;
    while (gnode != NULL) {
        printf("  {%d: ", gnode->current->constantId);
        debug_value(gnode->current->value);
        printf("}\n");
        gnode = gnode->next;
    }
    printf("}\n");
    printf("Vars: {\n");
    for (int i = 0; i <= vm->scopep; i++) {
        printf("Level %d:\n", i);
        VariableNode* vnode = vm->scope[i].variableStack->head;
        while (vnode != NULL) {
            printf("  {%d: ", vnode->current->constantId);
            debug_value(vnode->current->value);
            printf("}\n");
            vnode = vnode->next;
        }
    }
    printf("}\n");
    printf("Functions: {\n");
    ObjFnNode* node = vm->fnTable->head;
    while (node != NULL) {
        ObjFn* fn = node->current;
        printf("  {%d: address %d, params: [", fn->id, fn->address);
        FieldNode* fieldNode = fn->params->head;
        while (fieldNode != NULL) {
            printf("%d ", fieldNode->current->name);
            print_type(fieldNode->current->type);
            fieldNode = fieldNode->next;
            if (fieldNode != NULL) printf(",");
        }
        printf("]}\n");
        node = node->next;
    }
    printf("}\n");
    printf("Structs: {\n");
    StructDefinitionNode* objNode = vm->structs->head;
    while (objNode != NULL) {
        StructDefinition* obj = objNode->current;
        printf("  %d: {", obj->id);
        FieldNode* fieldNode = obj->fields->head;
        while (fieldNode != NULL) {
            printf("%d ", fieldNode->current->name);
            print_type(fieldNode->current->type);
            fieldNode = fieldNode->next;
            if (fieldNode != NULL) printf(",");
        }
        printf("}\n");
        objNode = objNode->next;
    }
    printf("}\n");
    printf("Stack: {\n");
    for (Value* s = vm->stack;s < vm->sp; s++) {
        printf("  {");
        debug_value(*s);
        printf("}\n");
    }
    printf("}\n");
    debug_gc(&vm->gc);
}

void vm_error(VM* vm, const char* error) {
    printf("ERROR: VM[%2x@%x]\n reason: %s\n", vm->program[vm->pc], vm->pc, error);
    exit(VM_ERROR);
}

uint8_t read_byte(VM* vm) {
    uint8_t val = vm->program[vm->pc++];
    return val;
}

uint16_t read_short(VM* vm) {
    uint16_t val;
    memcpy(&val, &vm->program[vm->pc], sizeof(val));
    vm->pc += sizeof(val);
    return val;
}

uint32_t read_address(VM* vm) {
    uint32_t val;
    memcpy(&val, &vm->program[vm->pc], sizeof(val));
    vm->pc += sizeof(val);
    return val;
}

uint64_t read_int(VM* vm) {
    uint64_t val;
    memcpy(&val, &vm->program[vm->pc], sizeof(val));
    vm->pc += sizeof(val);
    return val;
}

void pop_call(VM* vm) {
    vm->pc = vm->callStack[--vm->callsp];
    free(vm->scope[vm->scopep--].variableStack);
}

void push_call(VM* vm) {
    vm->callStack[vm->callsp++] = vm->pc;
    vm->scope[++vm->scopep] = (Scope) {.level=0, .variableStack=make_Variable_list()};
}

char* read_string(VM* vm) {
    uint16_t len = read_short(vm);
    char* b = (char*) malloc(len + 1);
    memcpy(b, &vm->program[vm->pc], len);
    b[len] = '\0';
    vm->pc += len;
    return b;
}

Type read_type(VM* vm) {
    Type t;
    switch (read_byte(vm)) {
        case TP_NUM:
            t.type = V_NUM;
            break;
        case TP_INT:
            t.type = V_INT;
            break;
        case TP_STR:
            t.type = V_STR;
            break;
        case TP_BOOL:
            t.type = V_BOOL;
            break;
        case TP_OBJ: {
            t.type = V_STR;
            Constant cnst = get_constant(vm, read_short(vm));
            t.name = cnst.value.as.string;
            break;
        }
        default:
            vm_error(vm, "Unknown type\n");
    }
    if (read_byte(vm) == 1) {
        Constant cnst = get_constant(vm, read_short(vm));
        t.qualifier = cnst.value.as.string;
    } else {
        t.qualifier = NULL;
    }
    
    t.isArray = read_byte(vm) == 1;
    return t;
}

void read_header(VM* vm) {
    if (read_byte(vm) != OP_HEADER_START) {
        vm_error(vm, "Expected OP_HEADER_START");
    }
    uint16_t val = read_short(vm);
    while (val != OP_HEADER_END) {
        Constant c;
        c.constantId = val;
        c.tp = read_byte(vm);
        switch (c.tp) {
            case TP_INT:
                c.value = INT_VALUE(read_int(vm));
                break;
            case TP_STR:
                c.value = STR_VALUE(read_string(vm));
                break;
            default:
                vm_error(vm, "NYI type!\n");
                break;
        }
        write_Constant(vm->constantArray, c);
        val = read_short(vm);
    }
    
    if (read_byte(vm) != OP_HEADER_START) {
        vm_error(vm, "Expected OP_HEADER_START");
    }
    uint16_t declVal = read_short(vm);
    while (declVal != OP_HEADER_END) {
        vm->pc -= 2; // reset to prior
        uint8_t tp = read_byte(vm);
        if (tp == DECL_FN) {
            uint8_t argsize = read_byte(vm);
            uint16_t constantId = read_short(vm);
            uint32_t address = read_address(vm);
            ObjFn* fn = (ObjFn*) malloc(sizeof(*fn));
            fn->obj.type = O_FN;
            fn->id = constantId;
            fn->argCount = argsize;
            fn->address = address;
            fn->params = make_Field_list();
            for (int i = 0; i < argsize; i++) {
                Field* p = (Field*) malloc(sizeof(*p));
                p->type = read_type(vm);
                p->name = read_short(vm);
                add_Field(fn->params, p);
            }
            add_ObjFn(vm->fnTable, fn);
        } else if (tp == DECL_OBJ) {
            StructDefinition* sd = (StructDefinition*) malloc(sizeof(*sd));
            sd->id = read_short(vm);
            sd->fields = make_Field_list();
            int len = read_byte(vm);
            for (int i = 0; i < len; i++) {
                Field* field = (Field*) malloc(sizeof(*field));
                field->type = read_type(vm);
                field->name = read_short(vm);
                add_Field(sd->fields, field);
            }
            add_StructDefinition(vm->structs, sd);
        } else {
            printf("Declaration %d\n", tp);
            vm_error(vm, "Declaration type not yet implemented\n");
        }
        declVal = read_short(vm);
    }
    debug_vm(vm);
}

void read_body(VM* vm) {
    debug_vm(vm);
}

void set_variable(VM* vm, int id, Value val) {
    VariableNode* node = vm->scope[vm->scopep].variableStack->head;
    while (node != NULL) {
        if (node->current->constantId == id) {
            node->current->value = val;
            return;
        }
        node = node->next;
    }

    debug_vm(vm);
    printf("Could not find variable %d\n", id);
    vm_error(vm, "Could not find variable\n");
    exit(1);
}

Value get_variable(VM* vm, int id) {
    VariableNode* node = vm->scope[vm->scopep].variableStack->head;
    while (node != NULL && node->current != NULL) {
        if (node->current->constantId == id) {
            return node->current->value;
        }
        node = node->next;
    }
    ObjFnNode* fnNode = vm->fnTable->head;
    while (fnNode != NULL) {
        if (fnNode->current->id == id) {
            return OBJ_VALUE(fnNode->current);
        }
        fnNode = fnNode->next;
    }
    VariableNode* gnode = vm->globalVariables->head;
    while (gnode != NULL && gnode->current != NULL) {
        if (gnode->current->constantId == id) {
            return gnode->current->value;
        }
        gnode = gnode->next;
    }

    debug_vm(vm);
    printf("Could not find variable %d\n", id);
    vm_error(vm, "Could not find variable\n");
    exit(1);
}

StructDefinition* get_struct_definition(VM* vm, uint16_t id) {
    StructDefinitionNode* node = vm->structs->head;
    while (node != NULL) {
        if (node->current->id == id) {
            return node->current;
        }
        node = node->next;
    }

    debug_vm(vm);
    printf("Could not find struct def %d\n", id);
    vm_error(vm, "Could not find struct def\n");
    exit(1);
}

Constant get_constant(VM* vm, int id) {
    for (int i = 0; i < vm->constantArray->count; i++) {
        if (vm->constantArray->data[i].constantId == id) {
            return vm->constantArray->data[i];
        }
    }
    printf("Could not find constant %d\n", id);
    vm_error(vm, "Could not find constant\n");
    exit(1);
}

void push_stack(VM* vm, Value val) {
    *vm->sp = val;
    vm->sp++;
}

Value pop_stack(VM* vm) {
    return *(--vm->sp);
}

Value concatenate_string(Value a, Value b) {
    int asize = strlen(a.as.string);
    int bsize = strlen(b.as.string);
    int size = asize + bsize + 1;
    char* value = (char*) malloc(size);
    int i = 0;
    for (; i < asize; i++) {
        value[i] = a.as.string[i];
    }
    for (int j = 0; j < bsize; j++) {
        value[i + j] = b.as.string[j];
    }
    value[size - 1] = '\0'; 
    return STR_VALUE(value);
}

Value concatenate(VM* vm, ObjArray* a, ObjArray* b) {
    ObjArray* r = (ObjArray*) malloc(sizeof *r);
    r->componentType = a->componentType;
    r->obj.type = O_ARRAY;
    r->size = a->size + b->size;
    r->vals = (Value*) calloc(r->size, sizeof(Value));
    register_object(&vm->gc, (Obj*) r, sizeof *r);
    int i = 0;
    for (; i < a->size; i++) {
        r->vals[i] = a->vals[i];
    }
    for (int j = 0; j < b->size; j++) {
        r->vals[i + j] = b->vals[j];
    }
    return OBJ_VALUE(r);
}

Value appendl(VM* vm, ObjArray* a, Value b) {
    ObjArray* r = (ObjArray*) malloc(sizeof *r);
    r->componentType = a->componentType;
    r->obj.type = O_ARRAY;
    r->size = a->size + 1;
    r->vals = (Value*) calloc(r->size, sizeof(Value));
    register_object(&vm->gc, (Obj*) r, sizeof *r);
    r->vals[0] = b;
    for (int i = 1; i < a->size + 1; i++) {
        r->vals[i] = a->vals[i - 1];
    }
    return OBJ_VALUE(r);
}

Value appendr(VM* vm, Value a, ObjArray* b) {
    ObjArray* r = (ObjArray*) malloc(sizeof *r);
    r->componentType = b->componentType;
    r->obj.type = O_ARRAY;
    r->size = b->size + 1;
    r->vals = (Value*) calloc(r->size, sizeof(Value));
    register_object(&vm->gc, (Obj*) r, sizeof *r);
    for (int i = 0; i < b->size; i++) {
        r->vals[i] = b->vals[i];
    }
    r->vals[b->size] = a;
    return OBJ_VALUE(r);
}

void handle_instruction(VM* vm, char b) {
    //printf("Handling instruction %02x@%04x\n", b, vm->pc - 1);
    switch (b) {
        case OP_SCOPE_START: {
            vm->scope->level++;
            break;
        }
        case OP_SCOPE_END: {
            VariableNode* node = vm->scope->variableStack->tail;

            // Stuff is out of scope now. 
            while (node != NULL) {
                if (node->current->level >= vm->scope->level) {
                    if (node->prev != NULL) node->prev->next = NULL;
                    vm->scope->variableStack->tail = node->prev;
                    free(node);
                } else {
                    break;
                }
                node = node->prev;
            }

            vm->scope->level--;
            break;
        }
        case OP_VAR_DECL: {
            uint16_t constantId = read_short(vm);
            Value val = pop_stack(vm);
            Variable* var = (Variable*) malloc(sizeof(*var));
            var->constantId = constantId;
            var->level = vm->scope[vm->scopep].level;
            var->value = val;
            add_Variable(vm->scope[vm->scopep].variableStack, var);
            break;
        }
        case OP_VAR_ASS: {
            Value val = pop_stack(vm);
            uint8_t levels = read_byte(vm);
            uint16_t constantId = read_short(vm);
            if (levels == 1) {
                set_variable(vm, constantId, val);
            }
            else {
                ObjStruct* str = NULL;
                Value var = get_variable(vm, constantId);
                for (int i = 1; i < levels - 1; i++) {
                    str = (ObjStruct*) var.as.object;
                    uint16_t field = read_short(vm);
                    var = get_field(str, field);
                }
                str = (ObjStruct*) var.as.object;
                uint16_t field = read_short(vm);
                set_field(str, field, val);
            }
            break;
        }
        case OP_VAR_REF: {
            uint8_t levels = read_byte(vm);
            // first level
            uint16_t constantId = read_short(vm);
            Value var = get_variable(vm, constantId);
            for (int i = 1; i < levels; i++) {
                // assumption - is struct
                ObjStruct* str = (ObjStruct*) var.as.object;
                uint16_t field = read_short(vm);
                var = get_field(str, field);
            }
            push_stack(vm, var);
            break;
        }
        case OP_GETARR: {
            Value arr = pop_stack(vm);
            Value index = pop_stack(vm);
            if (arr.type != V_OBJ) {
                vm_error(vm, "Expected obj type\n");
            }
            if (arr.as.object->type != O_ARRAY) {
                vm_error(vm, "Expected array type\n");
            }
            ObjArray* array = (ObjArray*) arr.as.object;
            push_stack(vm, array->vals[index.as.integer]);
            break;
        }
        case OP_CONCAT: {
            Value l = pop_stack(vm);
            Value r = pop_stack(vm);
            if (l.type != r.type)
                vm_error(vm, "Binary operation on non-matching types!");
            if (l.type != V_STR && (l.type != V_OBJ || l.as.object->type != O_ARRAY))
                vm_error(vm, "Type is not array");
            if (l.type == V_STR)
                push_stack(vm, concatenate_string(l, r));
            else
                push_stack(vm, concatenate(vm, AS_ARRAY(l), AS_ARRAY(r)));
            break;
        }
        case OP_APPEND: {
            Value r = pop_stack(vm);
            Value l = pop_stack(vm);
            bool lIsArray = (l.type != V_OBJ || l.as.object->type != O_ARRAY);
            bool rIsArray = (r.type != V_OBJ || r.as.object->type != O_ARRAY);
            if (l.type == r.type)
                vm_error(vm, "Append operation on invalid type!");
            if (!lIsArray && !rIsArray)
                vm_error(vm, "Type is not array");
            //if (l.type == V_STR)
                //push_stack(vm, concatenate_string(l, r));
            //else
            if (lIsArray) 
              push_stack(vm, appendr(vm, l, AS_ARRAY(r)));
            if (rIsArray)
              push_stack(vm, appendl(vm, AS_ARRAY(l), r));
            break;
        }
        case OP_NEG: {
            UNARY(-)
            break;
        }
        case OP_ADD: {
            BINARY(+)
            break;
        }
        case OP_MUL: {
            BINARY(*)
            break;
        }
        case OP_SUB: {
            BINARY(-)
            break;
        }
        case OP_DIV: {
            BINARY(/)
            break;
        }
        case OP_LT: {
            CMP(<)
            break;
        }
        case OP_LE: {
            CMP(<=)
            break;
        }
        case OP_GT: {
            CMP(>)
            break;
        }
        case OP_GE: {
            CMP(>=)
            break;
        }
        case OP_EQ: {
            CMP(==)
            break;
        }
        case OP_CONSTANT_BOOL_TRUE:
            push_stack(vm, BOOL_VALUE(true));
            break;
        case OP_CONSTANT_BOOL_FALSE:
            push_stack(vm, BOOL_VALUE(false));
            break;
        case OP_FN_CALL: {
            Value v = pop_stack(vm);
            if (v.type != V_OBJ) {
                debug_value(v);
                printf("\n");
                vm_error(vm, "Stack value not object!");
            }
            if (v.as.object->type == O_FN) {
                ObjFn* fn = (ObjFn*) v.as.object;
                uint8_t argCount = read_byte(vm);
                push_call(vm);
                call(vm, fn, argCount);
            }
            else if (v.as.object->type == O_NATFN) {
                ObjNatFn* fn = (ObjNatFn*) v.as.object;
                printf("Calling %s\n", fn->name);
                uint8_t argc = read_byte(vm);
                Value args[argc];
                for (int i = argc - 1; i >= 0; i--) {
                    args[i] = pop_stack(vm);
                }
                fn->fn(argc, args);
            }
            else {
                debug_value(v);
                printf("\n");
                vm_error(vm, "Stack value not function!");
            }
            break;
        }
        case OP_NEWA: {
            ObjArray* s = (ObjArray*) malloc(sizeof(*s));
            s->obj.type = O_ARRAY;
            s->size = read_short(vm);
            s->componentType = read_type(vm);
            s->vals = (Value*) calloc(s->size, sizeof(Value));
            register_object(&vm->gc, (Obj*) s, sizeof(*s));

            for (int i = s->size-1; i >= 0; i--) {
                Value val = pop_stack(vm);
                s->vals[i] = val;
            }
            Value val = {.type=V_OBJ, .as.object = (Obj*) s};
            push_stack(vm, val);
            break;
        }
        case OP_NEW: {
            ObjStruct* s = (ObjStruct*) malloc(sizeof(*s));
            s->obj.type = O_STRUCT;
            s->name = read_short(vm);
            register_object(&vm->gc, (Obj*) s, sizeof(*s));

            StructDefinition* def = get_struct_definition(vm, s->name);
            s->fvs = (FieldValue*) calloc(sizeof(FieldValue), def->fields->size);
            s->size = def->fields->size;
            FieldNode* fn = def->fields->tail;
            int i = 0;
            while (fn != NULL) {
                Value val = pop_stack(vm);
                s->fvs[i] = (FieldValue) {.name=fn->current->name, .val=val};
                fn = fn->prev;
                i++;
            }
            Value val = {.type=V_OBJ, .as.object = (Obj*) s};
            push_stack(vm, val);
            break;
        }
        case OP_CONSTANT_NUM:
        case OP_CONSTANT_STR:
        case OP_CONSTANT_INT: 
        {
            uint16_t constantId = read_short(vm);
            push_stack(vm, get_constant(vm, constantId).value);
            break;
        }
        case OP_JUMP: {
            uint32_t address = read_address(vm);
            vm->pc = address;
            break;
        }
        case OP_JUMP_FALSE: {
            Value cond = pop_stack(vm);
            uint32_t address = read_address(vm);
            if (cond.type == V_BOOL && !cond.as.boolean) {
                vm->pc = address;
            }
            break;
        }
        case OP_RETURN:
            pop_call(vm);
            break;
        default:
            printf("Unknown %d\n", b);
            vm_error(vm, "Instruction NYI");
    }
}

void call(VM* vm, ObjFn* fn, uint8_t argCount) {
    vm->pc = fn->address;
    FieldNode* param = fn->params->head;
    while (param != NULL) {
        Value v = pop_stack(vm);
        Variable* var = (Variable*) malloc(sizeof(*var));
        var->constantId = param->current->name;
        var->level = vm->scope[vm->scopep].level;
        var->value = v;
        add_Variable(vm->scope[vm->scopep].variableStack, var);
        param = param->next;
    }
}

VmStatus run_main(VM* vm) {
    ObjFnNode* node = vm->fnTable->head;
    ObjFn* mainFn = NULL;
    while (node != NULL) {
        Constant c = get_constant(vm, node->current->id);
        if (c.tp == TP_STR && strcmp(c.value.as.string, "main") == 0) {
            mainFn = node->current;
            break;
        }
        node = node->next;
    }
    if (mainFn == NULL) {
        vm_error(vm, "No main function found.\n");
    }
    push_call(vm);
    call(vm, mainFn, 0);
    while (true) {
        uint8_t b = read_byte(vm);
        handle_instruction(vm, b);
        
        run_gc(&vm->gc, vm->stack, vm->sp - vm->stack, vm->scope[vm->scopep].variableStack);
        //debug_vm(vm);
        if (vm->callsp == 0) {
            break;
        }
    }
    debug_vm(vm);
    return VM_SUCCESS;
}

uint16_t push_constant_string(VM* vm, char* str) {
    for (int i = 0; i < vm->constantArray->count; i++) {
        Constant c = vm->constantArray->data[i];
        if (c.tp == TP_STR && strcmp(c.value.as.string, str) == 0) {
            return c.constantId;
        }
    }

    Constant c;
    c.constantId = vm->constantArray->count;
    c.tp = TP_STR;
    c.value = STR_VALUE(str);
    write_Constant(vm->constantArray, c);
    return c.constantId;
}

VmStatus run(VM* vm) {
    read_header(vm);
    read_body(vm);
    initialize_natives(vm);
    return run_main(vm);
}
