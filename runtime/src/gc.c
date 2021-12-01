#include "gc.h"
#include <stdio.h>

void register_object(GC* gc, Obj* obj, int allocated) {
    obj->next = gc->head;
    obj->isMarked = false;
    gc->head = obj;
    gc->newAllocated += allocated;
}

void init_gc(GC* gc) {
    gc->head = NULL;
    gc->allocated = 0;
    gc->newAllocated = 0;
}

void debug_gc(GC* gc) {
    Obj* obj = gc->head;
    printf("Registered heap: {");
    while (obj != NULL) {
        printf(" ");
        debug_object(obj);
        printf(" ");
        obj = obj->next;
    }
    printf("}\n");
}


void mark(Obj* obj) {
    obj->isMarked = true;
    if (obj->type == O_STRUCT) {
        ObjStruct* s = (ObjStruct*) obj;
        for (int i = 0; i < s->size; i++) {
            Value fv = s->fvs[i].val;
            if (fv.type == V_OBJ) 
                mark(fv.as.object);
        }
    }
}

void free_object(Obj* obj) {
    if (obj->type == O_STRUCT) {
        ObjStruct* s = (ObjStruct*) obj;
        free(s->fvs);
        s->fvs = NULL;
    }
    free(obj);
}

void run_gc(GC* gc, Value* stack, int length, VariableList* vl) {
    if (!(gc->newAllocated > 64 + gc->allocated)) {
        return;
    }
    gc->allocated += gc->newAllocated;
    gc->newAllocated = 0;

    // mark
    for (int i = 0; i < length; i++) {
        Value toMark = stack[i];
        if (toMark.type == V_OBJ) {
            mark(toMark.as.object);
        }
    }
    VariableNode* node = vl->head;
    while (node != NULL) {
        if (node->current->value.type == V_OBJ)
            mark(node->current->value.as.object);
        node = node->next;
    }

    // sweep
    Obj* prev = NULL;
    Obj* currentObj = gc->head;
    while (currentObj != NULL) {
        if (!currentObj->isMarked) {
            if (prev == NULL) {
                gc->head = currentObj->next;
            } else {
                prev->next = currentObj->next;
            }
            Obj* toGc = currentObj;
            currentObj = currentObj->next;
            printf("GC'd object:");
            debug_object(toGc);
            printf("\n");
            free_object(toGc);
        } else {
            currentObj->isMarked = false;
            prev = currentObj;
            currentObj = currentObj->next;
        }
    } 
}