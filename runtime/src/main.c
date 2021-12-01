#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"

char* read_to_string(const char* path);

int main(int argc, char const *argv[]) {
    VM vm;
    if (argc == 2) {
        char* str = read_to_string(argv[1]);
        init_vm(&vm, str);
        return run(&vm);
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
    printf("RUNNING PROGRAM: %s\n", path);
    fclose(file);
    return buffer;
}