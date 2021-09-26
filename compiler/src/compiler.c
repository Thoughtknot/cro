#include "compiler.h"
#include "parser.h"
#include <stdbool.h>

void compile(const char* program) {
    Lexer lexer = (Lexer) { .program=program, .current=program, .column=1, .line=1};
    Parser parser = {.lexer = &lexer};
    init_parser(&parser);
    parse(&parser);
}