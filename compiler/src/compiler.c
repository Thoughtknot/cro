#include "compiler.h"
#include "parser.h"
#include "expressions.h"
#include <string.h>
#include "../../common/src/types.h"

MAKE_DYNAMIC_ARRAY(Constant)

void init_compiler(Compiler* c) {
    FILE* file = fopen(c->path, "w");
    c->file = file;
    c->constantPool = make_array_Constant();
    c->currentPos = 0;
}

void push_int(Compiler* c, uint32_t b) {
    memcpy(&c->buffer[c->currentPos], &b, sizeof(b));
    c->currentPos += sizeof(b);
}
void push_short(Compiler* c, uint16_t b) {
    memcpy(&c->buffer[c->currentPos], &b, sizeof(b));
    c->currentPos += sizeof(b);
}
void push_byte(Compiler* c, int b) {
    c->buffer[c->currentPos++] = b;
}

uint16_t push_constant_value(Compiler* c, Value val) {
    for (int i = 0; i < c->constantPool->count; i++) {
        Constant cnst = c->constantPool->data[i];
        if (cnst.value.type != val.type) continue;
        switch (cnst.value.type) {
            case TP_INT:
                if (cnst.value.as.integer == val.as.integer)
                    return cnst.constantId;
                break;
            case TP_NUM:
                if (cnst.value.as.number == val.as.number)
                    return cnst.constantId;
                break;
            case TP_STR:
                if (strcmp(cnst.value.as.string, val.as.string) == 0)
                    return cnst.constantId;
                break;
            default:
                printf("ERROR: Can't push value of type %d\n", val.type);
                exit(1);
        }
    }
    
    Constant cnst = {.constantId = c->constantPool->count, .value = val};
    write_Constant(c->constantPool, cnst);
    
    push_short(c, cnst.constantId);
    push_byte(c, cnst.value.type);
    switch (cnst.value.type) {
            case TP_INT:
                memcpy(&c->buffer[c->currentPos], &cnst.value.as.integer, sizeof(cnst.value.as.integer));
                c->currentPos += 8;
                break;
            case TP_NUM:
                memcpy(&c->buffer[c->currentPos], &cnst.value.as.number, sizeof(cnst.value.as.number));
                c->currentPos += 8;
                break;
            case TP_STR: {
                uint16_t len = strlen(cnst.value.as.string);
                push_short(c, len);
                memcpy(&c->buffer[c->currentPos], cnst.value.as.string, len);
                c->currentPos += len;
                break;
            }
            default:
                printf("ERROR: Can't push value of type %d\n", val.type);
                exit(1);
    }

    return cnst.constantId;
}

uint16_t push_type_header(Compiler* c, Identifier type) {
    if (strcmp(type.value, KW_INTEGER) == 0) {
        // Do nothing
    } else if (strcmp(type.value, KW_STRING) == 0) {
        // Do nothing
    } else if (strcmp(type.value, KW_BOOL) == 0) {
        // Do nothing
    } else if (strcmp(type.value, KW_NUMBER) == 0) {
        // Do nothing
    } else if (strcmp(type.value, KW_BYTE) == 0) {
        // Do nothing
    } else {
        Value val = {.type=TP_STR, .as.string = type.value};
        return push_constant_value(c, val);
    }
    return 0x1234;
}
void push_type(Compiler* c, Type type) {
    if (strcmp(type.type.value, KW_INTEGER) == 0) {
        push_byte(c, TP_INT);
    } else if (strcmp(type.type.value, KW_STRING) == 0) {
        push_byte(c, TP_STR);
    } else if (strcmp(type.type.value, KW_BOOL) == 0) {
        push_byte(c, TP_BOOL);
    } else if (strcmp(type.type.value, KW_NUMBER) == 0) {
        push_byte(c, TP_NUM);
    }  else if (strcmp(type.type.value, KW_BYTE) == 0) {
        push_byte(c, TP_BYTE);
    } else {
        push_byte(c, TP_OBJ);
        push_short(c, type.type.constantId);
    }

    if (type.quantifier.value != NULL) {
        push_byte(c, 1);
        push_short(c, type.type.constantId);
    } else {
        push_byte(c, 0);
    }
    
    push_byte(c, type.array ? 1 : 0);
}

