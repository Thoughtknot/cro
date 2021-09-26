#include "parser.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "expressions.h"

MAKE_LINKED_LIST(Expression)
MAKE_DYNAMIC_ARRAY(Lexeme)

Precedence precedences[] = {
    [T_LPAREN] = P_PAREN,
    [T_RPAREN] = P_NONE,
    [T_LCPAREN] = P_NONE,
    [T_RCPAREN] = P_NONE,
    [T_COMMA] = P_NONE,
    [T_SEMIC] = P_NONE,
    [T_NOT] = P_NONE,
    [T_VAR] = P_NONE,
    [T_FN] = P_NONE,
    [T_IF] = P_NONE,
    [T_ELSE] = P_NONE,
    [T_ELIF] = P_NONE,
    [T_LOOP] = P_NONE,
    [T_STRING] = P_NONE,
    [T_FLOAT] = P_NONE,
    [T_INTEGER] = P_NONE,
    [T_MINUS] = P_ADD,
    [T_PLUS] = P_ADD,
    [T_MUL] = P_MUL,
    [T_DIV] = P_MUL,
    [T_EQQ] = P_CMP,
    [T_GT] = P_CMP,
    [T_GEQ] = P_CMP,
    [T_LT] = P_CMP,
    [T_LEQ] = P_CMP,
    [T_PERIOD] = P_PAREN,
};

void parser_error(Parser* p, const char* reason);

void print_stack(Parser* p) {
    printf("Expressions {");
    ExpressionNode* n = p->stack->head;
    while (n != NULL && n->current != NULL) {
        print_expression(n->current);
        n = n->next;
        if (n != NULL) {
            printf(", ");
        }
    }
    printf("}\n");
}

void push_expression(Parser* p, Expression* exp) {
    add_Expression(p->stack, exp);
}

Expression* pop_expression(Parser* p) {
    Expression* exp = pop_Expression(p->stack);
    return exp;
}

void read_next_lexeme(Parser* p) {
    if (p->read->count == p->currentLexeme) {
        Lexeme l = read_next_token(p->lexer);
        write_Lexeme(p->read, l);
        print_lexeme(l);
        if (l.token == T_ERROR) {
            print_lexeme(l);
            parser_error(p, "Lexer error");
        }
    }
    p->currentLexeme++;
}

void read_expected(Parser* p, Token token) {
    read_next_lexeme(p);
    if (get_current_lexeme(p).token != token) {
        char reason[50];
        sprintf(reason, "Expected token %d, was %d", token, get_current_lexeme(p).token);
        parser_error(p, reason);
    }
}

void read_expression(Parser* p, Precedence pr) {
    printf("READ EXP:");
    read_next_lexeme(p);
    Expression* exp;
    switch (get_current_lexeme(p).token) {
        case T_LPAREN:
            exp = make_paren(p);
            break;
        case T_MINUS:
        case T_NOT:
            exp = make_unary(p);
            break;
        case T_STRING:
            exp = make_string(p);
            break;
        case T_INTEGER:
            exp = make_integer(p);
            break;
        case T_FLOAT:
            exp = make_float(p);
            break;
        case T_IDENTIFIER:
            exp = make_var_ref(p);
            break;
        default:
            parser_error(p, "Syntax error, expected operand");
    }
    push_expression(p, exp);
    read_next_lexeme(p);
    while (precedences[get_current_lexeme(p).token] >= pr) {
        read_next_lexeme(p);
        switch (get_previous_lexeme(p).token) {
            case T_LPAREN:
                push_expression(p, make_fn_call_exp(p));
                break;
            case T_PERIOD:
                push_expression(p, make_field_ref_exp(p));
                break;
            case T_EQQ:
            case T_GT:
            case T_GEQ:
            case T_LT:
            case T_LEQ:
            case T_NEQ:
            case T_MINUS:
            case T_PLUS:
            case T_MUL:
            case T_DIV:
                push_expression(p, make_binary(p));
                break;
            default:
                parser_error(p, "Syntax error, expected operator");
        }
    }
    p->currentLexeme--;
}

Statement* read_statement(Parser* p) {
    read_next_lexeme(p);
    if (get_current_lexeme(p).token == T_VAR) {
        return make_variable_decl(p);
    } 
    else if (get_current_lexeme(p).token == T_FN) {
        return make_fn_decl(p);
    }
    else if (get_current_lexeme(p).token == T_OBJ) {
        return make_obj_decl(p);
    }
    else if (get_current_lexeme(p).token == T_IDENTIFIER) {
        Identifier name = read_identifier(p);
        read_next_lexeme(p);
        if (get_current_lexeme(p).token == T_EQ) {
            return make_variable_assign(p, name);
        }
        if (get_current_lexeme(p).token == T_LPAREN) {
            return make_fn_call_stmt(p, name);
        } 
        else {
            printf("NYI\n");
            parser_error(p, "Token NYI");
        }
    }
    else if (get_current_lexeme(p).token == T_RCPAREN) {
        return NULL;
    }
    else if (get_current_lexeme(p).token == T_EOF) {
        return NULL;
    }
    else {
        parser_error(p, "Statement starting on unknown token");
    }
    return NULL;
}

void parse(Parser* p) {
    while (true) {
        Statement* stmt = read_statement(p);
        if (stmt == NULL) break;
        print_statement(stmt);
    }
}

void init_parser(Parser* p) {
    p->stack = make_Expression_list();
    p->read = make_array_Lexeme();
}

void parser_error(Parser* p, const char* reason) {
    printf("Parsing error: %s\n at: ", reason);
    print_lexeme(get_current_lexeme(p));
    exit(21);
}

Lexeme get_previous_lexeme(Parser* p) {
    return p->read->data[p->currentLexeme - 2];
}

Lexeme get_current_lexeme(Parser* p) {
    return p->read->data[p->currentLexeme - 1];
}