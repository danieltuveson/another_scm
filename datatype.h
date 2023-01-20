#ifndef DATATYPE
#define DATATYPE
#include <stdbool.h>
#include <stdlib.h>

/* Constants */
#define MAXIMUM_SYMBOL_LENGTH 255

/* Data structures */

enum Type
{
    LIST,
    SYMBOL,
    CHAR,
    NUMBER,
    STRING,
    BOOLEAN,
    PROCEDURE
};

struct Value
{
    enum Type type;
    union
    {
        struct List *list;
        char *symbol;
        char character;
        double number;
        struct List *string;
        bool boolean;
        struct List *proc;
    };
};

struct List 
{
    unsigned int size;
    unsigned int capacity;
    struct Value **values;
};

typedef struct List ScmString;

/* Function definitions */

// /* Creates a scheme list. Returns NULL on failure. */
struct List *list(void);

/* Adds a value to the list. Returns true on success, false otherwise */
bool append(struct List *lst, struct Value *v);

/* Remove element from the end of the list and return it */
struct Value *pop(struct List *lst);

/* Deletes a list and all of its elements */
void delete_list(struct List *lst);

/* Deletes a value */
void delete_value(struct Value *v);

/* Convert to and from Value strings */
ScmString *to_scm_string(char *str);
char *from_scm_string(ScmString *sstr);

/* Create a deep copy of a value and return it */
struct Value *copy_value(struct Value *v);

/* Utility functions */

/* Gets value from specified index (null if index out of bounds) */
static inline struct Value *list_lookup(struct List *lst, unsigned int index)
{
    return (index >= lst->size ? NULL : lst->values[index]);
}

/* Sugar for checking if a list is empty */
static inline bool is_empty(struct List *lst)
{
    return (lst != NULL && lst->size == 0);
}

/* Sugar for dealing with procs */
static inline struct Value *get_args(struct Value *v)
{
    return (v->type == PROCEDURE) ? list_lookup(v->proc, 0) : NULL;
}

static inline struct Value *get_body(struct Value *v)
{
    return (v->type == PROCEDURE) ? list_lookup(v->proc, 1) : NULL;
}

/* Sugar for creating heap-allocated values */
static inline struct Value *vsymbol(char *symbol)
{
    struct Value *v = malloc(sizeof(*v));
    if (v == NULL) return NULL;
    v->type = SYMBOL;
    v->symbol = symbol;
    return v;
}

static inline struct Value *vcharacter(char character)
{
    struct Value *v = malloc(sizeof(*v));
    if (v == NULL) return NULL;
    v->type = CHAR;
    v->character = character;
    return v;
}

static inline struct Value *vnumber(double number)
{
    struct Value *v = malloc(sizeof(*v));
    if (v == NULL) return NULL;
    v->type = NUMBER;
    v->number = number;
    return v;
}

static inline struct Value *vstring(struct List *string)
{
    struct Value *v = malloc(sizeof(*v));
    if (v == NULL) return NULL;
    v->type = STRING;
    v->string = string;
    return v;
}

static inline struct Value *vboolean(bool boolean)
{
    struct Value *v = malloc(sizeof(*v));
    if (v == NULL) return NULL;
    v->type = BOOLEAN;
    v->boolean = boolean;
    return v;
}

static inline struct Value *vlist(struct List *list)
{
    struct Value *v = malloc(sizeof(*v));
    if (v == NULL) return NULL;
    v->type = LIST;
    v->list = list;
    return v;
}

static inline struct Value *vproc(struct Value *args, struct Value *body)
{
    struct Value *v = malloc(sizeof(*v));
    if (v == NULL) 
    {
        return NULL;
    }
    struct List *proc = list();
    if (proc == NULL) 
    {
        free(v);
        return NULL;
    }
    v->type = PROCEDURE;
    v->proc = proc;
    append(proc, args);
    append(proc, body);
    return v;
}


#endif
