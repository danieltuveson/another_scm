#include <stdio.h>
#include <string.h>
#include <float.h>
#include <assert.h>
#include "eval.h"
#include "error.h"
#include "parser.h"
#include "datatype.h"
#include "namespace.h"
#include "file.h"
#include "print.h"

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
        parser->error = EXPECTED_LIST_OR_SYMBOL;
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

struct Value *eval_eq(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    if (lst->size != 3) 
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    struct Value *arg1, *arg2;
    arg1 = checked_eval(nsp, parser, list_lookup(lst, 1));
    if (arg1 == NULL) return NULL;

    arg2 = checked_eval(nsp, parser, list_lookup(lst, 2));
    if (arg2 == NULL) return NULL;

    // If types not equal, know it's not eq
    if (arg1->type != arg2->type)
    {
        return vboolean(false);
    }
    // Basing this on the behavior of Chez Scheme
    switch (arg1->type)
    {
        case CHAR:
            return vboolean(arg1->character == arg2->character);
        // Numbers are just doubles so won't match other Schemes like Chez
        case NUMBER:
            return vboolean(arg1->number == arg2->number);
        case STRING:
            return vboolean(arg1->string == arg2->string);
        case BOOLEAN:
            return vboolean(arg1->boolean == arg2->boolean);
        case PROCEDURE:
            return vboolean(arg1->proc == arg2->proc);
        case LIST:
            if (is_empty(arg1->list) && is_empty(arg2->list))
            {
                return vboolean(true);
            }
            else
            {
                return vboolean(arg1->list == arg2->list);
            }
        case SYMBOL:
            return vboolean(strcmp(arg1->symbol, arg2->symbol) == 0);
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
    if (args->type == LIST)
    {
        for (unsigned int i = 0; i < args->list->size; i++)
        {
            if (list_lookup(args->list, i)->type != SYMBOL)
            {
                parser->error = EXPECTED_SYMBOL;
                return NULL;
            }
        }
        for (unsigned int i = 0; i < args->list->size; i++)
        {
            for (unsigned int j = i + 1; j < args->list->size; j++)
            {
                // if (args->list->value[i]->symbol args->list
            }
        }
        return vproc(args, body);
    }
    else if (args->type == SYMBOL)
    {
        return vproc(args, body);
    }
    else
    {
        parser->error = EXPECTED_LIST_OR_SYMBOL;
        return NULL;
    }
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
        v = checked_typed_eval(nsp, parser, list_lookup(lst, i), NUMBER, EXPECTED_NUMBER);
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

struct Value *eval_bin_bool(struct Namespace *nsp, struct Parser *parser, struct List *lst, bool init)
{
    bool is_and, is_or;
    is_and = init; // if it's "and", it inits to true
    is_or = !init; // if it's "or", it inits to false

    struct Value *v;
    for (unsigned int i = 1; i < lst->size; i++)
    {
        v = to_bool(nsp, parser, list_lookup(lst, i));
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
    return list_lookup(lst, 1);
}

struct Value *eval_proc(struct Namespace *nsp, struct Parser *parser, struct List *lst, struct Value *proc)
{
    struct Value *args;
    struct Value *body;
    args = get_args(proc);
    body = get_body(proc);

    // Must pass an value for each argument
    if ((args->type == LIST) && (lst->size - 1 != args->list->size))
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }

    // Create a namespace for this scope
    struct Namespace *child_nsp = new_nsp(nsp);

    // Evaluate each argument and bind it to the symbol given
    struct Value *arg;

    if (args->type == LIST)
    {
        for (unsigned int i = 0; i < args->list->size; i++)
        {
            arg = checked_eval(nsp, parser, list_lookup(lst, i+1));
            if (arg == NULL) return NULL;
            define(child_nsp, list_lookup(args->list, i), arg);
        }
    }
    else if (args->type == SYMBOL)
    {
        struct List *lst_args = list();
        if (lst_args == NULL) return NULL;
        for (unsigned int i = 1; i < lst->size; i++)
        {
            arg = checked_eval(nsp, parser, list_lookup(lst, i));
            if (arg == NULL) return NULL;
            append(lst_args, arg);
        }
        define(child_nsp, args, vlist(lst_args));
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
        v = eval(nsp, parser, list_lookup(lst, i));

        if (parser->error != NO_ERROR) return NULL;
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
    struct Value *v = checked_eval(nsp, parser, list_lookup(lst, 1));
    return eval(nsp, parser, v);
}

struct Value *eval_car(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    if (lst->size != 2)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    struct Value *v = checked_typed_eval(nsp, parser, list_lookup(lst, 1), LIST, EXPECTED_PAIR);
    if (v == NULL) 
    {
        return NULL;
    }
    else if (is_empty(v->list))
    {
        parser->error = EXPECTED_PAIR;
        return NULL;
    }
    return copy_value(list_lookup(v->list, 0));
}

struct Value *eval_cdr(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    struct Value *v, *copy;
    if (lst->size != 2)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    v = checked_typed_eval(nsp, parser, list_lookup(lst, 1), LIST, EXPECTED_PAIR);
    if (v == NULL) 
    {
        return NULL;
    }
    else if (is_empty(v->list))
    {
        parser->error = EXPECTED_PAIR;
        return NULL;
    }
    struct List *lst_copy = list();
    if (lst_copy == NULL) return NULL;
    for (unsigned int i = 1; i < v->list->size; i++)
    {
        copy = copy_value(list_lookup(v->list, i));
        if (copy == NULL) 
        {
            delete_list(lst_copy);
            return NULL;
        }
        append(lst_copy, copy);
    }
    return vlist(lst_copy);
}

struct Value *eval_cons(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    struct Value *car, *cdr;
    if (lst->size != 3)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    car = checked_eval(nsp, parser, list_lookup(lst, 1));
    if (car == NULL) return NULL;
    cdr = checked_typed_eval(nsp, parser, list_lookup(lst, 2), LIST, EXPECTED_LIST);
    if (cdr == NULL) return NULL;
    struct List *l;
    l = list();
    append(l, car);
    for (unsigned int i = 0; i < cdr->list->size; i++) 
    {
        append(l, list_lookup(cdr->list, i));
    }
    return vlist(l);
}

void eval_set(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    struct Value *name, *val;
    if (lst->size != 3)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return;
    }
    name = list_lookup(lst, 1);
    if (name->type != SYMBOL)
    {
        parser->error = EXPECTED_SYMBOL;
        return;
    }
    val = checked_eval(nsp, parser, list_lookup(lst, 2));
    if (val == NULL) return;

    define(nsp, name, val);
}

struct Value *eval_load(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    struct Value *v;
    if (lst->size != 2)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    v = checked_typed_eval(nsp, parser, list_lookup(lst, 1), STRING, EXPECTED_STRING);
    if (v == NULL)
    {
        return NULL;
    }
    ScmString *sstr = read_file(from_scm_string(v->string));
    if (sstr == NULL)
    {
        parser->error = CANT_OPEN_FILE;
        return NULL;
    }
    // print(vstring(sstr), true);
    struct Parser p;
    init_parser(&p, sstr);
    while (has_next(&p))
    {
        parse(&p);
        // print(p.value, true);
        // if (p.value == NULL) { printf("p is null\n"); } else { printf("p is not null\n");}
        // print(p.value, true);
        if (p.error != NO_ERROR)
        {
            parser->error = p.error;
            return NULL;
        }
        v = eval(nsp, parser, p.value);
        if (parser->error != NO_ERROR)
        {
            return NULL;
        }
        // printf("...evaling...\n");
    }
    return eval(nsp, parser, p.value);
}

void eval_display(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    struct Value *v;
    if (lst->size != 2)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return;
    }
    v = checked_eval(nsp, parser, list_lookup(lst, 1));
    if (v != NULL) 
    {
        print(v, true);
    }
}

struct Value *eval_is_type(struct Namespace *nsp, struct Parser *parser, struct List *lst, enum Type t)
{
    if (lst->size != 2)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    struct Value *v = checked_eval(nsp, parser, list_lookup(lst, 1));
    if (v == NULL) return NULL;
    return v == NULL ? NULL : vboolean(v->type == t);
}

static inline struct Value *eval_is_boolean(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    return eval_is_type(nsp, parser, lst, BOOLEAN);
}

static inline struct Value *eval_is_symbol(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    return eval_is_type(nsp, parser, lst, SYMBOL);
}

static inline struct Value *eval_is_char(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    return eval_is_type(nsp, parser, lst, CHAR);
}

static inline struct Value *eval_is_procedure(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    return eval_is_type(nsp, parser, lst, PROCEDURE);
}

static inline struct Value *eval_is_list(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    return eval_is_type(nsp, parser, lst, LIST);
}

static inline struct Value *eval_is_number(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    return eval_is_type(nsp, parser, lst, NUMBER);
}

static inline struct Value *eval_is_string(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    return eval_is_type(nsp, parser, lst, NUMBER);
}

struct Value *eval_is_pair(struct Namespace *nsp, struct Parser *parser, struct List *lst)
{
    if (lst->size != 2)
    {
        parser->error = INCORRECT_NUMBER_OF_ARGS;
        return NULL;
    }
    struct Value *v = checked_typed_eval(nsp, parser, list_lookup(lst, 1), LIST, EXPECTED_PAIR);
    if (v == NULL) 
    {
        return NULL;
    }
    else if (is_empty(v->list))
    {
        return vboolean(false);
    }
    else
    {
        return vboolean(v->type == LIST);
    }
}


/* Integer procedures */

// Helper function for evaluating single argument numeric functions
// struct Value *eval_n_numbers(struct Namespace *nsp, struct Parser *parser, struct List *lst, unsigned long n)
// {
//     if (lst->size != (n - 1))
//     {
//         parser->error = INCORRECT_NUMBER_OF_ARGS;
//         return NULL;
//     }
//     struct Value *v = checked_typed_eval(nsp, parser, list_lookup(lst, 1), NUMBER, EXPECTED_NUMBER);
//     return v;
// }
// 
// struct Value *eval_(struct Namespace *nsp, struct Parser *parser, struct List *lst)
// {
//     if (!eval_n_numbers(nsp, parser, lst, 1)) return NULL;
// 
// }
// 
// 
// 
// 
// 
// /* Character procedures */
// struct Value *eval_char_integer(struct Namespace *nsp, struct Parser *parser, struct List *lst)
// {
//     if (lst->size != 1)
//     {
//         parser->error = INCORRECT_NUMBER_OF_ARGS;
//         return NULL;
//     }
//     struct Value *v = checked_typed_eval(nsp, parser, list_lookup(lst, 1), CHAR, EXPECTED_CHAR);
//     if (v == NULL)
//     {
//         return NULL;
//     }
// }

struct InternalFunction
{
    char *name;
    struct Value (*function_ptr)(struct Namespace*, struct Parser*, struct List*);
};

// Register an internal function
struct InternalFunction *register_function(
        char *fun_name, 
        struct Namespace *nsp,
        struct Value (*function_ptr)(struct Namespace*, struct Parser*, struct List*))
{
    define(nsp, vsymbol(fun_name), NULL);
    struct InternalFunction *it = malloc(sizeof(*it));
    it->name = fun_name;
    it->function_ptr = function_ptr;
    return it;
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
    else if (match("set!"))
    {
        eval_set(nsp, parser, lst);
        return NULL;
    }
    else if (match("+"))
    {
        define(nsp, vsymbol("+"), NULL);
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
    else if (match("car"))
    {
        return eval_car(nsp, parser, lst);
    }
    else if (match("cdr"))
    {
        return eval_cdr(nsp, parser, lst);
    }
    else if (match ("cons"))
    {
        return eval_cons(nsp, parser, lst);
    }
    else if (match("eq?"))
    {
        return eval_eq(nsp, parser, lst);
    }
    else if (match("display"))
    {
        eval_display(nsp, parser, lst);
        return NULL;
    }
    else if (match("load"))
    {
        return eval_load(nsp, parser, lst);
    }
    else if (match("boolean?"))
    {
        return eval_is_boolean(nsp, parser, lst);
    }
    else if (match("symbol?"))
    {
        return eval_is_symbol(nsp, parser, lst);
    }
    else if (match("char?"))
    {
        return eval_is_char(nsp, parser, lst);
    }
    else if (match("procedure?"))
    {
        return eval_is_procedure(nsp, parser, lst);
    }
    else if (match("list?"))
    {
        return eval_is_list(nsp, parser, lst);
    }
    else if (match("number?"))
    {
        return eval_is_number(nsp, parser, lst);
    }
    else if (match("string?"))
    {
        return eval_is_string(nsp, parser, lst);
    }
    else if (match("pair?"))
    {
        return eval_is_pair(nsp, parser, lst);
    }
    else
    {
        struct Value *proc = checked_typed_eval(nsp, parser, first, PROCEDURE, EXPECTED_PROC);
        if (proc == NULL) return NULL;
        return eval_proc(nsp, parser, lst, proc);
    }
#undef match
}

void load(struct Namespace *nsp, struct Parser *p, char *filename)
{
    struct List *l = list();
    append(l, vsymbol("load"));
    append(l, vstring(to_scm_string(filename)));
    eval_load(nsp, p, l);
}

struct Value *eval(struct Namespace *nsp, struct Parser *parser, struct Value *val)
{
    if (val == NULL) 
    {
        parser->error = CANT_EVAL_UNDEF;
        return NULL;
    }
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
