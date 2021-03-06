#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "compiler.h"

char* read_to_string(const char* path);

int main(int argc, char const *argv[]) {
    if (argc == 2) {
        char* str = read_to_string(argv[1]);
        char* path = malloc(strlen(str));
        strcpy(path, strtok(argv[1], ".crc"));
        memcpy(path + strlen(path), ".crb", 5);
        Lexer lexer = (Lexer) { .program=str, .current=str, .column=1, .line=1};
        Parser parser = {.lexer = &lexer};
        Compiler compiler = {.path=path};
        init_parser(&parser);
        init_compiler(&compiler);
        compile(&compiler, &parser);
    } else {
        printf("Invalid number of args, %d\n", argc);
        return 1;
    }
    return 0;
}

char* read_to_string(const char* path) {
    FILE* file = fopen(path, "rb");

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}