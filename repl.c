#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "datatype.h"
#include "parser.h"
#include "repl.h"
#include "eval.h"
#include "error.h"


/* Read-Eval-Print Loop for Scheme interpreter */
struct PString *read(void)
{
    unsigned int i, buff_size, parencount;
    char c, *buff;
    buff_size = 2000;
    parencount = 0;
    buff = calloc(buff_size, sizeof(char));
    c = fgetc(stdin);

    for (i = 0; !(c == '\n' && parencount == 0); 
            i++, c = fgetc(stdin))
    {
        if (c == EOF)
        {
            putchar('\n');
            exit(0);
        }
        if (i == buff_size)
        {
            // TODO: get rid of arbitrary limit
            printf("Exceeded maximum size of %d"
                    " characters for command line input\n", buff_size);
            return NULL;
        }
        else
        {
            buff[i] = c;
        }

        if (c == '(')
        {
            parencount++;
        }
        else if (c == ')')
        {
            parencount--;
        }
    }

    struct PString *pstr = malloc(sizeof(*pstr));
    pstr->length = i;
    pstr->string = buff;

    return pstr;
}

void print( /* should take a scheme value */ )
{
}

void repl()
{
    struct PString *pstr;
    struct Parser p;
    struct Environment env;
    enum Error err;

    while (1)
    {
        printf("> ");
        pstr = read();
        init_parser(&p, pstr);
        parse(&p);
        if (p->error != NO_ERROR)
        {
            printf("Error row %u, column %u: %s\n", 
                    p.row, p.column,
                    parse_error_to_string(p.error));
            continue;
        }
        eval(env, err, parser->value)
        if (eval(p->value
    }
}

