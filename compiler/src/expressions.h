#include "parser.h"
#include "lexer.h"
#include "../../common/src/linked_list.h"
#include "../../common/src/dynamic_array.h"
#include "common.h"

typedef struct {
    Expression* cond;
    StatementList* body;
} Elif;

DEFINE_LINKED_LIST(Elif);
DEFINE_DYNAMIC_ARRAY(Identifier);
DEFINE_DYNAMIC_ARRAY(Type);

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
    Expression* e;
    Expression* index;
} ArrayRefExpression;

typedef struct {
    Expression exp;
    IdentifierArray* varName;
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
    bool value;
} BooleanExpression;

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
    Identifier name;
    ExpressionList* args;
} ObjLitExpression;

typedef struct {
    Expression exp;
    Type type;
    ExpressionList* args;
} ArrayLitExpression;

typedef struct {
    Expression exp;
    ExpressionList* vals;
} TupleExpression;

typedef struct {
    Expression exp;
    Expression* l;
    Expression* r;
    Token operator;
} BinaryExpression;

typedef struct {
    Expression exp;
    Expression* value;
} MaybeExpression;

typedef struct {
    Statement stmt;
    Expression* retVal;
} ReturnStatement;

typedef struct {
    Statement stmt;
    Expression* cond;
    StatementList* body;
} LoopStatement;

typedef struct {
    Statement stmt;
    Identifier name;
    ParamList* parameters;
    StatementList* body;
    TypeArray* ret;
} FnDeclStatement;

typedef struct {
    Statement stmt;
    Identifier name;
    Type tp;
    Expression* value;
} VarDeclStatement;

typedef struct {
    Statement stmt;
    IdentifierArray* name;
    ExpressionList* list;
} FnCallStatement;

typedef struct {
    Statement stmt;
    IdentifierArray* name;
    Expression* value;
} VarAssStatement;

typedef struct {
    Statement stmt;
    Expression* cond;
    StatementList* ifBody;
    ElifList* elifs;
    StatementList* elseBody;
} IfStatement;

typedef struct {
    Statement stmt;
    Identifier typeName;
    Identifier objectName;
    Identifier varName;
    ExpressionList* conditions;
} DefineStatement;

void print_statement(Statement* stmt);
void print_expression(Expression* exp);
Identifier read_identifier(Parser* p);
IdentifierArray* read_identifier_list(Parser* p);
Type read_type(Parser* p);

Expression* make_none(Parser* p);
Expression* make_just(Parser* p);
Expression* make_allocation_lit(Parser* p);
Expression* make_string_exp(Parser* p);
Expression* make_paren(Parser* p);
Expression* make_integer(Parser* p);
Expression* make_float(Parser* p);
Expression* make_var_ref(Parser* p);
Expression* make_binary(Parser* p);
Expression* make_unary(Parser* p);
Expression* make_fn_call_exp(Parser* p);
Expression* make_array_ref_exp(Parser* p);
Expression* make_bool_exp(Parser* p, bool b);
Expression* make_tuple_exp(Parser* p);

Statement* make_typedef(Parser* p);
Statement* make_return(Parser* p);
Statement* make_if(Parser* p);
Statement* make_loop(Parser* p);
Statement* make_obj_decl(Parser* p);
Statement* make_fn_decl(Parser* p);
Statement* make_variable_decl(Parser* p);
Statement* make_field_assign(Parser* p, IdentifierArray* name);
Statement* make_variable_assign(Parser* p, IdentifierArray* name);
Statement* make_fn_call_stmt(Parser* p, IdentifierArray* name);