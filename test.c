#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "error.h"
#include "datatype.h"
#include "repl.h"
#include "parser.h"


/* Helper function for checking that the state of the parser is what we expect */ 
void assert_init_parser(struct Parser p, enum Error err, unsigned int row, unsigned int column, unsigned int index)
{
    assert((p.error == NO_ERROR && err == NO_ERROR) || (p.error != NO_ERROR && err != NO_ERROR));
    assert(p.error == NO_ERROR);
    assert(p.row == row);
    assert(p.column == column);
    assert(p.index == index);
}

void test_parse_list()
{
    ScmString *sstr;
    struct Parser p;

    // Test simple list
    sstr = to_scm_string(" ( this  is  4  atoms  )");
    init_parser(&p, sstr);

    // Check parser struct 
    assert(parse_list(&p));
    assert_init_parser(p, NO_ERROR, 1, 25, 24);

    // Check internal value
    assert(p.value != NULL);
    assert(p.value->type == LIST);
    assert(p.value->list->size == 4);
    assert(strcmp(p.value->list->values[0]->symbol, "this") == 0);
    assert(strcmp(p.value->list->values[1]->symbol, "is") == 0);
    assert(p.value->list->values[2]->number == 4);
    assert(strcmp(p.value->list->values[3]->symbol, "atoms") == 0);

    // Test nested lists
    sstr = to_scm_string("(this is a (nested ( (list  ) (with) (internal)) sublists))");
    init_parser(&p, sstr);

    // Check parser struct 
    assert(parse_list(&p));
    assert_init_parser(p, NO_ERROR, 1, 60, 59);

    // Check internal value
    assert(p.value != NULL);
    assert(p.value->type == LIST);
    assert(p.value->list->size == 4);
    assert(strcmp(p.value->list->values[0]->symbol, "this") == 0);
    assert(strcmp(p.value->list->values[1]->symbol, "is") == 0);
    assert(strcmp(p.value->list->values[2]->symbol, "a") == 0);
    assert(strcmp(p.value->list->values[3]->list->values[0]->symbol, "nested") == 0);
    assert(strcmp(p.value->list->values[3]->list->values[1]->list->values[0]->list->values[0]->symbol, "list") == 0);
    assert(strcmp(p.value->list->values[3]->list->values[1]->list->values[1]->list->values[0]->symbol, "with") == 0);
    assert(strcmp(p.value->list->values[3]->list->values[1]->list->values[2]->list->values[0]->symbol, "internal") == 0);
    assert(strcmp(p.value->list->values[3]->list->values[2]->symbol, "sublists") == 0);
}

/* Tests for parsing a symbol */
void test_parse_symbol()
{
    ScmString *sstr;
    struct Parser p;

    // Test false
    sstr = to_scm_string("flippity-floo");
    init_parser(&p, sstr);

    // Check parser struct 
    assert(parse_atom(&p));
    assert_init_parser(p, NO_ERROR, 1, 14, 13);

    // Check internal value
    assert(p.value != NULL);
    assert(p.value->type == SYMBOL);
    assert(strcmp(p.value->symbol, "flippity-floo") == 0);
}

/* Tests for parsing number values */ 
void test_parse_number()
{
    ScmString *sstr;
    struct Parser p;

    // Test positive int
    sstr = to_scm_string("1234567890");
    init_parser(&p, sstr);

    // Check parser struct 
    assert(parse_atom(&p));
    assert_init_parser(p, NO_ERROR, 1, 11, 10);

    // Check internal value
    assert(p.value != NULL);
    assert(p.value->type == NUMBER);
    assert(p.value->number == 1234567890);
}

