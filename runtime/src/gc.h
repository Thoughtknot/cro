#include "../../common/src/dynamic_array.h"
#include "object.h"

typedef struct {
    Obj* head;
    int allocated;
    int newAllocated;
} GC;

void init_gc(GC* gc);
void debug_gc(GC* gc);
void register_object(GC* gc, Obj* obj, int allocated);
void run_gc(GC* gc, Value* stack, int length, VariableList* vl);