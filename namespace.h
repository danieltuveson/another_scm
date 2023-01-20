#ifndef NAMESPACE
#define NAMESPACE

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

/* Namespace stores a list of bindings and a pointer
 * to the parent namespace */
struct Namespace
{
    Bindings *bindings;
    struct Namespace *parent;
};


/* Function definitions */

/* Initializes namespace with given parent namespace
 * If no parent namespace exists, pass in NULL for parent */
void init_nsp(struct Namespace *nsp, struct Namespace *parent);

/* Allocates a new namespace, then runs init_nsp with its parent */
struct Namespace *new_nsp(struct Namespace *parent);

/* Checks if variable is bound to lname in current namespace and
 * recursively checks parents if not found
 * Returns NULL if not found */
struct Value *lookup_var(struct Namespace *nsp, char *lname);

/* Adds binds symbol to input value within current namespace's bindings */
void define(struct Namespace *nsp, struct Value *symbol, struct Value *val);

/* Utility functions */

/* helper functions for handling bindings */
static inline struct Value *get_name(Binding *b)
{
    return list_lookup(b->list, 0);
}

static inline struct Value *get_value(Binding *b)
{
    return list_lookup(b->list, 1);
}

static inline void set_value(Binding *b, struct Value *v)
{
    b->list->values[1] = v;
}

static inline void push_var(Bindings *bindings, Binding *bind)
{
    append(bindings, bind);
}


#endif