/* Tests for parsing hash-prefixed values (bools and chars) */ 
void test_parse_hash()
{
    ScmString *sstr;
    struct Parser p;

    // Test false
    sstr = to_scm_string("#f");
    init_parser(&p, sstr);

    // Check parser struct 
    assert(parse_atom(&p));
    assert_init_parser(p, NO_ERROR, 1, 3, 2);

    // Check internal value
    assert(p.value != NULL);
    assert(p.value->type == BOOLEAN);
    assert(p.value->boolean == 0);

    // Test true
    sstr = to_scm_string("#t");
    init_parser(&p, sstr);

    // Check parser struct 
    assert(parse_atom(&p));
    assert_init_parser(p, NO_ERROR, 1, 3, 2);
    assert(p.value != NULL);
    assert(p.value->type == BOOLEAN);
    assert(p.value->boolean == 1);


    // Test a character
    sstr = to_scm_string("#\\s");
    init_parser(&p, sstr);

    // Check parser struct 
    assert(parse_atom(&p));
    assert_init_parser(p, NO_ERROR, 1, 4, 3);
    assert(p.value != NULL);
    assert(p.value->type == CHAR);
    assert(p.value->character == 's');


    // Test space
    sstr = to_scm_string("#\\space");
    init_parser(&p, sstr);

    // Check parser struct 
    assert(parse_atom(&p));
    assert_init_parser(p, NO_ERROR, 1, 8, 7);
    assert(p.value != NULL);
    assert(p.value->type == CHAR);
    assert(p.value->character == ' ');

    // Test tab
    sstr = to_scm_string("#\\tab");
    init_parser(&p, sstr);

    // Check parser struct 
    assert(parse_atom(&p));
    assert_init_parser(p, NO_ERROR, 1, 6, 5);
    assert(p.value != NULL);
    assert(p.value->type == CHAR);
    assert(p.value->character == '\t');

    // Test newline
    sstr = to_scm_string("#\\newline");
    init_parser(&p, sstr);

    // Check parser struct 
    assert(parse_atom(&p));
    assert_init_parser(p, NO_ERROR, 1, 10, 9);
    assert(p.value != NULL);
    assert(p.value->type == CHAR);
    assert(p.value->character == '\n');

    // Test bad bool-looking input
    sstr = to_scm_string("#r");
    init_parser(&p, sstr);

    assert(!parse_atom(&p));
    assert(p.error != NO_ERROR);
    assert(p.value == NULL);

    // Test early termination of bool-looking input
    sstr = to_scm_string("#");
    init_parser(&p, sstr);

    assert(!parse_atom(&p));
    assert(p.error != NO_ERROR);
    assert(p.value == NULL);

    // Test bad char-looking input
    sstr = to_scm_string("#\\ta");
    init_parser(&p, sstr);

    assert(!parse_atom(&p));
    assert(p.error != NO_ERROR);
    assert(p.value == NULL);

    // Test early termination of char-looking input
    sstr = to_scm_string("#\\");
    init_parser(&p, sstr);

    assert(!parse_atom(&p));
    assert(p.error != NO_ERROR);
    assert(p.value == NULL);
}

/* Tests for parsing Scheme's string type */
void test_parse_string()
{
    ScmString *sstr;
    struct Parser p; 
    sstr = to_scm_string("\"this is \\nsome input\" jpwrijnfpwirj ");
    init_parser(&p, sstr);

    // Check parser struct 
    assert(parse_atom(&p));
    assert_init_parser(p, NO_ERROR, 1, 23, 22);

    // Check internal value
    assert(p.value != NULL);
    assert(p.value->type == STRING);
    assert(p.value->string != NULL);
    assert(!is_empty(p.value->string));
    assert(strcmp(from_scm_string(p.value->string), "this is \\nsome input") == 0);
}

/* Tests for making sure that the list type works properly */
void test_list()
{
    struct List *lst = list();

    struct Value *v = malloc(sizeof(*v));
    v->type = NUMBER;
    v->number = 3.14;

    // testing basic append
    append(lst, v);
    assert(lst->capacity == 8);
    assert(lst->size == 1);
    assert(lst->values[0]->number == 3.14);

    // test that capacity grows at a certain threshold
    for (unsigned int i = 0; i < 10; i++)
    {
        v = malloc(sizeof(*v));
        v->type = NUMBER;
        v->number = i;
        append(lst, v);
        assert(lst->size == (i+2));
    }
    assert(lst->capacity == 16);
    assert(lst->values[9]->number == 8);

    // test pop
    v = pop(lst);
    assert(v->number == 9);
    assert(lst->size == 10);

    v = pop(lst);
    assert(v->number == 8);
    assert(lst->size == 9);
    assert(lst->capacity == 16);

    for (int i = 0; i < 8; i++) v = pop(lst);
    assert(v->number == 0);
    assert(lst->size == 1);
    assert(lst->capacity == 8);

    v = pop(lst);
    assert(v->number == 3.14);
    assert(lst->size == 0);
    assert(lst->capacity == 8);

    assert(pop(lst) == NULL);

    // Stress test growth / shrinking
    for (unsigned int i = 0; i < 10000; i++)
    {
        v = malloc(sizeof(*v));
        v->type = SYMBOL;
        v->symbol = "hi";
        append(lst, v);
    }
    assert(strcmp(lst->values[9999]->symbol, "hi") == 0);
    assert(lst->size == 10000);
    assert(lst->capacity == 16384);

    for (unsigned int i = 0; i < 7500; i++)
    {
        pop(lst);
    }
    assert(lst->size == 2500);
    assert(lst->capacity == 8192);
    do { v = pop(lst); } while (v != NULL);
    assert(lst->size == 0);
    assert(lst->capacity == 8);
}

int main()
{
    test_list();
    test_parse_string();
    test_parse_hash();
    test_parse_number();
    test_parse_symbol();
    test_parse_list();
    printf("ran tests successfully\n");
}
