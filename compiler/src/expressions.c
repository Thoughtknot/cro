#include "expressions.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

MAKE_LINKED_LIST(Param)
MAKE_LINKED_LIST(Statement)

void make_fn_call_list(ExpressionList* list, Parser* p) {
    read_next_lexeme(p);
    if (get_current_lexeme(p).token == T_RPAREN) {
        return;
    }
    p->currentLexeme--;
    while (true) {
        read_expression(p, P_ASS);
        printf("Expression stack size: %d\n", p->stack->size);
        Expression* value = pop_expression(p);
        fflush(stdout);
        print_expression(value);
        add_Expression(list, value);
        read_next_lexeme(p);
        if (get_previous_lexeme(p).token == T_RPAREN) {
            p->currentLexeme--;
            break;
        }
        if (get_current_lexeme(p).token == T_RPAREN) break;
        if (get_current_lexeme(p).token != T_COMMA) parser_error(p, "Expected comma in fn call");
    }
}

Type read_type(Parser* p) {
    Type t;
    Identifier name;
    Lexeme current = get_current_lexeme(p);
    if (current.token == T_IDENTIFIER) {
        name.length = current.length;
        name.value = malloc(sizeof(name.length + 1));
        memcpy(name.value, current.start, name.length);
        name.value[name.length] = '\0';
    } else {
        parser_error(p, "Reading identifier - but not finding identifier");
    }
    t.type = name;
    read_next_lexeme(p);
    if (current.token == T_LSPAREN) {
        read_expected(p, T_RSPAREN);
        t.array = true;
    } else {
        p->currentLexeme--;
        t.array = false;
    }
    return t;
}

Identifier read_identifier(Parser* p) {
    Identifier name;
    Lexeme current = get_current_lexeme(p);
    if (current.token == T_IDENTIFIER) {
        name.length = current.length;
        name.value = malloc(sizeof(name.length + 1));
        memcpy(name.value, current.start, name.length);
        name.value[name.length] = '\0';
    } else {
        parser_error(p, "Reading identifier - but not finding identifier");
    }
    return name;
}

Identifier read_float(Parser* p) {
    Identifier name;
    if (get_current_lexeme(p).token == T_FLOAT) {
        name.length = get_current_lexeme(p).length;
        name.value = malloc(sizeof(name.length + 1));
        memcpy(name.value, get_current_lexeme(p).start, name.length);
        name.value[name.length] = '\0';
    } else {
        parser_error(p, "Reading float - but not finding float");
    }
    return name;
}

Expression* make_float(Parser* p) {
    Identifier name;
    if (get_current_lexeme(p).token == T_FLOAT) {
        name.length = get_current_lexeme(p).length;
        name.value = malloc(sizeof(name.length + 1));
        memcpy(name.value, get_current_lexeme(p).start, name.length);
        name.value[name.length] = '\0';
    } else {
        parser_error(p, "Reading float - but not finding float");
    }
    FloatExpression* floatVal = (FloatExpression*) malloc(sizeof(*floatVal));
    floatVal->exp.type = EXP_NUMLIT;
    floatVal->value = name;
    return (Expression*) floatVal;
}

Expression* make_integer(Parser* p) {
    Identifier name;
    if (get_current_lexeme(p).token == T_INTEGER) {
        name.length = get_current_lexeme(p).length;
        name.value = malloc(sizeof(name.length + 1));
        memcpy(name.value, get_current_lexeme(p).start, name.length);
        name.value[name.length] = '\0';
    } else {
        parser_error(p, "Reading int - but not finding int");
    }
    IntegerExpression* intval = (IntegerExpression*) malloc(sizeof(*intval));
    intval->exp.type = EXP_INTLIT;
    intval->value = name;
    return (Expression*) intval;
}

Expression* make_var_ref(Parser* p) {
    Identifier value = read_identifier(p);
    VarRefExpression* varRef = (VarRefExpression*) malloc(sizeof(*varRef));
    varRef->exp.type = EXP_VARREF;
    varRef->varName = value;
    return (Expression*) varRef;
}

Expression* make_paren(Parser* p) {
    read_expression(p, P_ASS);
    ParenExpression* paren = (ParenExpression*) malloc(sizeof(*paren));
    paren->exp.type = EXP_PAREN;
    paren->value = pop_expression(p);
    read_expected(p, T_RPAREN);
    return (Expression*) paren;
}

Expression* make_unary(Parser* p) {
    Token operator = get_current_lexeme(p).token;
    read_expression(p, P_UNARY);
    UnaryExpression* unary = (UnaryExpression*) malloc(sizeof(*unary));
    unary->exp.type = EXP_UNARY;
    unary->r = pop_expression(p);
    unary->operator = operator;
    return (Expression*) unary;
}