void push_constant(Compiler* c, Expression* exp) {
    switch (exp->type) {
        case EXP_NONELIT:
            // Do nothing
            break;
        case EXP_JUSTLIT: {
            MaybeExpression* mb = (MaybeExpression*) exp;
            push_constant(c, mb->value);
            break;
        }
        case EXP_ARRAYLIT: {
            ArrayLitExpression* al = (ArrayLitExpression*) exp;
            al->type.type.constantId = push_type_header(c, al->type.type);
            if (al->type.quantifier.value != NULL) {
                Value quantifier = {TP_STR, .as.string = al->type.quantifier.value};
                al->type.quantifier.constantId = push_constant_value(c, quantifier);
            }
            ExpressionNode* arg = al->args->head;
            while (arg != NULL) {
                push_constant(c, arg->current);
                arg = arg->next;
            }
            break;
        }
        case EXP_OBJLIT: {
            ObjLitExpression* ol = (ObjLitExpression*) exp;
            Value val = {.type=TP_STR, .as.string = ol->name.value};
            ol->exp.constantId = push_constant_value(c, val);
            ExpressionNode* arg = ol->args->head;
            while (arg != NULL) {
                push_constant(c, arg->current);
                arg = arg->next;
            }
            break;
        }
        case EXP_BOOLLIT: {
            // Do nothing
            break;
        }
        case EXP_ARRAYREF: {
            ArrayRefExpression* e = (ArrayRefExpression*) exp;
            push_constant(c, e->e);
            push_constant(c, e->index);
            break;
        }
        case EXP_BINARY: {
            BinaryExpression* b = (BinaryExpression*) exp;
            push_constant(c, b->l);
            push_constant(c, b->r);
            break;
        }
        case EXP_FNCALL: {
            FnCallExpression* fc = (FnCallExpression*) exp;
            ExpressionNode* arg = fc->list->head;
            push_constant(c, fc->fn);
            while (arg != NULL) {
                push_constant(c, arg->current);
                arg = arg->next;
            }
            break;
        }
        case EXP_PAREN: {
            ParenExpression* pe = (ParenExpression*) exp;
            push_constant(c, pe->value);
            break;
        }
        case EXP_UNARY: {
            UnaryExpression* u = (UnaryExpression*) exp;
            push_constant(c, u->r);
            break;
        }
        case EXP_NUMLIT: {
            FloatExpression* f = (FloatExpression*) exp;
            double val = strtod(f->value.value, (char**) NULL);
            Value v = {TP_NUM, .as.number = val};
            f->exp.constantId = push_constant_value(c, v);
            break;
        }
        case EXP_INTLIT: {
            IntegerExpression* i = (IntegerExpression*) exp;
            uint64_t val = strtol(i->value.value, (char **)NULL, 10);
            Value v = {TP_INT, .as.integer = val};
            i->exp.constantId = push_constant_value(c, v);
            break;
        }
        case EXP_STRLIT: {
            StringExpression* s = (StringExpression*) exp;
            Value v = {TP_STR, .as.string = s->value.value};
            s->exp.constantId = push_constant_value(c, v);
            break;
        }
        case EXP_VARREF: {
            VarRefExpression* v = (VarRefExpression*) exp;
            for (int i = 0; i < v->varName->count; i++) {
                Value val = {TP_STR, .as.string = v->varName->data[i].value};
                v->varName->data[i].constantId = push_constant_value(c, val);
            }
            break;
        }
        default:
            // Do nothing
            break;
    }
}

