#ifndef types_h
#define types_h

typedef enum {
    TP_INT,
    TP_NUM,
    TP_BOOL,
    TP_STR,
    TP_OBJ,
    TP_BYTE,
} TypeCode;

typedef enum {
    DECL_OBJ,
    DECL_FN
} DeclarationType;

typedef enum {
    OP_HEADER_START = 0x01,
    OP_HEADER_END = 0xFFFF,

    OP_CONSTANT_INT = 0x10,
    OP_CONSTANT_NUM = 0x11,
    OP_CONSTANT_STR = 0x12,
    OP_CONSTANT_OBJ = 0x13,
    OP_CONSTANT_BOOL_TRUE = 0x14,
    OP_CONSTANT_BOOL_FALSE = 0x15,
    OP_NEW = 0x16,
    OP_NEWA = 0x17,
    OP_CONSTANT_BYTE = 0x18,

    OP_MUL = 0x20,
    OP_DIV = 0x21,
    OP_ADD = 0x22,
    OP_SUB = 0x23,

    OP_NEG = 0x24,
    OP_CONCAT = 0x25,
    OP_APPEND = 0x26,

    OP_JUMP = 0x30,
    OP_JUMP_FALSE = 0x31,

    OP_LT = 0x40,
    OP_LE = 0x41,
    OP_GT = 0x42,
    OP_GE = 0x43,
    OP_EQ = 0x44,

    OP_RETURN = 0x50,
    OP_SCOPE_START = 0x51,
    OP_SCOPE_END = 0x52,

    OP_VAR_DECL = 0x61,
    OP_VAR_ASS = 0x62,
    OP_VAR_REF = 0x63,
    OP_GETARR = 0x64,

    OP_FN_DECL = 0x70,
    OP_FN_CALL = 0x71,
    OP_FN_REF_CALL = 0x72,

} Opcode;

#endif