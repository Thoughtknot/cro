#ifndef parser_h
#define parser_h
#include "lexer.h"
#include "../../common/src/linked_list.h"
#include "../../common/src/dynamic_array.h"
#include "common.h"

DEFINE_DYNAMIC_ARRAY(Lexeme)

typedef enum {
    P_NONE,
    P_ASS,
    P_CMP,
    P_ADD,
    P_MUL,
    P_UNARY,
    P_PAREN,
    P_CALL,
    P_TUPLE
} Precedence;

typedef struct {
    LexemeArray* read;
    int currentLexeme;
    Lexer* lexer;
    ExpressionList* stack;
} Parser;

StatementList* parse(Parser* p);
void init_parser(Parser* p);
void parser_error(Parser* p, const char* reason);

Lexeme get_previous_lexeme(Parser* p);
Lexeme get_current_lexeme(Parser* p);
void read_next_lexeme(Parser* p);

Statement* read_statement(Parser* p);
void read_expression(Parser* p, Precedence pr);
void read_expected(Parser* p, Token token);

void push_expression(Parser* p, Expression* exp);
Expression* pop_expression(Parser* p);

extern Precedence precedences[];

#endif