void push_header(Compiler* c, Statement* stmt) {
    switch (stmt->type) {
        case STMT_TYPEDEF: {
            DefineStatement* def = (DefineStatement*) stmt;
            ExpressionNode* arg = def->conditions->head;
            while (arg != NULL) {
                push_constant(c, arg->current);
                arg = arg->next;
            }
            break;
        }
        case STMT_RET: {
            ReturnStatement* ret = (ReturnStatement*) stmt;
            push_constant(c, ret->retVal);
            break;
        }
        case STMT_IF: {
            IfStatement* ifs = (IfStatement*) stmt;
            push_constant(c, ifs->cond);
            StatementNode* arg = ifs->ifBody->head;
            while (arg != NULL) {
                push_header(c, arg->current);
                arg = arg->next;
            }
            ElifNode* elifNode = ifs->elifs->head;
            while (elifNode != NULL) {
                push_constant(c, elifNode->current->cond);
                StatementNode* elifSNode = elifNode->current->body->head;
                while (elifSNode != NULL) {
                    push_header(c, elifSNode->current);
                    elifSNode = elifSNode->next;
                }
                elifNode = elifNode->next;
            }
            StatementNode* elseS = ifs->elseBody->head;
            while (elseS != NULL) {
                push_header(c, elseS->current);
                elseS = elseS->next;
            }
            break;
        }
        case STMT_VARDECL: {
            VarDeclStatement* vd = (VarDeclStatement*) stmt;
            Value v = {TP_STR, .as.string = vd->name.value};
            vd->stmt.constantId = push_constant_value(c, v);
            push_constant(c, vd->value);
            break;
        }
        case STMT_VARASS: {
            VarAssStatement* va = (VarAssStatement*) stmt;
            for (int i = 0; i < va->name->count; i++) {
                Value v = {TP_STR, .as.string = va->name->data[i].value};
                va->name->data[i].constantId = push_constant_value(c, v);
            }
            push_constant(c, va->value);
            break;
        }
        case STMT_LOOP: {
            LoopStatement* ls = (LoopStatement*) stmt;
            push_constant(c, ls->cond);
            StatementNode* arg = ls->body->head;
            while (arg != NULL) {
                push_header(c, arg->current);
                arg = arg->next;
            }
            break;
        }
        case STMT_FNCALL: {
            FnCallStatement* fc = (FnCallStatement*) stmt;
            //fc->name
            for (int i = 0; i < fc->name->count; i++) {
                Value val = {.type=TP_STR, .as.string = fc->name->data[i].value};
                fc->name->data[i].constantId = push_constant_value(c, val);
            }

            ExpressionNode* arg = fc->list->head;
            while (arg != NULL) {
                push_constant(c, arg->current);
                arg = arg->next;
            }
            break;
        }
        default: // do nothing
            break;
    }
}

void push_constants(Compiler* c, StatementNode* listHead) {
    push_byte(c, OP_HEADER_START);
    StatementNode* node = listHead;
    while (node != NULL) {
        if (node->current->type == STMT_FNDECL) {
            FnDeclStatement* stmt = (FnDeclStatement*) node->current;
            Value v = {TP_STR, .as.string = stmt->name.value};
            stmt->stmt.constantId = push_constant_value(c, v);

            ParamNode* param = stmt->parameters->head;
            while (param != NULL) {
                Param* p = param->current;
                p->type.type.constantId = push_type_header(c, p->type.type);
                if (p->type.quantifier.value != NULL) {
                    Value quantifier = {TP_STR, .as.string = p->type.quantifier.value};
                    p->type.quantifier.constantId = push_constant_value(c, quantifier);
                }
                Value name = {TP_STR, .as.string = p->name.value};
                p->name.constantId = push_constant_value(c, name);
                param = param->next;
            }

            StatementNode* line = stmt->body->head;
            while (line != NULL) {
                push_header(c, line->current);
                line = line->next;
            }
        } else if (node->current->type == STMT_OBJDECL) {
            ObjDeclStatement* od = (ObjDeclStatement*) node->current;
            Value v = { TP_STR, .as.string = od->name.value };
            od->name.constantId = push_constant_value(c, v);
            printf("Pushing constant %d\n", od->name.constantId);
            ParamNode* param = od->fields->head;
            while (param != NULL) {
                Param* p = param->current;
                p->type.type.constantId = push_type_header(c, p->type.type);
                if (p->type.quantifier.value != NULL) {
                    Value quantifier = {TP_STR, .as.string = p->type.quantifier.value};
                    p->type.quantifier.constantId = push_constant_value(c, quantifier);
                }
                Value name = {TP_STR, .as.string = p->name.value};
                p->name.constantId = push_constant_value(c, name);
                param = param->next;
            }
        } else {
            printf("Can't compile statement [%d]", node->current->type);
        }
        node = node->next;
    }
    push_short(c, OP_HEADER_END);
}

