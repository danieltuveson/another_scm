#include <stdio.h>
#include <string.h>
#include <float.h>
#include <assert.h>
#include "error.h"
#include "parser.h"
#include "datatype.h"
#include "eval.h"

// Probably fine if val is NULL, but symbol must be a symbol
void new_var(Binding *bind, struct Value *symbol, struct Value *val)
{
    assert(symbol->type == SYMBOL);

    bind->type = LIST;
    bind->list = list();

    append(bind->list, symbol);
    append(bind->list, val);
}

void init_env(struct Environment *env, struct Environment *parent)
{
    env->bindings = list();
    env->parent = parent;
}

void new_env(struct Environment *parent)
{
    struct Environment *env = malloc(sizeof(*env));
    init_env(env, parent);
}

/* Does boilerplate error checking for when we expect eval to return a value */
struct Value *checked_eval(struct Environment *env, struct Parser *parser, struct Value *val)
{
    struct Value *eval_result = eval(env, parser, val);
    if (parser->error != NO_ERROR)
    {
        return NULL;
    }
    else if (eval_result == NULL)
    {
        parser->error = UNDEFINED;
        return NULL;
    }
    return eval_result;
}
struct Value *checked_typed_eval(
        struct Environment *env, struct Parser *parser, 
        struct Value *val, enum Type t, enum Error err)
{
    struct Value *eval_result = checked_eval(env, parser, val);
    if (eval_result == NULL)
    {
        return NULL;
    }
    else if (eval_result->type != t)
    {
        parser->error = err;
        return NULL;
    }
    return eval_result;
}

// TODO: need to replace existing variable with name of symbol if it exists in this scope
void define(struct Environment *env, struct Value *symbol, struct Value *val)
{
    Binding *bind = malloc(sizeof(*bind));
    new_var(bind, symbol, val);
    append(env->bindings, bind);
}

// TODO: Update this to look at parent environment
// This function looks for variable with the given name
struct Value *lookup_var(struct Environment *env, char *lname)
{
    if (is_empty(env->bindings))
    {
        if (env->parent == NULL)
        {
            return NULL;
        }
        else
        {
            return lookup_var(env->parent, lname);
        }
    }
    char *name;
    Bindings *binds = env->bindings;

    for (unsigned int i = 0; i < binds->size; i++)
    {
        // Found variable with this name
        name = get_name(binds->values[i]);
        if (strcmp(name, lname) == 0)
        {
            return get_value(binds->values[i]);
        }
    }
    return NULL;
}

// TODO: Implement
void add_scope(Bindings *bindings, Binding *bind)
{
}

struct Value *eval_symbol(struct Environment *env, struct Parser *parser, char *symbol)
{
    struct Value *val = lookup_var(env, symbol);
    if (val == NULL) parser->error = SYMBOL_NOT_BOUND;
    return val;
}

void eval_define(struct Environment *env, struct Parser *parser, struct List *lst)
{
    if (lst->size != 3)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return;
    }
    struct Value *name = list_lookup(lst, 1);
    if (name->type != SYMBOL)
    {
        parser->error = EXPECTED_SYMBOL;
        return;
    }
    struct Value *val = checked_eval(env, parser, list_lookup(lst, 2));
    if (val == NULL) return;

    define(env, name, val);
}

struct Value *eval_add(struct Environment *env, struct Parser *parser, struct List *lst)
{
    struct Value *val;
    double sum = 0;
    for (unsigned int i = 1; i < lst->size; i++)
    {
        val = list_lookup(lst, i);
        val = checked_typed_eval(env, parser, val, NUMBER, EXPECTED_NUMBER);
        if (val == NULL) return NULL;
        sum += val->number;
    }
    return vnumber(sum);
}

struct Value *eval_subtract(struct Environment *env, struct Parser *parser, struct List *lst)
{
    struct Value *val;
    if (lst->size == 1)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    val = list_lookup(lst, 1);
    val = checked_typed_eval(env, parser, val, NUMBER, EXPECTED_NUMBER);
    if (val == NULL) 
    {
        return NULL;
    }
    else if (lst->size == 2)
    {
        // if only one value passed to '-', then it's a unary '-'
        return vnumber((-1) * val->number);
    }
    double sum = val->number;
    for (unsigned int i = 2; i < lst->size; i++)
    {
        val = list_lookup(lst, i);
        val = checked_typed_eval(env, parser, val, NUMBER, EXPECTED_NUMBER);
        sum -= val->number;
    }
    return vnumber(sum);
}

