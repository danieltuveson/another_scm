#include <stdlib.h>
#include <string.h>
#include "utils.h"

// static inline bool is_space(char c)
// {
//     return (c == ' ' || c == '\n' || c == '\t');
// }

// Pretty sure I need to fix this... 
// or maybe just stick with null terminated strings...
struct PString *pstring(char *string)
{
    struct PString *pstring = malloc(sizeof(pstring));
    pstring->length = strlen(string); 
    pstring->string = string;
    return pstring;
}
