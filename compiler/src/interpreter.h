#include "parser.h"
#include "expressions.h"

DEFINE_LINKED_LIST(ObjDeclStatement)
DEFINE_LINKED_LIST(FnDeclStatement)

typedef enum {
    STRUCT
} ValueType;

typedef struct {
} Obj;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        int integer;
        double number;
        char* string;
        Obj* object;
    } as;
} Value;

typedef struct {
    char* name;
    Value value;
} Variable;
DEFINE_LINKED_LIST(Variable)

typedef struct {
    VariableList* list;
} Scope;

typedef struct {
    FnDeclStatementList* fns;
    ObjDeclStatementList* objs;

    Scope scope;
    ConstantArray* array;
} Interpreter;

void init_interpreter(Interpreter* i);
void interpret(Interpreter* i, Parser* p);