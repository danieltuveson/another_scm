#include <stdlib.h> 
#include <limits.h>
#include "datatype.h"

/* Internal constants */

const unsigned int MAX_LIST_CAPACITY = UINT_MAX;
const unsigned char INIT_LIST_CAPACITY = 8;

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


/* Exposed functions */
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
        return NULL;
    }
    lst->size = 0;
    lst->capacity = INIT_LIST_CAPACITY;
    return lst;
}

int append(struct List *lst, struct Value *v)
{
    if (lst->size == lst->capacity)
    {
        if (!grow_list(lst))
        {
            return 0;
        }
    }
    lst->values[lst->size] = v;
    lst->size++;
    return 1;
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

// Need to update this to handle deleting nested lists. 
void delete_list(struct List *lst)
{
    for (unsigned int i = 0; i < lst->size; i++)
    {
        free(lst->values[i]);
    }
    free(lst->values);
    free(lst);
}

/* Copy size elements from src to tgt. Assumes both lists have at least size elements. */
void copy_values(struct Value **tgt, struct Value **src, unsigned int size)
{
    for (unsigned int i = 0; i < size; i++)
    {
        tgt[i] = src[i];
    }
}

static int grow_list(struct List *lst)
{
    // If growing the list would cause overflow, then return
    if (lst->capacity > (MAX_LIST_CAPACITY / GROWTH_FACTOR))
    {
        return 0;
    }
    unsigned int new_capacity = lst->capacity * GROWTH_FACTOR;

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

struct Value *to_vstring(char *str)
{
    struct List *lst = list();
    for (; *str != '\0'; str++)
    {
        append(lst, vcharacter(*str));
    }
    return vstring(lst);
}

char *from_vstring(struct Value *vstr)
{
    if (vstr == NULL || vstr->type != STRING)
    {
        return NULL;
    }
    unsigned int i = 0;
    char *str = calloc(vstr->string->size + 1, sizeof(char));
    for (i = 0; i < vstr->string->size; i++)
    {
        str[i] = vstr->string->values[i]->character;
    }
    str[i+1] = '\0';
    return str;
}