Expression* make_binary(Parser* p) {
    Token operator = get_previous_lexeme(p).token;
    p->currentLexeme--;
    read_expression(p, (Precedence) (precedences[operator] + 1));
    p->currentLexeme++;

    BinaryExpression* binary = (BinaryExpression*) malloc(sizeof(*binary));
    binary->exp.type = EXP_BINARY;
    binary->l = pop_expression(p);
    binary->r = pop_expression(p);
    binary->operator = operator;
    return (Expression*) binary;
}

Expression* make_field_ref_exp(Parser* p) {
    p->currentLexeme--;
    read_expression(p, P_PAREN);
    Expression* exp1 = pop_expression(p);
    Expression* exp = pop_expression(p);
    printf("fieldRef:{");
    print_expression(exp1);
    print_expression(exp);
    printf("}\n");
    FieldRefExpression* fr = (FieldRefExpression*) malloc(sizeof(*fr));
    fr->exp.type = EXP_FIELDREF;
    if (exp1->type == EXP_VARREF) {
        VarRefExpression* v = (VarRefExpression*) exp1;
        fr->name = v->varName;
        free(v);
    }
    fr->obj = exp;

    //?? can't be right
    p->currentLexeme++;
    //

    return (Expression*) fr;
}

Expression* make_fn_call_exp(Parser* p) {
    FnCallExpression* fnCall = (FnCallExpression*) malloc(sizeof(*fnCall));
    fnCall->exp.type = EXP_FNCALL;
    fnCall->fn = pop_expression(p);
    fnCall->list = make_Expression_list();
    if (get_current_lexeme(p).token != T_LPAREN)
        p->currentLexeme--;
    make_fn_call_list(fnCall->list, p);
    if (get_current_lexeme(p).token == T_RPAREN)
        read_next_lexeme(p);
    return (Expression*) fnCall;
}
Statement* make_fn_decl(Parser* p) {
    read_next_lexeme(p);
    Identifier name = read_identifier(p);
    read_expected(p, T_LPAREN);

    ParamList* list = make_Param_list();
    while (true) {
        read_next_lexeme(p);
        if (get_current_lexeme(p).token == T_IDENTIFIER) {
            if (get_previous_lexeme(p).token != T_LPAREN &&
                get_previous_lexeme(p).token != T_COMMA) {
                parser_error(p, "Invalid function call");
            }
            Type type = read_type(p);
            read_next_lexeme(p);
            Identifier name = read_identifier(p);
            Param* param = malloc(sizeof(*param));
            param->type = type;
            param->name = name;
            add_Param(list, param);
        } else if (get_current_lexeme(p).token == T_RPAREN) {
            break;
        } else if (get_current_lexeme(p).token == T_COMMA) {
            continue;
        } else {
            parser_error(p, "Expected parameter or right paren, got other token");
        }
    }
    read_next_lexeme(p);
    Type ret = read_type(p);
    
    read_expected(p, T_LCPAREN);
    StatementList* slist = make_Statement_list();
    while (true) {
        Statement* stmt = read_statement(p);
        if (stmt == NULL) {
            break;
        }
        print_statement(stmt);
        add_Statement(slist, stmt);
    }
    
    FnDeclStatement* fnDecl = (FnDeclStatement*) malloc(sizeof(*fnDecl));
    fnDecl->stmt.type = STMT_FNDECL;
    fnDecl->name = name;
    fnDecl->parameters = list;
    fnDecl->body = slist;
    fnDecl->ret = ret;

    return (Statement*) fnDecl;
}

Statement* make_obj_decl(Parser* p) {
    read_next_lexeme(p);
    Identifier name = read_identifier(p);

    ObjDeclStatement* stmt = (ObjDeclStatement*) malloc(sizeof(*stmt));
    stmt->stmt.type = STMT_OBJDECL;
    stmt->fields = make_Param_list();
    stmt->name = name;

    read_expected(p, T_LCPAREN);
    bool first = true;
    while (true) {
        read_next_lexeme(p);
        if (get_current_lexeme(p).token == T_RCPAREN) break;
        if (first) {
            first = false;
            p->currentLexeme--;
        }
        else {
            if (get_current_lexeme(p).token != T_COMMA) 
                parser_error(p, "Expected comma");
        }
        read_next_lexeme(p);
        Type tp = read_type(p);
        read_next_lexeme(p);
        Identifier name = read_identifier(p);
        Param* param = malloc(sizeof(*param));
        param->type = tp;
        param->name = name;
        add_Param(stmt->fields, param);
    }
    return (Statement*) stmt;
}

Statement* make_fn_call_stmt(Parser* p, Identifier name) {
    FnCallStatement* fnCall = (FnCallStatement*) malloc(sizeof(*fnCall));
    fnCall->stmt.type = STMT_FNCALL;
    fnCall->name = name;
    fnCall->list = make_Expression_list();
    make_fn_call_list(fnCall->list, p);
    return (Statement*) fnCall;
}

