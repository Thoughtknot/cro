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
    STMT_OBJDECL,
    STMT_LOOP,
    STMT_IF,
    STMT_RET,
    STMT_TYPEDEF
} StatementType;

typedef enum {
    EXP_NUMLIT,
    EXP_INTLIT,
    EXP_STRLIT,
    EXP_BOOLLIT,
    EXP_NONELIT,
    EXP_JUSTLIT,
    EXP_ARRAYLIT,
    EXP_ARRAYREF,
    EXP_VARREF,
    EXP_BINARY,
    EXP_UNARY,
    EXP_PAREN,
    EXP_FNCALL,
    EXP_OBJLIT,
    EXP_TUPLE,
} ExpressionType;

typedef struct {
    int length;
    char* value;
    int constantId;
} Identifier;

typedef struct {
    Identifier quantifier;
    Identifier type;
    bool array;
} Type;

typedef struct {
    ExpressionType type;
    int constantId;
} Expression;

typedef struct {
    StatementType type;
    int constantId;
} Statement;

typedef struct {
    Type type;
    Identifier name;
} Param;

DEFINE_LINKED_LIST(Param)
DEFINE_LINKED_LIST(Statement)
DEFINE_LINKED_LIST(Expression)

#endif