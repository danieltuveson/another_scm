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
        printf("\"%s\"", from_vstring(v));
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
    struct PString *pstr;
    struct Parser p;
    struct Environment env;
    struct Value *v;

    init_env(&env, NULL);

    while (1)
    {
        printf("> ");
        pstr = read();
        init_parser(&p, pstr);
        parse(&p);
        if (p.error != NO_ERROR)
        {
            printf("Error row %u, column %u: %s\n", 
                    p.row, p.column,
                    parse_error_to_string(p.error));
            continue;
        }
        v = eval(&env, &p, p.value);
        if (p.error != NO_ERROR)
        {
            printf("Error while executing: %s\n",
                    parse_error_to_string(p.error));
            continue;
        }
        if (v != NULL) print(v, true);
    }
}

