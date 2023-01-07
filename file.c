#include <stdio.h>
#include "datatype.h"
#include "error.h"

ScmString *read_file(char *filename)
{
    if (filename == NULL) return NULL;
    char c;
    struct Value *vchar;
    ScmString *sstr; 

    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        return NULL;
    }

    sstr = list();
    if (sstr == NULL) return NULL;
    while ((c = fgetc(fp)) != EOF)
    {
        vchar = vcharacter(c);
        if (vchar == NULL) 
        {
            delete_list(sstr);
            return NULL;
        }
        else if (!append(sstr, vchar))
        {
            delete_value(vchar);
            delete_list(sstr);
            return NULL;
        }
    }
    return sstr;
}
