#include "object.h"
#include <stdio.h>
#include <stdlib.h>

MAKE_LINKED_LIST(Field)

void debug_object(Obj* obj) {
    if (obj == NULL) {
        printf("NULL");
        return;
    }
    switch (obj->type) {
        case O_STRUCT: {
            ObjStruct* str = (ObjStruct*) obj;
            printf("obj[%d]: {", str->name);
            for (int i = 0; i < str->size; i++) {
                printf(" %d: ", str->fvs[i].name);
                debug_value(str->fvs[i].val);
            }
            printf("}");
            break;
        }
        case O_FN: {
            ObjFn* str = (ObjFn*) obj;
            printf("fn[%d]@[%d]: {", str->id, str->address);
            FieldNode* n = str->params->head;
            while (n != NULL) {
                printf(" %d %s", n->current->name, n->current->type.name);
                n = n->next;
            }
            printf("}");
            break;
        }
        case O_NATFN: {
            ObjNatFn* str = (ObjNatFn*) obj;
            printf("natfn[%s]", str->name);
            break;
        }
        case O_ARRAY: {
            ObjArray* str = (ObjArray*) obj;
            printf("array[%d, %s]: {", str->componentType.type, str->componentType.name);
            for (int i = 0; i < str->size; i++) {
                debug_value(str->vals[i]);
                if (i < str->size - 1) printf(", ");
            }
            printf("}");
            break;
        }
        default:
            printf("Can't print obj %d\n", obj->type);
    }
}

void debug_value(Value v) {
    switch (v.type) {
        case V_INT:
            printf("%ld", v.as.integer);
            break;
        case V_STR:
            printf("\"%s\"", v.as.string);
            break;
        case V_BOOL:
            printf("%s", v.as.boolean ? "true" : "false");
            break;
        case V_NUM:
            printf("%f", v.as.number);
            break;
        case V_OBJ:
            debug_object(v.as.object);
            break;
        default:
            printf("Can't print unknown value %d", v.type);
            break;
    }
}

Value get_field(ObjStruct* str, uint16_t name) {
    for (int i = 0; i < str->size; i++) {
        FieldValue fv = str->fvs[i];
        if (fv.name == name) {
            return fv.val;
        }
    }
    printf("ERROR: could not find field %d", name);
    debug_object((Obj*) str);
    return (Value) {.type=V_OBJ, .as.object=NULL};
}

void set_field(ObjStruct* str, uint16_t name, Value val) {
    for (int i = 0; i < str->size; i++) {
        FieldValue* fv = &str->fvs[i];
        if (fv->name == name) {
            fv->val = val;
            return;
        }
    }
    printf("ERROR: could not find field %d", name);
    debug_object((Obj*) str);
    return;
}
