#ifndef EVAL
#define EVAL

#include "parser.h"
#include "datatype.h"

/* Data structures */

/* Binding is an alias for a value of a list of symbol-value pairs 
 * Binding should only contain:
 * - 1 symbol as the first element
 * - 1 value as the second element
 * - Should not contain more values */
typedef struct Value Binding;

/* Bindings is just an alias for a list of lists of the 
 * form of Binding Bindings should *not* contain 
 * non-Binding elements 
 *
 * At some point we should replace this with a real 
 * hash table */
typedef struct List Bindings;

/* Environment stores a list of bindings and a pointer
 * to the parent environment */
struct Environment
{
    Bindings *bindings;
    struct Environment *parent;
};


/* Function definitions */

/* Evaluates a scheme value in the current environment */
struct Value *eval(struct Environment *env, struct Parser *parser, struct Value *val);

/* Initializes environment with given parent environment
 * If no parent environment exists, pass in NULL for parent */
void init_env(struct Environment *env, struct Environment *parent);

/* Utility functions */

/* helper functions for handling bindings */
static inline char *get_name(Binding *b)
{
    return b->list->values[0]->symbol;
}

static inline struct Value *get_value(Binding *b)
{
    return b->list->values[1];
}

static inline void push_var(Bindings *bindings, Binding *bind)
{
    append(bindings, bind);
}




#endif