struct Value *eval_if(struct Environment *env, struct Parser *parser, struct List *lst)
{
    if (lst->size < 3 || lst->size > 4)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }

    struct Value *val;
    val = list_lookup(lst, 1);
    val = checked_typed_eval(env, parser, val, BOOLEAN, EXPECTED_BOOLEAN);
    if (val == NULL) 
    {
        return NULL;
    }
    else if (val->boolean == true)
    {
        return eval(env, parser, list_lookup(lst, 2));
    }
    else if (lst->size == 4)
    {
        return eval(env, parser, list_lookup(lst, 3));
    }
    else
    {
        return NULL;
    }
}

struct Value *eval_lambda(struct Parser *parser, struct List *lst)
{
    if (lst->size != 3)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    struct Value *args, *body;
    args = list_lookup(lst, 1);
    body = list_lookup(lst, 2);
    if (args->type != LIST)
    {
        parser->error = EXPECTED_LIST;
        return NULL;
    }
    return vproc(args, body);
}

enum CompareOp 
{
    EQ,
    LEQ,
    GEQ,
    LESS,
    GREATER,
};

struct Value *eval_eq_op(struct Environment *env, struct Parser *parser, struct List *lst, enum CompareOp op)
{
    if (lst->size == 1)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    struct Value *v;
    double prev = (op == GEQ || op == GREATER) ? DBL_MAX : -DBL_MAX;

    for (unsigned int i = 1; i < lst->size; i++)
    {
        v = checked_typed_eval(env, parser, lst->values[i], NUMBER, EXPECTED_NUMBER);
        if (v == NULL) return NULL;
        if ((op == EQ && i != 1 && !(prev == v->number)) ||
                (op == LEQ && !(prev <= v->number)) ||
                (op == GEQ && !(prev >= v->number)) ||
                (op == LESS && !(prev < v->number)) ||
                (op == GREATER && !(prev > v->number)))
        {
            return vboolean(0);
        }
        prev = v->number;
    }
    return vboolean(1);
}

struct Value *eval_list(struct Environment *env, struct Parser *parser, struct List *lst)
{
#define match(name) (strcmp(first->symbol, (name)) == 0)
    if (is_empty(lst))
    {
        // should throw error or something
        return NULL;
    }
    struct Value *first = list_lookup(lst, 0);
    if (first == NULL)
    {
        return NULL;
    }
    else if (first->type != SYMBOL)
    {
        parser->error = FIRST_NOT_PROC;
        return NULL;
    }
    else if (match("define"))
    {
        eval_define(env, parser, lst);
        return NULL;
    }
    else if (match("+"))
    {
        return eval_add(env, parser, lst);
    }
    else if (match("-"))
    {
        return eval_subtract(env, parser, lst);
    }
    else if (match(">"))
    {
        return eval_eq_op(env, parser, lst, GREATER);
    }
    else if (match("<"))
    {
        return eval_eq_op(env, parser, lst, LESS);
    }
    else if (match("="))
    {
        return eval_eq_op(env, parser, lst, EQ);
    }
    else if (match(">="))
    {
        return eval_eq_op(env, parser, lst, GEQ);
    }
    else if (match("<="))
    {
        return eval_eq_op(env, parser, lst, LEQ);
    }
    else if (match("not"))
    {
    }
    else if (match("and"))
    {
    }
    else if (match("or"))
    {
    }
    else if (match("lambda"))
    {
        return eval_lambda(parser, lst);
    }
    else if (match("if"))
    {
        return eval_if(env, parser, lst);
    }
    else
    {
        parser->error = UNDEFINED_VARIABLE;
        return NULL;
    }
#undef match
}

struct Value *eval(struct Environment *env, struct Parser *parser, struct Value *val)
{
    switch (val->type)
    {
        case LIST:
            return eval_list(env, parser, val->list);
        case SYMBOL:
            return eval_symbol(env, parser, val->symbol);
        case PROCEDURE:
            printf("I don't think we'll actually ever use this?\n");
            return NULL;
        //     return eval(env, parser, val->expression);
        case CHAR:
        case NUMBER:
        case STRING:
        case BOOLEAN:
            return val;
    }
}
