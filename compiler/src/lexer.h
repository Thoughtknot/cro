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
#define KW_OBJ "obj"
#define KW_RET "return"
#define KW_IF "if"
#define KW_ELSE "else"
#define KW_LOOP "loop"
#define KW_VAR "var"
#define KW_STRING "string"
#define KW_INTEGER "int"
#define KW_NUMBER "num"


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
    T_OBJ
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