void push_declarations(Compiler* c, StatementNode* listHead) {
    push_byte(c, OP_HEADER_START);
    StatementNode* node = listHead;
    while (node != NULL) {
        if (node->current->type == STMT_FNDECL) {
            FnDeclStatement* stmt = (FnDeclStatement*) node->current;
            push_byte(c, DECL_FN);
            push_byte(c, stmt->parameters->size);
            push_short(c, stmt->stmt.constantId);
            FnInfo fn = {.constantId=stmt->stmt.constantId, .pc=c->currentPos};
            push_int(c, 0x1234);
            c->fns[stmt->stmt.constantId] = fn;
            
            ParamNode* p = stmt->parameters->head;
            while (p != NULL) {
                push_type(c, p->current->type);
                push_short(c, p->current->name.constantId);
                p = p->next;
            }
        } else if (node->current->type == STMT_OBJDECL) {
            ObjDeclStatement* stmt = (ObjDeclStatement*) node->current;
            push_byte(c, DECL_OBJ);
            push_short(c, stmt->name.constantId);
            push_byte(c, stmt->fields->size);
            ParamNode* p = stmt->fields->head;
            while (p != NULL) {
                push_type(c, p->current->type);
                push_short(c, p->current->name.constantId);
                p = p->next;
            }
        }  else {
            printf("Can't compile declaration [%d]", node->current->type);
        }
        node = node->next;
    }
    push_short(c, OP_HEADER_END);
}

void push_headers(Compiler* c, StatementNode* listHead) {
    printf("--- Compiling headers ---\n");
    push_constants(c, listHead);
    push_declarations(c, listHead);
}

