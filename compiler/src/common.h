#ifndef common_h
#define common_h
#include "../../common/src/dynamic_array.h"
#include "../../common/src/linked_list.h"
#include "../../common/src/types.h"
#include <stdbool.h>

typedef enum {
    STMT_VARDECL,
    STMT_FNDECL,
    STMT_VARASS,
    STMT_FNCALL,
    STMT_OBJDECL
} StatementType;

typedef enum {
    EXP_NUMLIT,
    EXP_INTLIT,
    EXP_STRLIT,
    EXP_VARREF,
    EXP_BINARY,
    EXP_UNARY,
    EXP_PAREN,
    EXP_FNCALL,
    EXP_FIELDREF
} ExpressionType;

typedef struct {
    int length;
    char* value;
} Identifier;

typedef struct {
    Identifier type;
    bool array;
} Type;

typedef struct {
    ExpressionType type;
} Expression;

typedef struct {
    StatementType type;
} Statement;

typedef struct {
    Type type;
    Identifier name;
} Param;

DEFINE_LINKED_LIST(Param)
DEFINE_LINKED_LIST(Statement)
DEFINE_LINKED_LIST(Expression)

#endif