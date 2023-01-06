#include <stdlib.h> 
#include <limits.h>
#include "datatype.h"

/* Internal constants */

const unsigned int MAX_LIST_CAPACITY = UINT_MAX;
const unsigned char INIT_LIST_CAPACITY = 8;

/* Forward declarations */

/* Code may rely on GROWTH_FACTOR being 
 * even (or even being 2) to function correctly
 */
const unsigned int GROWTH_FACTOR = 2;

/* Private function definitions */

/* Grows list capacity when list requires more space
 * Returns 1 on success.
 */
static int grow_list(struct List *lst);

/* Shrinks list capacity when list capacity is too large. 
 * Returns 1 on success.
 */
static int shrink_list(struct List *lst);

/* Deletes list and all of its contents */
void delete_list(struct List *lst);

struct List *list(void)
{
    struct List *lst;
    lst = malloc(sizeof(*lst));
    if (lst == NULL)
    {
        return NULL;
    }
    lst->values = calloc(INIT_LIST_CAPACITY, sizeof(*(lst->values)));
    if (lst->values == NULL)
    {
        free(lst);
        return NULL;
    }
    lst->size = 0;
    lst->capacity = INIT_LIST_CAPACITY;
    return lst;
}

bool append(struct List *lst, struct Value *v)
{
    if (lst->size == lst->capacity)
    {
        if (!grow_list(lst))
        {
            return false;
        }
    }
    lst->values[lst->size] = v;
    lst->size++;
    return true;
}

struct Value *pop(struct List *lst)
{
    if (lst->size == 0)
    {
        return NULL;
    }
    // Only shrink if we're reasonably sure we won't need to grow again soon
    else if (lst->size < lst->capacity / (GROWTH_FACTOR * GROWTH_FACTOR))
    {
        // We can ignore the return type - if it fails to shrink that's fine
        shrink_list(lst); 
    }
    lst->size--;
    struct Value *v = lst->values[lst->size];
    lst->values[lst->size] = NULL;
    return v;
}

void delete_value(struct Value *v)
{
    if (v == NULL) return;
    switch (v->type)
    {
        case LIST:
            delete_list(v->list);
            break;
        case SYMBOL:
            free(v->symbol);
            break;
        case STRING:
            delete_list(v->string);
            break;
        case PROCEDURE:
            delete_list(v->proc);
            break;
        default: // Num, bool, and char
            break;
    }
    free(v);
}

void delete_list(struct List *lst)
{
    if (lst == NULL) return;
    for (unsigned int i = 0; i < lst->size; i++)
    {
        delete_value(lst->values[i]);
    }
    free(lst->values);
    free(lst);
}

/* Shallow copy size elements from src to tgt. Assumes both lists have at least size elements. */
static void inline copy_values(struct Value **tgt, struct Value **src, unsigned int size)
{
    for (unsigned int i = 0; i < size; i++) tgt[i] = src[i];
}

static int grow_list(struct List *lst)
{
    // If growing the list would cause overflow, then return
    if (lst->capacity > (MAX_LIST_CAPACITY / GROWTH_FACTOR)) return 0;

    unsigned int new_capacity = lst->capacity * GROWTH_FACTOR;
    struct Value **values = calloc(new_capacity, sizeof(*values));
    if (values == NULL) return 0;

    // Copy over the elements
    copy_values(values, lst->values, lst->size);

    // Update values in the list
    free(lst->values);
    lst->values = values;
    lst->capacity = new_capacity;
    return 1;
}

static int shrink_list(struct List *lst)
{
    unsigned int new_capacity;
    if (lst->capacity / GROWTH_FACTOR < INIT_LIST_CAPACITY)
    {
        new_capacity = INIT_LIST_CAPACITY;
    }
    else
    {
        new_capacity = lst->capacity / GROWTH_FACTOR;
    }

    struct Value **values = calloc(new_capacity, sizeof(*values));
    if (values == NULL)
    {
        return 0;
    }

    // Copy over the elements
    copy_values(values, lst->values, lst->size);

    // Update values in the list
    free(lst->values);
    lst->values = values;
    lst->capacity = new_capacity;
    return 1;
}

ScmString *to_scm_string(char *str)
{
    ScmString *lst = list();
    if (lst == NULL) return NULL;
    struct Value *v;
    for (; *str != '\0'; str++)
    {
        v = vcharacter(*str);
        if (v == NULL) 
        {
            delete_list(lst);
            return NULL;
        }
        if (!append(lst, v)) 
        {
            delete_value(v);
            delete_list(lst);
            return NULL;
        }
    }
    return lst;
}

char *from_scm_string(ScmString *sstr)
{
    if (sstr == NULL)
    {
        return NULL;
    }
    unsigned int i = 0;
    char *str = calloc(sstr->size + 1, sizeof(char));
    if (str == NULL) return NULL;
    for (i = 0; i < sstr->size; i++)
    {
        str[i] = sstr->values[i]->character;
    }
    str[i+1] = '\0';
    return str;
}

struct Value *copy_value(struct Value *v)
{
    if (v == NULL) return NULL;

    struct Value *copy, *args_copy, *body_copy;
    struct List *lst;
    switch (v->type)
    {
        case LIST:
            lst = list();
            if (lst == NULL) return NULL;
            for (unsigned int i = 0; i < v->list->size; i++)
            {
                copy = copy_value(v->list->values[i]);
                if (copy == NULL) 
                {
                    delete_list(lst);
                    return NULL;
                }
                append(lst, copy);
            }
            return vlist(lst);
        case SYMBOL:
            return vsymbol(v->symbol);
        case CHAR:
            return vcharacter(v->character);
        case NUMBER:
            return vnumber(v->number);
        case STRING:
            return vstring(v->string);
        case BOOLEAN:
            return vboolean(v->boolean);
        case PROCEDURE:
            args_copy = copy_value(get_args(v));
            if (args_copy == NULL) 
            {
                return NULL;
            }
            body_copy = copy_value(get_body(v));
            if (body_copy == NULL) 
            {
                free(args_copy);
                return NULL;
            }
            return vproc(args_copy, body_copy);
    }
}