void push_body_expression(Compiler* c, Expression* exp) {
    switch (exp->type) {
        case EXP_PAREN: {
            ParenExpression* p = (ParenExpression*) exp;
            push_body_expression(c, p->value);
            break;
        }
        case EXP_VARREF: {
            VarRefExpression* vr = (VarRefExpression*) exp;
            push_byte(c, OP_VAR_REF);
            push_byte(c, vr->varName->count);
            for (int i = 0; i < vr->varName->count; i++) {
                push_short(c, vr->varName->data[i].constantId);
            }
            break;
        }
        case EXP_ARRAYLIT: {
            ArrayLitExpression* ol = (ArrayLitExpression*) exp;
            ExpressionNode* arg = ol->args->head;
            while (arg != NULL) {
                push_body_expression(c, arg->current);
                arg = arg->next;
            }
            push_byte(c, OP_NEWA);
            push_short(c, ol->args->size);
            push_type(c, ol->type);
            break;
        }
        case EXP_OBJLIT: {
            ObjLitExpression* ol = (ObjLitExpression*) exp;
            ExpressionNode* arg = ol->args->head;
            while (arg != NULL) {
                push_body_expression(c, arg->current);
                arg = arg->next;
            }
            push_byte(c, OP_NEW);
            push_short(c, ol->exp.constantId);
            break;
        }
        case EXP_STRLIT:
            push_byte(c, OP_CONSTANT_STR);
            push_short(c, exp->constantId);
            break;
        case EXP_INTLIT:
            push_byte(c, OP_CONSTANT_INT);
            push_short(c, exp->constantId);
            break;
        case EXP_BOOLLIT: {
            BooleanExpression* b = (BooleanExpression*) exp;
            if (b->value)
                push_byte(c, OP_CONSTANT_BOOL_TRUE);
            else 
                push_byte(c, OP_CONSTANT_BOOL_FALSE);
            break;
        }
        case EXP_FNCALL: {
            FnCallExpression* f = (FnCallExpression*) exp;
            ExpressionNode* n = f->list->head;
            while (n != NULL) {
                push_body_expression(c, n->current);
                n = n->next;
            }
            push_body_expression(c, f->fn);
            push_byte(c, OP_FN_CALL);
            push_byte(c, f->list->size);
            break;
        }
        case EXP_UNARY: {
            UnaryExpression* be = (UnaryExpression*) exp;
            push_body_expression(c, be->r);
            switch (be->operator) {
                case T_MINUS:
                    push_byte(c, OP_NEG);
                    break;
                default:
                    printf("Unknown case %s\n", tokens[be->operator]);
                    exit(1);
            }
            break;
        }
        case EXP_ARRAYREF: {
            ArrayRefExpression* e = (ArrayRefExpression*) exp;
            push_body_expression(c, e->index);
            push_body_expression(c, e->e);
            push_byte(c, OP_GETARR);
            break;
        }
        case EXP_BINARY: {
            BinaryExpression* be = (BinaryExpression*) exp;
            push_body_expression(c, be->l);
            push_body_expression(c, be->r);
            switch (be->operator) {
                case T_MUL:
                    push_byte(c, OP_MUL);
                    break;
                case T_DIV:
                    push_byte(c, OP_DIV);
                    break;
                case T_PLUS:
                    push_byte(c, OP_ADD);
                    break;
                case T_CONCAT:
                    push_byte(c, OP_CONCAT);
                    break;
                case T_COLON:
                    push_byte(c, OP_APPEND);
                    break;
                case T_MINUS:
                    push_byte(c, OP_SUB);
                    break;
                case T_GT:
                    push_byte(c, OP_GT);
                    break;
                case T_GEQ:
                    push_byte(c, OP_LE);
                    break;
                case T_LEQ:
                    push_byte(c, OP_LE);
                    break;
                case T_LT:
                    push_byte(c, OP_LT);
                    break;
                case T_EQQ:
                    push_byte(c, OP_EQ);
                    break;
                default:
                    printf("Unknown case %s\n", tokens[be->operator]);
                    exit(1);
            }
            break;
        }
        default:
            printf("Can't compile expression [%d]\n", exp->type);
            break;
    }
}

