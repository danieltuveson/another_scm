#ifndef EVAL
#define EVAL

#include "parser.h"
#include "datatype.h"
#include "namespace.h"

/* Data structures */

/* Function definitions */

/* Evaluates a scheme value in the current namespace */
struct Value *eval(struct Namespace *nsp, struct Parser *parser, struct Value *val);

/* Loads and evaluates external Scheme source */
void load(struct Namespace *nsp, struct Parser *p, char *filename);

/* Utility functions */

#endif
