#ifndef PARSER
#define PARSER
#include <stdlib.h>
#include "utils.h"
#include "error.h"

/* Parser constants */
#define PARSE_FAILURE 0
#define PARSE_SUCCESS 1

/* Data structures */
struct Parser
{
    unsigned int row;
    unsigned int column;
    unsigned int index;
    struct PString *input;
    enum Error error;
    struct Value *value;
};


/* Function definitions */

/* Creates a new parser from the given PString */
void init_parser(struct Parser *parser, struct PString *pstr);

/* Parses a string token */
int parse_string(struct Parser *parser);

/* Parses any value that is not a list */
int parse_atom(struct Parser *parser);

/* Parses a list */
int parse_list(struct Parser *parser);

/* Parses a single scheme value (a list or an atom) */
int parse(struct Parser *parser);

/* Utility functions */

/* Iterate column */ 
static inline void itercol(struct Parser *p)
{
    p->column++;
    p->index++;
}

/* Iterate row */ 
static inline void iterrow(struct Parser *p)
{
    p->row++;
    p->index++;
}

/* Check if there are more characters */
static inline int has_next(struct Parser *p)
{
    return p->index < p->input->length;
}

/* look at next character without changing index */
static inline char peek(struct Parser *p)
{
    return p->input->string[p->index];
}

static inline int is_space(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

/* Check if next character would terminate atom */
static inline int is_terminal(char c)
{
    return (is_space(c) || c == '(' || c == ')');
}

/* Returns current character, moves index to next character 
 * Assumes that we've checked has_next already */
static inline char next(struct Parser *p)
{
    char c = p->input->string[p->index];
    if (c == '\n')
        p->row++;
    else 
        p->column++;
    p->index++;
    return c;
}

/* Parse and ignore whitespace */
static inline void spaces(struct Parser *p)
{
    while (has_next(p) && is_space(peek(p))) next(p);
}


#endif
