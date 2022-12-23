#ifndef EVAL
#define EVAL

#include "datatype.h"

struct Variable
{
    char *name;
    struct Value *value;
};

struct VariableList
{
    struct Variable *var;
    struct VariableList *next;
};

struct Environment
{
    struct VariableList *varlist;
    struct Environment *parent;
};

#endif
