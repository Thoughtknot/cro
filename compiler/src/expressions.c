#include "expressions.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

MAKE_DYNAMIC_ARRAY(Type)
MAKE_DYNAMIC_ARRAY(Identifier)
MAKE_LINKED_LIST(Param)
MAKE_LINKED_LIST(Statement)
MAKE_LINKED_LIST(Elif)

void print_type(Type tp) {
    if (tp.quantifier.length != 0) {
        printf("%s ", tp.quantifier.value);
    }
    printf("%s", tp.type.value);
    if (tp.array) {
        printf("[]");
    }
    printf(" ");
}

void print_type_array(TypeArray* tp) {
    if (tp->count == 1) {
        print_type(tp->data[0]);
    } else {
        printf("(");
        for (int i = 0; i < tp->count; i++) {
            print_type(tp->data[i]);
            if (i < tp->count - 1) printf(", ");
        }
        printf(")");
    }
}

void make_fn_call_list(ExpressionList* list, Parser* p) {
    read_next_lexeme(p);
    if (get_current_lexeme(p).token == T_RPAREN) {
        return;
    }
    p->currentLexeme--;
    while (true) {
        read_expression(p, P_ASS);
        Expression* value = pop_expression(p);
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
    Identifier quantifier = {.value = NULL, .length=0};
    Identifier name;
    if (get_current_lexeme(p).token == T_MAYBE) {
        quantifier.length = get_current_lexeme(p).length;
        quantifier.value = malloc(sizeof(quantifier.length + 1));
        memcpy(quantifier.value, get_current_lexeme(p).start, quantifier.length);
        quantifier.value[quantifier.length] = '\0';
        read_next_lexeme(p);
    }
    if (get_current_lexeme(p).token == T_IDENTIFIER) {
        name.length = get_current_lexeme(p).length;
        name.value = malloc(sizeof(name.length + 1));
        memcpy(name.value, get_current_lexeme(p).start, name.length);
        name.value[name.length] = '\0';
    } else {
        parser_error(p, "Reading identifier - but not finding identifier");
    }
    t.type = name;
    t.quantifier = quantifier;
    read_next_lexeme(p);
    if (get_current_lexeme(p).token == T_LSPAREN) {
        read_expected(p, T_RSPAREN);
        t.array = true;
    } else {
        print_lexeme(get_current_lexeme(p));
        p->currentLexeme--;
        t.array = false;
    }
    return t;
}

TypeArray* read_type_array(Parser* p) {
    TypeArray* tp = make_array_Type();
    if (get_current_lexeme(p).token == T_LPAREN) {
        do {
            read_next_lexeme(p);
            write_Type(tp, read_type(p));
            read_next_lexeme(p);
        }
        while (get_current_lexeme(p).token == T_COMMA);
    } 
    else {
        write_Type(tp, read_type(p));
    }
    return tp;
}

Type get_type(Parser* p, Expression* exp) {
    switch (exp->type) {
        case EXP_NONELIT: {
            //parser_error(p, "Can't determine type of None\n");
        }
        case EXP_JUSTLIT: {
            MaybeExpression* mb = (MaybeExpression*) exp;
            Type tp = get_type(p, mb->value);
            tp.quantifier=(Identifier) {.value="just", .length=4};
            return tp;
        }
        case EXP_OBJLIT: {
            ObjLitExpression* ol = (ObjLitExpression*) exp; 
            Type tp = {.array=false, .quantifier=(Identifier){.length=0,.value=NULL}, .type=ol->name};
            return tp;
        }
        case EXP_BOOLLIT: {
            Type tp = {.array=false, .quantifier=(Identifier){.length=0,.value=NULL}, .type=(Identifier){.length=4,.value="bool"} };
            return tp;
        }
        case EXP_ARRAYLIT: {
            ArrayLitExpression* b = (ArrayLitExpression*) exp;
            return b->type;
        }
        case EXP_ARRAYREF: {
            ArrayRefExpression* b = (ArrayRefExpression*) exp;
            Type tp = get_type(p, b->e);
            tp.array = false;
            return tp;
        }
        case EXP_BINARY: {
            BinaryExpression* b = (BinaryExpression*) exp;
            return get_type(p, b->l);
        }
        case EXP_FNCALL: {
            //parser_error(p, "Can't get type of fncall\n");
        }
        case EXP_PAREN: {
            ParenExpression* pe = (ParenExpression*) exp;
            return get_type(p, pe->value);
        }
        case EXP_UNARY: {
            UnaryExpression* u = (UnaryExpression*) exp;
            return get_type(p, u->r);
        }
        case EXP_NUMLIT: {
            Type tp = {.array=false, .quantifier=(Identifier){.length=0,.value=NULL}, .type=(Identifier){.length=4,.value="num"}};
            return tp;
        }
        case EXP_INTLIT: {
            Type tp = {.array=false, .quantifier=(Identifier){.length=0,.value=NULL}, .type=(Identifier){.length=4,.value="int"}};
            return tp;
        }
        case EXP_STRLIT: {
            Type tp = {.array=false, .quantifier=(Identifier){.length=0,.value=NULL}, .type=(Identifier){.length=4,.value="string"}};
            return tp;
        }
        case EXP_VARREF: {
            //parser_error(p, "Can't get type of varref\n");
        }
        default: {
            printf("EXP[%d]", exp->type);
            //parser_error(p, "Can't get type of expression\n");
        }
    }
    return (Type) {.array=false, .quantifier=(Identifier){.length=0,.value=NULL}, .type=(Identifier){.length=0,.value=NULL}};
}

IdentifierArray* read_identifier_list(Parser* p) {
    IdentifierArray* arr = make_array_Identifier();
    Identifier i = read_identifier(p);
    write_Identifier(arr, i);
    while (true) {
        read_next_lexeme(p);
        if (get_current_lexeme(p).token != T_PERIOD) {
            p->currentLexeme--;
            break;
        }
        read_next_lexeme(p);
        Identifier ident = read_identifier(p);
        write_Identifier(arr, ident);
    }
    return arr;
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

Expression* make_string_exp(Parser* p) {
    Identifier name;
    if (get_current_lexeme(p).token == T_STRING) {
        name.length = get_current_lexeme(p).length - 2;
        name.value = malloc(sizeof(name.length + 1));
        memcpy(name.value, get_current_lexeme(p).start + 1, name.length);
        name.value[name.length] = '\0';
    } else {
        parser_error(p, "Reading string - but not finding string");
    }
    StringExpression* str = (StringExpression*) malloc(sizeof(*str));
    str->exp.type = EXP_STRLIT;
    str->value = name;
    return (Expression*) str;
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

Expression* make_tuple_exp(Parser* p) {
    TupleExpression* exp = (TupleExpression*) malloc(sizeof(*exp));
    exp->exp.type = EXP_TUPLE;
    exp->vals = make_Expression_list();
    Expression* fst = pop_expression(p);
    add_Expression(exp->vals, fst);
    read_expression(p, P_ASS);
    Expression* snd = pop_expression(p);
    add_Expression(exp->vals, snd);
    while (true) {
        read_next_lexeme(p);
        if (get_current_lexeme(p).token == T_COMMA) {
            read_expression(p, P_ASS);
            Expression* e = pop_expression(p);
            add_Expression(exp->vals, e);
        } else {
            p->currentLexeme--;
            break;
        }
    }
    return (Expression*) exp;
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
    IdentifierArray* value = read_identifier_list(p);
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

Expression* make_obj_lit(Parser* p, Identifier name) {
    ObjLitExpression* obj = (ObjLitExpression*) malloc(sizeof(*obj));
    obj->exp.type = EXP_OBJLIT;
    obj->name = name;
    obj->args = make_Expression_list();
    while (true) {
        read_next_lexeme(p);
        if (get_current_lexeme(p).token == T_RPAREN) {
            break;
        } else if (get_current_lexeme(p).token == T_COMMA) {
            read_expression(p, P_ASS);
            Expression* exp = pop_expression(p);
            add_Expression(obj->args, exp);
        } else {
            p->currentLexeme--;
            read_expression(p, P_ASS);
            Expression* exp = pop_expression(p);
            add_Expression(obj->args, exp);
        }
    }
    return (Expression*) obj;
}

Expression* make_array_lit(Parser* p) {
    Identifier quantifier = {.value = NULL, .length=0};
    Identifier name = read_identifier(p);
    ArrayLitExpression* obj = (ArrayLitExpression*) malloc(sizeof(*obj));
    obj->exp.type = EXP_ARRAYLIT;
    obj->type = (Type) {.quantifier=quantifier, .type=name, .array=true};
    obj->args = make_Expression_list();
    read_expected(p, T_LSPAREN);
    while (true) {
        read_next_lexeme(p);
        if (get_current_lexeme(p).token == T_RSPAREN) {
            break;
        } else if (get_current_lexeme(p).token == T_COMMA) {
            read_expression(p, P_ASS);
            Expression* exp = pop_expression(p);
            add_Expression(obj->args, exp);
        } else {
            p->currentLexeme--;
            read_expression(p, P_ASS);
            Expression* exp = pop_expression(p);
            add_Expression(obj->args, exp);
        }
    }
    return (Expression*) obj;
}

Expression* make_allocation_lit(Parser* p) {
    read_next_lexeme(p);
    read_next_lexeme(p);
    if (get_current_lexeme(p).token == T_LPAREN) {
        p->currentLexeme--;
        Identifier name = read_identifier(p);
        p->currentLexeme++;
        return make_obj_lit(p, name);
    }
    else {
        p->currentLexeme--;
        return make_array_lit(p);
    }
}

Expression* make_none(Parser* p) {
    MaybeExpression* exp = (MaybeExpression*) malloc(sizeof(*exp));
    exp->exp.type = EXP_NONELIT;
    exp->value = NULL;
    return (Expression*) exp;
}

Expression* make_just(Parser* p) {
    MaybeExpression* exp = (MaybeExpression*) malloc(sizeof(*exp));
    exp->exp.type = EXP_JUSTLIT;
    read_expression(p, P_ASS);
    exp->value = pop_expression(p);
    return (Expression*) exp;
}

Expression* make_bool_exp(Parser* p, bool b) {
    BooleanExpression* be = (BooleanExpression*) malloc(sizeof(*be));
    be->exp.type = EXP_BOOLLIT;
    be->value = b;
    return (Expression*) be;
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

Expression* make_array_ref_exp(Parser* p) {
    p->currentLexeme--;
    read_expression(p, P_ASS);
    p->currentLexeme++;
    ArrayRefExpression* ar = (ArrayRefExpression*) malloc(sizeof(*ar));
    ar->exp.type = EXP_ARRAYREF;
    ar->index = pop_expression(p);
    ar->e = pop_expression(p);
    if (get_current_lexeme(p).token != T_RSPAREN)
        parser_error(p, "Error detected - expected ]\n");
    else
        p->currentLexeme++;
    return (Expression*) ar;
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
    TypeArray* ret = read_type_array(p);
    
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
    while (true) {
        read_next_lexeme(p);
        if (get_current_lexeme(p).token == T_RCPAREN) break;
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

Statement* make_fn_call_stmt(Parser* p, IdentifierArray* name) {
    FnCallStatement* fnCall = (FnCallStatement*) malloc(sizeof(*fnCall));
    fnCall->stmt.type = STMT_FNCALL;
    fnCall->name = name;
    fnCall->list = make_Expression_list();
    make_fn_call_list(fnCall->list, p);
    return (Statement*) fnCall;
}

Statement* make_return(Parser* p) {
    read_expression(p, P_ASS);
    Expression* exp = pop_expression(p);
    ReturnStatement* ret = (ReturnStatement*) malloc(sizeof(*ret));
    ret->stmt.type = STMT_RET;
    ret->retVal = exp;
    return (Statement*) ret;
}

Statement* make_loop(Parser* p) {
    read_expression(p, P_ASS);
    Expression* cond = pop_expression(p);
    read_expected(p, T_LCPAREN);

    LoopStatement* stmt = (LoopStatement*) malloc(sizeof(*stmt));
    stmt->stmt.type = STMT_LOOP;
    stmt->cond = cond;
    stmt->body = make_Statement_list();
    while (true) {
        Statement* s = read_statement(p);
        if (s == NULL) {
            break;
        }
        print_statement(s);
        add_Statement(stmt->body, s);
    }
    if (get_current_lexeme(p).token != T_RCPAREN) read_expected(p, T_RCPAREN);
    return (Statement*) stmt;
}
    
Statement* make_variable_assign(Parser* p, IdentifierArray* name) {
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

    read_next_lexeme(p);
    VarDeclStatement* varDecl = (VarDeclStatement*) malloc(sizeof(*varDecl));
    varDecl->stmt.type = STMT_VARDECL;
    varDecl->name = name;
    bool implicit_type;
    if (get_current_lexeme(p).token == T_COLON) {
        read_expected(p, T_EQ);
        implicit_type = true;
    }
    else {
        p->currentLexeme--;
        implicit_type = false;
        varDecl->tp = read_type(p);
    }
    
    read_expression(p, P_ASS);
    Expression* value = pop_expression(p);
    varDecl->value = value;
    if (implicit_type) {
        varDecl->tp = get_type(p, value);
    }
    return (Statement*) varDecl;
}

Statement* make_typedef(Parser* p) {
    read_next_lexeme(p);
    Identifier name = read_identifier(p);
    read_expected(p, T_IS);
    read_next_lexeme(p);
    Identifier objName = read_identifier(p);
    read_next_lexeme(p);
    Identifier varName = read_identifier(p);
    read_next_lexeme(p);
    ExpressionList* list = make_Expression_list();
    if (get_current_lexeme(p).token == T_COLON) {
        read_expected(p, T_LCPAREN);
        while (true) {
            read_next_lexeme(p);
            if (get_current_lexeme(p).token == T_RCPAREN) break;
            p->currentLexeme--;
            read_expression(p, P_ASS);
            Expression* exp = pop_expression(p);
            add_Expression(list, exp);
            read_next_lexeme(p);
            if (get_current_lexeme(p).token == T_COMMA) continue;
            if (get_current_lexeme(p).token == T_RCPAREN) break;
        }
    }

    DefineStatement* stmt = (DefineStatement*) malloc(sizeof(*stmt));
    stmt->stmt.type = STMT_TYPEDEF;
    stmt->typeName = name;
    stmt->objectName = objName;
    stmt->varName = varName;
    stmt->conditions = list;
    return (Statement*) stmt;
}

Statement* make_if(Parser* p) {
    IfStatement* stmt = (IfStatement*) malloc(sizeof(*stmt));
    stmt->stmt.type = STMT_IF;
    stmt->elifs = make_Elif_list();
    stmt->ifBody = make_Statement_list();
    stmt->elseBody = make_Statement_list();

    read_expression(p, P_ASS);
    Expression* cond = pop_expression(p);
    stmt->cond = cond;
    read_expected(p, T_LCPAREN);
    while (true) {
        Statement* s = read_statement(p);
        if (s == NULL) {
            break;
        }
        print_statement(s);
        add_Statement(stmt->ifBody, s);
    }
    if (get_current_lexeme(p).token != T_RCPAREN) read_expected(p, T_RCPAREN);

    while(true) {
        read_next_lexeme(p);
        if (get_current_lexeme(p).token == T_ELIF) {
            read_expression(p, P_ASS);
            Expression* elifCond = pop_expression(p);
            Elif* elif = (Elif*) malloc(sizeof(*elif));
            elif->cond = elifCond;
            elif->body = make_Statement_list();
            read_expected(p, T_LCPAREN);
            while (true) {
                Statement* s = read_statement(p);
                if (s == NULL) {
                    break;
                }
                print_statement(s);
                add_Statement(elif->body, s);
            }
            if (get_current_lexeme(p).token != T_RCPAREN) read_expected(p, T_RCPAREN);
            add_Elif(stmt->elifs, elif);
        } else if (get_current_lexeme(p).token == T_ELSE) {
            read_expected(p, T_LCPAREN);
            while (true) {
                Statement* s = read_statement(p);
                if (s == NULL) {
                    break;
                }
                print_statement(s);
                add_Statement(stmt->elseBody, s);
            }
            if (get_current_lexeme(p).token != T_RCPAREN) read_expected(p, T_RCPAREN);
            break;
        } else {
            p->currentLexeme--;
            break;
        }
    }
    return (Statement*) stmt;
}

void print_identifier_array(IdentifierArray* a) {
    for (int i = 0; i < a->count; i++) {
        printf("%s", a->data[i].value);
        if (i < a->count - 1) {
            printf(".");
        }
    }
}

void print_statement(Statement* stmt) {
    switch (stmt->type) {
        case STMT_TYPEDEF: {
            DefineStatement* def = (DefineStatement*) stmt;
            printf("Define[%s %s] as [%s]: {", def->objectName.value, def->varName.value, def->typeName.value); 
            ExpressionNode* arg = def->conditions->head;
            while (arg != NULL) {
                print_expression(arg->current);
                arg = arg->next;
            }
            printf("}\n");
            break;
        }
        case STMT_RET: {
            ReturnStatement* ret = (ReturnStatement*) stmt;
            printf("Return[");
            print_expression(ret->retVal);
            printf("]\n");
            break;
        }
        case STMT_IF: {
            IfStatement* ifs = (IfStatement*) stmt;
            printf("If[");
            print_expression(ifs->cond);
            printf("] {\n");
            StatementNode* arg = ifs->ifBody->head;
            while (arg != NULL) {
                printf("  ");
                print_statement(arg->current);
                arg = arg->next;
            }
            printf("}\n");
            ElifNode* elifNode = ifs->elifs->head;
            while (elifNode != NULL) {
                printf("ElIf[");
                print_expression(elifNode->current->cond);
                printf("] {\n");
                StatementNode* elifSNode = elifNode->current->body->head;
                while (elifSNode != NULL) {
                    printf("  ");
                    print_statement(elifSNode->current);
                    elifSNode = elifSNode->next;
                }
                printf("}\n");
                elifNode = elifNode->next;
            }
            printf("Else {\n");
            StatementNode* elseS = ifs->elseBody->head;
            while (elseS != NULL) {
                printf("  ");
                print_statement(elseS->current);
                elseS = elseS->next;
            }
            printf("}\n");
            break;
        }
        case STMT_VARDECL: {
            VarDeclStatement* vd = (VarDeclStatement*) stmt;
            printf("VarDecl[%s]: {", vd->name.value);
            print_expression(vd->value);
            printf("}\n");
            break;
        }
        case STMT_VARASS: {
            VarAssStatement* va = (VarAssStatement*) stmt;
            printf("VarAss[");
            print_identifier_array(va->name);
            printf("]: {");
            print_expression(va->value);
            printf("}\n");
            break;
        }
        case STMT_OBJDECL: {
            ObjDeclStatement* od = (ObjDeclStatement*) stmt;
            printf("ObjDecl[%s](", od->name.value);
            ParamNode* node = od->fields->head;
            while (node != NULL) {
                print_type(node->current->type);
                printf("%s", node->current->name.value);
                node = node->next;
                if (node != NULL) printf(", ");
            }
            printf(")\n");
            break;
        }
        case STMT_LOOP: {
            LoopStatement* ls = (LoopStatement*) stmt;
            printf("Loop[");
            print_expression(ls->cond);
            printf("] {\n");
            StatementNode* arg = ls->body->head;
            while (arg != NULL) {
                printf("  ");
                print_statement(arg->current);
                arg = arg->next;
            }
            printf("}\n");
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
            print_type_array(fd->ret);
            printf("{\n");

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
            printf("FnCall[");
            for (int i = 0; i < fc->name->count; i++) {
                printf("%s", fc->name->data[i].value);
                if (i < fc->name->count - 1) {
                    printf(".");
                }
            }
            printf("]: {");
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
        case EXP_NONELIT:
            printf("None[]");
            break;
        case EXP_JUSTLIT: {
            MaybeExpression* mb = (MaybeExpression*) exp;
            printf("Just[");
            print_expression(mb->value);
            printf("]");
            break;
        }
        case EXP_OBJLIT: {
            ObjLitExpression* ol = (ObjLitExpression*) exp;
            printf("ObjLit[%s]{", ol->name.value);
            ExpressionNode* arg = ol->args->head;
            while (arg != NULL) {
                print_expression(arg->current);
                arg = arg->next;
                if (arg != NULL) printf(", ");
            }
            printf("}");
            break;
        }
        case EXP_BOOLLIT: {
            BooleanExpression* b = (BooleanExpression*) exp;
            printf("Bool[%s]", b->value?"true":"false");
            break;
        }
        case EXP_ARRAYLIT: {
            ArrayLitExpression* b = (ArrayLitExpression*) exp;
            printf("Array[");
            print_type(b->type);
            printf("][");
            ExpressionNode* node = b->args->head;
            while (node != NULL) {
                print_expression(node->current);
                node = node->next;
                if (node != NULL) printf(", ");
            }
            printf("]");
            break;
        }
        case EXP_ARRAYREF: {
            ArrayRefExpression* b = (ArrayRefExpression*) exp;
            printf("ArrayRef[");
            print_expression(b->e);
            printf("][");
            print_expression(b->index);
            printf("]");
            break;
        }
        case EXP_BINARY: {
            BinaryExpression* b = (BinaryExpression*) exp;
            printf("Binary[%s] {", tokens[b->operator]);
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
            printf("}");
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
            printf("VarRef[");
            print_identifier_array(v->varName);
            printf("]");
            break;
        }
        default:
            printf("EXP[%d]", exp->type);
            break;
    }
}