#include "interpreter.h"
#include <stdio.h>

#define VARIABLE_SIZE 0x100

MAKE_LINKED_LIST(ObjDeclStatement)
MAKE_LINKED_LIST(FnDeclStatement)
MAKE_LINKED_LIST(Variable)

void init_interpreter(Interpreter* i) {
    i->fns = make_FnDeclStatement_list();
    i->objs = make_ObjDeclStatement_list();
    i->scope = (Scope) {.list=make_Variable_list()};
}

void interpret_statement(Interpreter* i, Statement* stmt) {

}

Value interpret_expression(Interpreter* i, Expression* exp) {

}

void interpret_fn_call(Interpreter* i, FnDeclStatement* fn, ExpressionList* args) {
    ParamNode* node = fn->parameters->head;
    ExpressionNode* arg = args->head;
    while (node != NULL) {
        Variable* v = (Variable*) malloc(sizeof(*v));
        v->name = node->current->name.value;
        v->value = interpret_expression(i, arg->current);
        add_Variable(i->scope.list, v);
        node = node->next;
        arg = arg->next;
    }
    StatementNode* stmt = fn->body->head;
    while (stmt != NULL) {
        interpret_statement(i, stmt);
    }
}

void interpret(Interpreter* i, Parser* p) {
    StatementNode* stmt = parse(p)->head;
    printf("****** STARTING INTERPPRETER ******\n");
    while (stmt != NULL) {
        if (stmt->current->type == STMT_FNDECL) {
            add_FnDeclStatement(i->fns, (FnDeclStatement*) stmt->current);
        } else if (stmt->current->type == STMT_OBJDECL) {
            add_ObjDeclStatement(i->objs, (ObjDeclStatement*) stmt->current);
        } else {
            printf("NYI [%d]\n", stmt->current->type);
        }
        stmt = stmt->next;
    }
    
    printf("****** FINISHED ******\n");
}