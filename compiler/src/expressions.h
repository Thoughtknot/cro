#include "parser.h"
#include "lexer.h"
#include "../../common/src/linked_list.h"
#include "../../common/src/dynamic_array.h"
#include "common.h"

typedef struct {
    Statement stmt;
    Identifier name;
    ParamList* parameters;
    StatementList* body;
    Type ret;
} FnDeclStatement;

typedef struct {
    Statement stmt;
    Identifier name;
    Expression* value;
} VarDeclStatement;

typedef struct {
    Statement stmt;
    Identifier name;
    ExpressionList* list;
} FnCallStatement;

typedef struct {
    Statement stmt;
    Identifier name;
    Expression* value;
} VarAssStatement;

typedef struct {
    Statement stmt;
    Identifier name;
    ParamList* fields;
} ObjDeclStatement;

typedef struct {
    Expression exp;
    Expression* fn;
    ExpressionList* list;
} FnCallExpression;

typedef struct {
    Expression exp;
    Expression* obj;
    Identifier name;
} FieldRefExpression;

typedef struct {
    Expression exp;
    Identifier varName;
} VarRefExpression;

typedef struct {
    Expression exp;
    Identifier value;
} StringExpression;

typedef struct {
    Expression exp;
    Identifier value;
} IntegerExpression;

typedef struct {
    Expression exp;
    Identifier value;
} FloatExpression;

typedef struct {
    Expression exp;
    Expression* r;
    Token operator;
} UnaryExpression;

typedef struct {
    Expression exp;
    Expression* value;
} ParenExpression;

typedef struct {
    Expression exp;
    Expression* l;
    Expression* r;
    Token operator;
} BinaryExpression;

void print_statement(Statement* stmt);
void print_expression(Expression* exp);
Identifier read_identifier(Parser* p);
Type read_type(Parser* p);

Expression* make_string(Parser* p);
Expression* make_paren(Parser* p);
Expression* make_integer(Parser* p);
Expression* make_float(Parser* p);
Expression* make_var_ref(Parser* p);
Expression* make_binary(Parser* p);
Expression* make_unary(Parser* p);
Expression* make_field_ref_exp(Parser* p);
Expression* make_fn_call_exp(Parser* p);

Statement* make_obj_decl(Parser* p);
Statement* make_fn_decl(Parser* p);
Statement* make_variable_decl(Parser* p);
Statement* make_variable_assign(Parser* p, Identifier name);
Statement* make_fn_call_stmt(Parser* p, Identifier name);