void push_body_statement(Compiler* c, Statement* stmt) {
    switch (stmt->type) {
        case STMT_FNCALL: {
            FnCallStatement* fc = (FnCallStatement*) stmt;
            ExpressionNode* arg = fc->list->head;
            while (arg != NULL) {
                push_body_expression(c, arg->current);
                arg = arg->next;
            }

            push_byte(c, OP_VAR_REF);
            push_byte(c, fc->name->count);
            for (int i = 0; i < fc->name->count; i++) {
                push_short(c, fc->name->data[i].constantId);
            }

            push_byte(c, OP_FN_CALL);
            push_byte(c, fc->list->size);
            break;
        }
        case STMT_VARDECL: {
            VarDeclStatement* vd = (VarDeclStatement*) stmt;
            push_body_expression(c, vd->value);
            push_byte(c, OP_VAR_DECL);
            push_short(c, vd->stmt.constantId);
            break;
        }
        case STMT_VARASS: {
            VarAssStatement* vd = (VarAssStatement*) stmt;
            push_body_expression(c, vd->value);
            push_byte(c, OP_VAR_ASS);
            push_byte(c, vd->name->count);
            for (int i = 0; i < vd->name->count; i++) {
                push_short(c, vd->name->data[i].constantId);
            }
            break;
        }
        case STMT_LOOP: {
            LoopStatement* loop = (LoopStatement*) stmt;
            push_byte(c, OP_SCOPE_START);
            uint32_t loopAdd = c->currentPos;
            push_body_expression(c, loop->cond);
            push_byte(c, OP_JUMP_FALSE);
            uint32_t loopBreak = c->currentPos;
            push_int(c, 0x1234); // placeholder address
            StatementNode* body = loop->body->head;
            while (body != NULL) {
                push_body_statement(c, body->current);
                body = body->next;
            }
            push_byte(c, OP_JUMP);
            push_int(c, loopAdd);
            memcpy(&c->buffer[loopBreak], &c->currentPos, sizeof(c->currentPos));
            push_byte(c, OP_SCOPE_END);
            break;
        }
        case STMT_IF: {
            IfStatement* ifStmt = (IfStatement*) stmt;
            push_body_expression(c, ifStmt->cond);
            push_byte(c, OP_SCOPE_START);
            push_byte(c, OP_JUMP_FALSE);
            uint32_t labelIf = c->currentPos;
            push_int(c, 0x1234); // placeholder address
            StatementNode* ifB = ifStmt->ifBody->head;
            while (ifB != NULL) {
                push_body_statement(c, ifB->current);
                ifB = ifB->next;
            }
            push_byte(c, OP_SCOPE_END);
            push_byte(c, OP_JUMP);
            uint32_t labelElse = c->currentPos;
            push_int(c, 0x1234); // placeholder address

            // write actual address to if-jmp
            memcpy(&c->buffer[labelIf], &c->currentPos, sizeof(c->currentPos));

            StatementNode* elseB = ifStmt->elseBody->head;
            push_byte(c, OP_SCOPE_START);
            while (elseB != NULL) {
                push_body_statement(c, elseB->current);
                elseB = elseB->next;
            }
            push_byte(c, OP_SCOPE_END);
            // write actual address to else-jmp
            memcpy(&c->buffer[labelElse], &c->currentPos, sizeof(c->currentPos));
            break;
        }
        case STMT_RET: {
            ReturnStatement* ret = (ReturnStatement*) stmt;
            push_body_expression(c, ret->retVal);
            push_byte(c, OP_RETURN);
            break;
        }
        default:
            printf("Can't compile statement [%d]", stmt->type);
            break;
    }
}

void push_body(Compiler* c, StatementNode* listHead) {
    printf("--- Compiling body ---\n");
    StatementNode* node = listHead;
    while (node != NULL) {
        if (node->current->type == STMT_FNDECL) {
            FnDeclStatement* stmt = (FnDeclStatement*) node->current;
            push_byte(c, OP_FN_DECL);
            push_short(c, stmt->stmt.constantId);
            push_byte(c, stmt->parameters->size);

            // write actual address of the fn to the header!
            uint32_t amend = c->fns[stmt->stmt.constantId].pc;
            //printf("amending %x\n", amend);
            memcpy(&c->buffer[amend], &c->currentPos, sizeof(c->currentPos));

            StatementNode* line = stmt->body->head;
            while (line != NULL) {
                push_body_statement(c, line->current);
                line = line->next;
            }
        } else if (node->current->type == STMT_OBJDECL) {
            // do nothing
        } else {
            printf("Can't compile statement [%d]\n", node->current->type);
        }
        node = node->next;
    }
}

void compile(Compiler* c, Parser* p) {
    printf("--- Compiling to %s ---", c->path);
    StatementList* list = parse(p);
    // PUSH_HEADER
    push_headers(c, list->head);
    // PUSH_BODY
    push_body(c, list->head);

    // WRITE FILE
    fwrite(c->buffer, 1, c->currentPos, c->file);
}
