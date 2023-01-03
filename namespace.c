#include <stdio.h>
#include <string.h>
#include <float.h>
#include <assert.h>
#include "error.h"
#include "parser.h"
#include "datatype.h"
#include "namespace.h"
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

void init_nsp(struct Namespace *nsp, struct Namespace *parent)
{
    nsp->bindings = list();
    nsp->parent = parent;
}

struct Namespace *new_nsp(struct Namespace *parent)
{
    struct Namespace *nsp = malloc(sizeof(*nsp));
    init_nsp(nsp, parent);
    return nsp;
}

// Gets index within current bindings of a name, if it exists
long get_binding_index(Bindings *binds, char *lname)
{
    if (is_empty(binds)) return -1;

    char *name;
    for (unsigned int i = 0; i < binds->size; i++)
    {
        // Found variable with this name
        name = get_name(binds->values[i])->symbol;
        if (strcmp(name, lname) == 0)
        {
            return i;
        }
    }
    return -1;
}

// Binds name to a value in the current namespace
void define(struct Namespace *nsp, struct Value *symbol, struct Value *val)
{
    long l = get_binding_index(nsp->bindings, symbol->symbol);

    // Binding exists in this namespace, overwrite its old value with the new one
    if (l != -1)
    {
        set_value(nsp->bindings->values[(unsigned int)l], val);
        return;
    }

    // If name isn't bound in the namespace, create a new binding
    Binding *bind = malloc(sizeof(*bind));
    new_var(bind, symbol, val);
    append(nsp->bindings, bind);
    return;
}

// This function looks for variable with the given name
struct Value *lookup_var(struct Namespace *nsp, char *lname)
{
    // Find index of variable with this name
    long index = get_binding_index(nsp->bindings, lname);

    if (index != -1)
    {
        return get_value(nsp->bindings->values[(unsigned int)index]);
    }
    else if (nsp->parent != NULL)
    {
        return lookup_var(nsp->parent, lname);
    }
    else 
    {
        return NULL;
    }
}
