#include "../../common/src/types.h"
#include <string.h>
#include "decompiler.h"
#include "compiler.h"

int main(int argc, char const *argv[]) {
    return 0;
}

uint8_t read_byte(Block* vm) {
    uint8_t val = vm->program[vm->pc++];
    return val;
}

uint16_t read_short(Block* vm) {
    uint16_t val;
    memcpy(&val, &vm->program[vm->pc], sizeof(val));
    vm->pc += sizeof(val);
    return val;
}

uint32_t read_address(Block* vm) {
    uint32_t val;
    memcpy(&val, &vm->program[vm->pc], sizeof(val));
    vm->pc += sizeof(val);
    return val;
}

uint64_t read_int(Block* vm) {
    uint64_t val;
    memcpy(&val, &vm->program[vm->pc], sizeof(val));
    vm->pc += sizeof(val);
    return val;
}

char* read_string(Block* vm) {
    uint16_t len = read_short(vm);
    char* b = (char*) malloc(len + 1);
    memcpy(b, &vm->program[vm->pc], len);
    b[len] = '\0';
    vm->pc += len;
    return b;
}

void read_header(Block* b) {
    if (read_byte(b) != OP_HEADER_START) {
        printf("Expected start header.");
    }
    uint16_t val = read_short(b);
    printf("Constants: {\n");
    Value constants[0x100];
    while (val != OP_HEADER_END) {
        uint8_t bt = read_byte(b);
        Value v;
        v.type = bt;
        switch (bt) {
            case TP_INT:
                v.as.integer = read_int(b);
                printf("{%d: int %ld}", val, v.as.integer);
                break;
            case TP_STR:
                v.as.string = read_string(b);
                printf("{%d: str %s}", val, v.as.string);
                break;
            default:
                printf("NYI type!\n");
                break;
        }
        constants[val] = v;
        val = read_short(b);
    }
    
    if (read_byte(b) != OP_HEADER_START) {
        printf("Expected start header.");
    }
    printf("Declarations: {\n");
    uint16_t declVal = read_short(b);
    while (declVal != OP_HEADER_END) {
        b->pc -= 2; // reset to prior
        uint8_t tp = read_byte(b);
        if (tp == DECL_FN) {
            uint8_t argsize = read_byte(b);
            uint16_t constantId = read_short(b);
            uint32_t address = read_address(b);
            printf("Fn {name: %s, id: %d, args: %d, address: %d}\n", 
                constants[constantId].as.string, constantId, argsize, address);
        } else {
            printf("Declaration type not yet implemented\n");
        }
        declVal = read_short(b);
    }
    /*
    while (true) {
        uint8_t bt = read_byte(b);
        if 
    }
    */
}