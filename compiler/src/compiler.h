#include "lexer.h"
#include "parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    TypeCode type;
    union {
        bool boolean;
        uint64_t integer;
        double number;
        char* string;
    } as;
} Value;

typedef struct {
    uint16_t constantId;
    Value value;
} Constant;

typedef struct {
    uint16_t constantId;
    uint32_t pc;
} FnInfo;

DEFINE_DYNAMIC_ARRAY(Constant)

typedef struct {
    char* path;
    FILE* file;

    int currentPos;
    char buffer[0x100000];

    FnInfo fns[0x100];
    ConstantArray* constantPool;
} Compiler;

void init_compiler(Compiler* compiler);
void compile(Compiler* compiler, Parser* parser);