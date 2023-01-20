#include <stdio.h>
#include "datatype.h"
#include "print.h"

void print(struct Value *v, bool newline)
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

