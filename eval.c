#include <string.h>
#include "error.h"
#include "datatype.h"
#include "eval.h"
#define INIT_ENV_SIZE 4

// enum Type
// {
//     LIST,
//     SYMBOL,
//     CHAR,
//     NUMBER,
//     STRING,
//     BOOLEAN,
//     EXPRESSION
// };
// 
// struct Value
// {
//     enum Type type;
//     union
//     {
//         struct List *list;
//         char *symbol;
//         char character;
//         double number;
//         struct List *string;
//         bool boolean;
//         struct List *expression;
//     };
// };
// 
// struct List 
// {
//     unsigned int size;
//     unsigned int capacity;
//     struct Value **values;
// };

// struct Variable
// {
//     char *name;
//     struct Value *value;
// };
// 
// struct VariableList
// {
//     struct Variable *var;
//     struct VariableList *next;
// };
// 
// struct Environment
// {
//     struct VariableList *varlist;
//     struct Environment *parent;
// };

void init_env(struct Environment *env, struct Environment *parent)
{
    env->varlist = NULL;
    env->parent = parent;
}

void init_varlist(struct VariableList *varlist)
{
    varlist->var = NULL;
    varlist->next = NULL;
}

// struct Value *list_lookup(struct VariableList
struct Value *eval(struct Environment *env, enum Error *error, struct Value *val);

struct Value *lookup(struct Environment *env, char *name)
{
    // for (struct Variable *var = env->
    return NULL;
}

void define(struct VariableList *varlist, struct Variable *var)
{
    if (varlist->var == NULL)
    {
        varlist->var = var;
    }
    else
    {
        struct VariableList *new_varlist = malloc(sizeof(*new_varlist));
        init_varlist(new_varlist);
        varlist->var = var;
        varlist->next = new_varlist;
    }
}

struct Value *eval_symbol(struct Environment *env, enum Error *error, char *symbol)
{
    struct Value *someval = lookup(env, symbol);
    if (someval == NULL)
    {
        // *error = NO_ERROR;
        return NULL;
    }
    return eval(env, error, someval);
}

struct Value *eval_list(struct Environment *env, enum Error *error, struct List *lst)
{
    if (is_empty(lst))
    {
        return NULL;
    }
    struct Value *first = list_lookup(lst, 0);
    if (first != NULL)
    {
        // If first is not a symbol, return error
        if (first->type != SYMBOL)
        {
            *error = FIRST_NOT_SYMBOL;
            return NULL;
        }
        else if (strcmp(first->symbol, "define") == 0)
        {
            // handle define
        }
        else if (strcmp(first->symbol, "lambda") == 0)
        {
            // handle lambda
        }
        else if (strcmp(first->symbol, "if") == 0)
        {
            // handle if
        }
        else
        {
            *error = UNDEFINED_VARIABLE;
            return NULL;
        }
    }
    return NULL;
}

struct Value *eval(struct Environment *env, enum Error *error, struct Value *val)
{
    switch (val->type)
    {
        case LIST:
            return eval_list(env, error, val->list);
        case SYMBOL:
            return eval_symbol(env, error, val->symbol);
        case EXPRESSION:
            return NULL;
        //     return eval(env, error, val->expression);
        case CHAR:
        case NUMBER:
        case STRING:
        case BOOLEAN:
            return val;
    }
}
