#include <stdio.h>
#include <string.h>
#include <float.h>
#include <assert.h>
#include "eval.h"
#include "error.h"
#include "parser.h"
#include "datatype.h"
#include "namespace.h"

/* Does boilerplate error checking for when we expect eval to return a value */
struct Value *checked_eval(struct Namespace *nsp, struct Parser *parser, struct Value *val)
{
    struct Value *eval_result = eval(nsp, parser, val);
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
        struct Namespace *nsp, struct Parser *parser, 
        struct Value *val, enum Type t, enum Error err)
{
    struct Value *eval_result = checked_eval(nsp, parser, val);
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

// In Scheme all values are truthy except for #f
struct Value *to_bool(struct Namespace *nsp, struct Parser *parser, struct Value *v)
{
    struct Value *v_bool = checked_eval(nsp, parser, v);
    if (v_bool == NULL) 
    {
        return NULL;
    }
    else if (v_bool->type == BOOLEAN && v_bool->boolean == false)
    {
        return vboolean(false);
    }
    else
    {
        return vboolean(true);
    }
}

struct Value *eval_symbol(struct Namespace *nsp, struct Parser *parser, char *symbol)
{
    struct Value *val = lookup_var(nsp, symbol);
    if (val == NULL) parser->error = SYMBOL_NOT_BOUND;
    return val;
}

void eval_define(struct Namespace *nsp, struct Parser *parser, struct List *lst)
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

    // Define as null *first* so we can recursively call if need be
    define(nsp, name, NULL);

    struct Value *val = checked_eval(nsp, parser, list_lookup(lst, 2));
    if (val == NULL) return;

    // Define with actual value
    define(nsp, name, val);
}

struct Value *eval_add(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    struct Value *val;
    double sum = 0;
    for (unsigned int i = 1; i < lst->size; i++)
    {
        val = list_lookup(lst, i);
        val = checked_typed_eval(nsp, parser, val, NUMBER, EXPECTED_NUMBER);
        if (val == NULL) return NULL;
        sum += val->number;
    }
    return vnumber(sum);
}

struct Value *eval_subtract(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    struct Value *val;
    if (lst->size == 1)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    val = list_lookup(lst, 1);
    val = checked_typed_eval(nsp, parser, val, NUMBER, EXPECTED_NUMBER);
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
        val = checked_typed_eval(nsp, parser, val, NUMBER, EXPECTED_NUMBER);
        sum -= val->number;
    }
    return vnumber(sum);
}

struct Value *eval_if(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    if (lst->size < 3 || lst->size > 4)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }

    struct Value *val;
    val = to_bool(nsp, parser, list_lookup(lst, 1));

    if (val == NULL) 
    {
        return NULL;
    }
    else if (val->boolean == true)
    {
        return eval(nsp, parser, list_lookup(lst, 2));
    }
    else if (lst->size == 4)
    {
        return eval(nsp, parser, list_lookup(lst, 3));
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
    for (unsigned int i = 0; i < args->list->size; i++)
    {
        if (args->list->values[i]->type != SYMBOL)
        {
            parser->error = EXPECTED_SYMBOL;
            return NULL;
        }
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

struct Value *eval_eq_op(struct Namespace *nsp, struct Parser *parser, struct List *lst, enum CompareOp op)
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
        v = checked_typed_eval(nsp, parser, lst->values[i], NUMBER, EXPECTED_NUMBER);
        if (v == NULL) return NULL;
        if ((op == EQ && i != 1 && !(prev == v->number)) ||
                (op == LEQ && !(prev <= v->number)) ||
                (op == GEQ && !(prev >= v->number)) ||
                (op == LESS && !(prev < v->number)) ||
                (op == GREATER && !(prev > v->number)))
        {
            return vboolean(false);
        }
        prev = v->number;
    }
    return vboolean(true);
}

struct Value *eval_not(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    if (lst->size != 2)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    struct Value *v = to_bool(nsp, parser, lst->values[1]);
    if (v == NULL) 
    {
        return NULL;
    }
    else if (v->boolean == false)
    {
        return vboolean(true);
    }
    else
    {
        return vboolean(false);
    }
}

struct Value *eval_bin_bool(struct Namespace *nsp, struct Parser *parser, struct List *lst, bool init)
{
    bool is_and, is_or;
    is_and = init; // if it's "and", it inits to true
    is_or = !init; // if it's "or", it inits to false

