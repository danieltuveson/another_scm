#ifndef UTILS
#define UTILS

/* Data structures */

/* String with length - like a pascal string */
struct PString
{
    unsigned int length;
    char *string;
};

/* Function definitions */

struct PString *pstring(char *string);

#endif
