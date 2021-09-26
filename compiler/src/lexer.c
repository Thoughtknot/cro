#include "lexer.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define SIMPLE_LEXEME(T) \
    case T: return make_lexeme(l, T_##T);

bool is_keyword(Lexer* l, const char* str);
bool is_digit(char c);
bool is_char(char c);
bool next_char(Lexer* l, char expected);
void skip_ws(Lexer* l);
void advance(Lexer* l);
char peek(Lexer* l, int lookahead);

Lexeme make_lexeme(Lexer* l, Token t) {
    return (Lexeme) { .token=t, .start=l->program, .line=l->line, .col=l->column, .length=(l->current - l->program) };
}

Lexeme make_number(Lexer* l) {
    while (is_digit(peek(l, 0))) advance(l);
    if (peek(l, 0) == PERIOD && is_digit(peek(l, 1))) {
        advance(l);
        while (is_digit(peek(l, 0))) advance(l);
        return make_lexeme(l, T_FLOAT);
    } else {
        return make_lexeme(l, T_INTEGER);
    }
}

Lexeme make_identifier(Lexer* l) {
    while (is_char(peek(l, 0))) advance(l);
    if (is_keyword(l, KW_FN)) {
        return make_lexeme(l, T_FN);
    }
    if (is_keyword(l, KW_VAR)) {
        return make_lexeme(l, T_VAR);
    }
    if (is_keyword(l, KW_OBJ)) {
        return make_lexeme(l, T_OBJ);
    }
    return make_lexeme(l, T_IDENTIFIER);
}

Lexeme make_string(Lexer* l) {
    while (true) {
        char c = peek(l, 0);
        if (c == QT || c == '\0' ) break;
        advance(l);
    }
    if (peek(l, 0) == '\0') return make_lexeme(l, T_ERROR);
    advance(l);
    return make_lexeme(l, T_STRING);
}

Lexeme read_next_token(Lexer* l) {
    skip_ws(l);
    l->program = l->current;
    char c = peek(l, 0);
    if (c == '\0') {
        return make_lexeme(l, T_EOF);
    }
    advance(l);
    if (is_digit(c)) return make_number(l);
    if (is_char(c)) return make_identifier(l);
    switch (c) {
        SIMPLE_LEXEME(RPAREN)
        SIMPLE_LEXEME(LPAREN)
        SIMPLE_LEXEME(RCPAREN)
        SIMPLE_LEXEME(LCPAREN)
        SIMPLE_LEXEME(RSPAREN)
        SIMPLE_LEXEME(LSPAREN)
        SIMPLE_LEXEME(SEMIC)
        SIMPLE_LEXEME(COLON)
        SIMPLE_LEXEME(PERIOD)
        SIMPLE_LEXEME(COMMA)
        SIMPLE_LEXEME(PLUS)
        SIMPLE_LEXEME(MINUS)
        SIMPLE_LEXEME(MUL)
        SIMPLE_LEXEME(DIV)
        case QT:
            return make_string(l);
        case EQ: 
            if (next_char(l, EQ)) {
                return make_lexeme(l, T_EQQ);
            } else {
                return make_lexeme(l, T_EQ);
            }
        case NOT: 
            if (next_char(l, EQ)) {
                return make_lexeme(l, T_NEQ);
            } else {
                return make_lexeme(l, T_NOT);
            }
        case GT: 
            if (next_char(l, EQ)) {
                return make_lexeme(l, T_GEQ);
            } else {
                return make_lexeme(l, T_GT);
            }
        case LT: 
            if (next_char(l, EQ)) {
                return make_lexeme(l, T_LEQ);
            } else {
                return make_lexeme(l, T_LT);
            }
    }

    printf("Error in lexing of char %d\n", c);
    exit(1);
}

void advance(Lexer* l) {
    l->current++;
    l->column++;
}

char peek(Lexer* l, int lookahead) {
    if (*l->current == '\0') return '\0';
    return l->current[lookahead];
}

void skip_ws(Lexer* l) {
    while (true) {
        char c = peek(l, 0);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(l);
                break;
            case DIV:
                if (peek(l, 1) != DIV) {
                    while (peek(l, 0) != NL && *l->current != '\0') {
                        advance(l);
                    }
                    break;
                }
            case NL:
                l->line++;
                l->column = 1;
                advance(l);
                break;
            default:
               return;
        }
    }
}

bool is_char(char c) {
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || c == '_';
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_keyword(Lexer* l, const char* str) {
    int len = l->current - l->program;
    int i = 0;
    for (; str[i] != '\0';i++) {
        if (l->program[i] != str[i]) {
            return false;
        }
    }
    if (len == i) {
        return true;
    }
    return false;
}

bool next_char(Lexer* l, char expected) {
    if (*l->current != expected) return false;
    l->current++;
    return true;
}

void print_lexeme(Lexeme l) {
    printf("Lexeme[%d][line: %d, col: %d][", l.token, l.line, l.col);
    for (int i = 0; i < l.length; i++) {
        printf("%c", l.start[i]);
    }
    printf("]\n");
}