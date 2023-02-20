#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "datatype.h"
#include "namespace.h"
#include "parser.h"
#include "repl.h"
#include "eval.h"
#include "print.h"
#include "error.h"


/* Read-Eval-Print Loop for Scheme interpreter */
ScmString *read(void)
{
    unsigned int i, parencount;
    char c;
    parencount = 0;
    c = fgetc(stdin);

    ScmString *sstr = list();
    for (i = 0; !(c == '\n' && parencount == 0); 
            i++, c = fgetc(stdin))
    {
        if (c == EOF)
        {
            printf("\nexiting scheme\n");
            exit(0);
        }
        append(sstr, vcharacter(c));

        if (c == '(')
        {
            parencount++;
        }
        else if (c == ')')
        {
            parencount--;
        }
    }

    return sstr;
}

void repl()
{
    printf("Entering Scheme interpreter. Type Ctrl+D to exit\n");
    ScmString *sstr;
    struct Parser p;
    struct Namespace nsp;
    struct Value *v;

    init_nsp(&nsp, NULL);

    // Load standard library functions at the top level
    load(&nsp, &p, "load.scm");
    while (1)
    {
        printf("> ");
        sstr = read();
        init_parser(&p, sstr);
        parse(&p);
        if (p.error != NO_ERROR)
        {
            printf("Error row %u, column %u: %s\n", 
                    p.row, p.column,
                    parse_error_to_string(p.error));
            continue;
        }
        v = eval(&nsp, &p, p.value);
        if (p.error != NO_ERROR)
        {
            printf("Error while executing: %s\n",
                    parse_error_to_string(p.error));
            continue;
        }
        if (v != NULL) print(v, true);
    }
}

