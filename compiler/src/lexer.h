#ifndef lexer_h
#define lexer_h

#include "stdint.h"
#define RPAREN ')'
#define LPAREN '('
#define RCPAREN '}'
#define LCPAREN '{'
#define LSPAREN '['
#define RSPAREN ']'
#define SEMIC ';'
#define COLON ':'
#define COMMA ','
#define PERIOD '.'
#define PLUS '+'
#define MINUS '-'
#define MUL '*'
#define DIV '/'
#define EQ '='
#define NOT '!'
#define LT '<'
#define GT '>'
#define NL '\n'
#define QT '"'

#define KW_FN "fn"
#define KW_ACTOR "act"
#define KW_OBJ "obj"
#define KW_RET "return"
#define KW_IF "if"
#define KW_ELIF "elif"
#define KW_ELSE "else"
#define KW_LOOP "loop"
#define KW_VAR "var"
#define KW_NEW "new"
#define KW_MAYBE "maybe"
#define KW_JUST "just"
#define KW_NONE "none"
#define KW_TRUE "true"
#define KW_FALSE "false"
#define KW_TYPEDEF "define"

#define KW_BYTE "byte"
#define KW_STRING "string"
#define KW_BOOL "bool"
#define KW_INTEGER "int"
#define KW_NUMBER "num"
#define KW_PID "pid"

#define KW_IS "is"

extern const char* tokens[];

typedef struct {
    const char* program;
    const char* current;
    uint32_t line;
    uint32_t column;
} Lexer;

typedef enum {
    T_EOF, T_ERROR,
    T_RPAREN, T_LPAREN,
    T_RCPAREN, T_LCPAREN,
    T_RSPAREN, T_LSPAREN,
    T_SEMIC, T_COLON,
    T_COMMA, T_PERIOD,
    T_PLUS, T_MINUS,
    T_EQQ, T_EQ,
    T_NOT, T_NEQ,
    T_LT, T_LEQ,
    T_GT, T_GEQ,
    T_MUL, T_DIV,
    T_STRING, T_INTEGER,
    T_FLOAT, T_IDENTIFIER,
    T_FN, T_IF, T_ELSE, T_ELIF,
    T_LOOP, T_RETURN, T_VAR,
    T_OBJ, T_TYPEDEF, T_ACTOR,
    T_MAYBE, T_JUST, T_NONE,
    T_TRUE, T_FALSE, T_IS,
    T_NEW, T_CONCAT, T_PID,
    T_TSTR, T_TINT, 
    T_TNUM, T_TBOOL,
} Token;

typedef struct {
    Token token;
    const char* start;
    uint32_t length;
    uint32_t line;
    uint32_t col;
} Lexeme;

Lexeme read_next_token(Lexer* l);
void print_lexeme(Lexeme l);

#endif