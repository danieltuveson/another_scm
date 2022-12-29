#include <stdio.h>
#include <string.h>
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

void push_var(Bindings *bindings, Binding *bind)
{
    append(bindings, bind);
}

// TODO: need to replace existing variable with name of symbol if it exists in this scope
void define(struct Environment *env, struct Value *symbol, struct Value *val)
{
    Binding *bind = malloc(sizeof(*bind));
    new_var(bind, symbol, val);
    append(env->bindings, bind);
}

static inline char *get_name(Binding *b)
{
    return b->list->values[0]->symbol;
}

static inline struct Value *get_value(Binding *b)
{
    return b->list->values[1];
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
    struct Value *val = eval(env, parser, list_lookup(lst, 2));
    if (parser->error != NO_ERROR)
    {
        return;
    }
    define(env, name, val);
}

struct Value *eval_add(struct Environment *env, struct Parser *parser, struct List *lst)
{
    struct Value *val;
    double sum = 0;
    for (unsigned int i = 1; i < lst->size; i++)
    {
        val = eval(env, parser, list_lookup(lst, i));
        if (val->type != NUMBER)
        {
            parser->error = EXPECTED_NUMBER;
            return NULL;
        }
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
    val = eval(env, parser, list_lookup(lst, 1));
    if (val->type != NUMBER)
    {
        parser->error = EXPECTED_NUMBER;
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
        val = eval(env, parser, list_lookup(lst, i));
        if (val->type != NUMBER)
        {
            parser->error = EXPECTED_NUMBER;
            return NULL;
        }
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
    val = eval(env, parser, list_lookup(lst, 1));
    if (val->type != BOOLEAN)
    {
        parser->error = EXPECTED_BOOLEAN;
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

struct Value *eval_list(struct Environment *env, struct Parser *parser, struct List *lst)
{
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
        parser->error = FIRST_NOT_SYMBOL;
        return NULL;
    }
    else if (strcmp(first->symbol, "define") == 0)
    {
        eval_define(env, parser, lst);
        return NULL;
    }
    else if (strcmp(first->symbol, "+") == 0)
    {
        return eval_add(env, parser, lst);
    }
    else if (strcmp(first->symbol, "-") == 0)
    {
        return eval_subtract(env, parser, lst);
    }
    else if (strcmp(first->symbol, "lambda") == 0)
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
        return NULL;
    }
    else if (strcmp(first->symbol, "if") == 0)
    {
        return eval_if(env, parser, lst);
    }
    else
    {
        parser->error = UNDEFINED_VARIABLE;
        return NULL;
    }
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