    struct Value *v;
    for (unsigned int i = 1; i < lst->size; i++)
    {
        v = to_bool(nsp, parser, lst->values[i]);
        if (v == NULL) return NULL;

        if ((v->boolean == true) && is_or)
        {
            return vboolean(true);
        } 
        else if ((v->boolean == false) && is_and)
        {
            return vboolean(false);
        }
    }
    return vboolean(init);
}

struct Value *eval_quote(struct Parser *parser, struct List *lst)
{
    if (lst->size != 2)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    return lst->values[1];
}

struct Value *eval_proc(struct Namespace *nsp, struct Parser *parser, struct List *lst, struct Value *proc)
{
    struct List *args;
    struct Value *body;
    args = get_args(proc);
    body = get_body(proc);

    // Must pass an value for each argument
    if (lst->size - 1 != args->size)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }

    // Create a namespace for this scope
    struct Namespace *child_nsp = new_nsp(nsp);

    // Evaluate each argument and bind it to the symbol given
    struct Value *arg;
    for (unsigned int i = 0; i < args->size; i++)
    {
        arg = checked_eval(nsp, parser, lst->values[i+1]);
        if (arg == NULL) return NULL;
        define(child_nsp, args->values[i], arg);
    }
    return eval(child_nsp, parser, body);
}

// TODO not sure if it's here or somewhere else, 
// but can't define something within a "begin" without it returning undefined
struct Value *eval_begin(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    struct Value *v = NULL;
    for (unsigned int i = 1; i < lst->size; i++)
    {
        v = checked_eval(nsp, parser, lst->values[i]);
        if (v == NULL) return NULL;
    }
    return v;
}

// Whoa, meta
struct Value *eval_eval(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    if (lst->size != 2)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    struct Value *v = checked_eval(nsp, parser, lst->values[1]);
    return eval(nsp, parser, v);
}


struct Value *eval_list(struct Namespace *nsp, struct Parser *parser, struct List *lst)
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
    else if (first->type == LIST)
    {
        struct Value *proc = checked_typed_eval(nsp, parser, first, PROCEDURE, EXPECTED_PROC);
        if (proc == NULL) return NULL;
        return eval_proc(nsp, parser, lst, proc);
    }
    else if (first->type != SYMBOL)
    {
        parser->error = FIRST_NOT_PROC;
        return NULL;
    }
    else if (match("define"))
    {
        eval_define(nsp, parser, lst);
        return NULL;
    }
    else if (match("+"))
    {
        return eval_add(nsp, parser, lst);
    }
    else if (match("-"))
    {
        return eval_subtract(nsp, parser, lst);
    }
    else if (match(">"))
    {
        return eval_eq_op(nsp, parser, lst, GREATER);
    }
    else if (match("<"))
    {
        return eval_eq_op(nsp, parser, lst, LESS);
    }
    else if (match("="))
    {
        return eval_eq_op(nsp, parser, lst, EQ);
    }
    else if (match(">="))
    {
        return eval_eq_op(nsp, parser, lst, GEQ);
    }
    else if (match("<="))
    {
        return eval_eq_op(nsp, parser, lst, LEQ);
    }
    else if (match("not"))
    {
        return eval_not(nsp, parser, lst);
    }
    else if (match("and"))
    {
        return eval_bin_bool(nsp, parser, lst, true);
    }
    else if (match("or"))
    {
        return eval_bin_bool(nsp, parser, lst, false);
    }
    else if (match("lambda"))
    {
        return eval_lambda(parser, lst);
    }
    else if (match("if"))
    {
        return eval_if(nsp, parser, lst);
    }
    else if (match("quote"))
    {
        return eval_quote(parser, lst);
    }
    else if (match("begin"))
    {
        return eval_begin(nsp, parser, lst);
    }
    else if (match("eval"))
    {
        return eval_eval(nsp, parser, lst);
    }
    else
    {
        struct Value *proc = checked_typed_eval(nsp, parser, first, PROCEDURE, EXPECTED_PROC);
        if (proc == NULL) return NULL;
        return eval_proc(nsp, parser, lst, proc);
    }
#undef match
}

struct Value *eval(struct Namespace *nsp, struct Parser *parser, struct Value *val)
{
    if (val == NULL) return NULL;
    switch (val->type)
    {
        case LIST:
            return eval_list(nsp, parser, val->list);
        case SYMBOL:
            return eval_symbol(nsp, parser, val->symbol);
        case PROCEDURE:
            printf("I don't think we'll actually ever use this?\n");
            return NULL;
        case CHAR:
        case NUMBER:
        case STRING:
        case BOOLEAN:
            return val;
    }
}
