#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "datatype.h"
#include "namespace.h"
#include "parser.h"
#include "repl.h"
#include "eval.h"
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

void print(struct Value *v, int newline)
{
    if (v == NULL)
    {
        printf("Error, cannot print NULL value\n");
        return;
    }
    switch (v->type)
    {
    case LIST:
        printf("(");
        struct List *l = v->list;
        for (unsigned int i = 0; i < l->size; i++)
        {
            print(l->values[i], 0);
            if (i < (l->size-1)) 
                printf(" ");
        }
        printf(")");
        break;
    case STRING:
        printf("\"%s\"", from_scm_string(v->string));
        break;
    case SYMBOL:
        printf("%s", v->symbol);
        break;
    case CHAR:
        printf("#\\%c", v->character);
        break;
    case NUMBER:
        printf("%f", v->number);
        break;
    case BOOLEAN:
        if (v->boolean)
            printf("#t");
        else
            printf("#f");
        break;
    case PROCEDURE:
        printf("(lambda ");
        print(v->proc->values[0], 0);
        printf(" ");
        print(v->proc->values[1], 0);
        printf(")");
        break;
    }
    if (newline) printf("\n");
}

void repl()
{
    printf("Entering Scheme interpreter. Type Ctrl+D to exit\n");
    ScmString *sstr;
    struct Parser p;
    struct Namespace nsp;
    struct Value *v;

    init_nsp(&nsp, NULL);

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