Statement* make_variable_assign(Parser* p, Identifier name) {
    read_expression(p, P_ASS);
    Expression* value = pop_expression(p);
    VarAssStatement* varAss = (VarAssStatement*) malloc(sizeof(*varAss));
    varAss->stmt.type = STMT_VARASS;
    varAss->name = name;
    varAss->value = value;
    return (Statement*) varAss;
}

Statement* make_variable_decl(Parser* p) {
    read_next_lexeme(p);
    Identifier name = read_identifier(p);
    read_expected(p, T_COLON);
    read_expected(p, T_EQ);
    
    read_expression(p, P_ASS);
    Expression* value = pop_expression(p);

    VarDeclStatement* varDecl = (VarDeclStatement*) malloc(sizeof(*varDecl));
    varDecl->stmt.type = STMT_VARDECL;
    varDecl->name = name;
    varDecl->value = value;
    return (Statement*) varDecl;
}

void print_statement(Statement* stmt) {
    switch (stmt->type) {
        case STMT_VARDECL: {
            VarDeclStatement* vd = (VarDeclStatement*) stmt;
            printf("VarDecl[%s]: {", vd->name.value);
            print_expression(vd->value);
            printf("}\n");
            break;
        }
        case STMT_VARASS: {
            VarAssStatement* va = (VarAssStatement*) stmt;
            printf("VarAss[%s]: {", va->name.value);
            print_expression(va->value);
            printf("}\n");
            break;
        }
        case STMT_OBJDECL: {
            ObjDeclStatement* od = (ObjDeclStatement*) stmt;
            printf("ObjDecl[%s](", od->name.value);
            ParamNode* node = od->fields->head;
            while (node != NULL) {
                printf("%s %s", node->current->type.type.value, node->current->name.value);
                node = node->next;
                if (node != NULL) printf(", ");
            }
            printf(")\n");
            break;
        }
        case STMT_FNDECL: {
            FnDeclStatement* fd = (FnDeclStatement*) stmt;
            printf("FnDecl[%s](", fd->name.value);
            ParamNode* node = fd->parameters->head;
            while (node != NULL) {
                printf("%s %s", node->current->type.type.value, node->current->name.value);
                node = node->next;
                if (node != NULL) printf(", ");
            }
            printf(")");
            printf(" %s {\n", fd->ret.type.value);

            StatementNode* arg = fd->body->head;
            while (arg != NULL) {
                printf("  ");
                print_statement(arg->current);
                arg = arg->next;
            }
            printf("}\n");
            break;
        }
        case STMT_FNCALL: {
            FnCallStatement* fc = (FnCallStatement*) stmt;
            printf("FnCall[%s]: {", fc->name.value);
            ExpressionNode* arg = fc->list->head;
            while (arg != NULL) {
                print_expression(arg->current);
                arg = arg->next;
            }
            printf("}\n");
            break;
        }
        default: {
            printf("Stmt[%d]\n", stmt->type);
            break;
        }
    }
}

void print_expression(Expression* exp) {
    switch (exp->type) {
        case EXP_FIELDREF: {
            FieldRefExpression* fr = (FieldRefExpression*) exp;
            printf("FieldRef[%s]{", fr->name.value);
            print_expression(fr->obj);
            printf("}");
            break;
        }
        case EXP_BINARY: {
            BinaryExpression* b = (BinaryExpression*) exp;
            printf("Binary[%d] {", b->operator);
            print_expression(b->l);
            printf(" ");
            print_expression(b->r);
            printf("}");
            break;
        }
        case EXP_FNCALL: {
            FnCallExpression* fc = (FnCallExpression*) exp;
            printf("FnCall[");
            print_expression(fc->fn);
            printf("]: {");
            ExpressionNode* arg = fc->list->head;
            while (arg != NULL) {
                print_expression(arg->current);
                arg = arg->next;
            }
            printf("}\n");
            break;
        }
        case EXP_PAREN: {
            ParenExpression* pe = (ParenExpression*) exp;
            printf("Paren {");
            print_expression(pe->value);
            printf("}");
            break;
        }
        case EXP_UNARY: {
            UnaryExpression* u = (UnaryExpression*) exp;
            printf("Unary[%d] {", u->operator);
            print_expression(u->r);
            printf("}");
            break;
        }
        case EXP_NUMLIT: {
            FloatExpression* f = (FloatExpression*) exp;
            printf("Float[%s]", f->value.value);
            break;
        }
        case EXP_INTLIT: {
            IntegerExpression* i = (IntegerExpression*) exp;
            printf("Int[%s]", i->value.value);
            break;
        }
        case EXP_STRLIT: {
            StringExpression* s = (StringExpression*) exp;
            printf("String[%s]", s->value.value);
            break;
        }
        case EXP_VARREF: {
            VarRefExpression* v = (VarRefExpression*) exp;
            printf("VarRef[%s]", v->varName.value);
            break;
        }
        default:
            printf("EXP[%d]", exp->type);
            break;
    }
}