#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "datatype.h"
#include "parser.h"


void init_parser(struct Parser *parser, struct PString *pstr)
{
    parser->error = NO_ERROR;
    parser->value = NULL;
    parser->row = 1;
    parser->column = 1;
    parser->index = 0;
    parser->input = pstr;
}


// Parses content of string, returns as list of characters. 
// Assumes that leading quote has been consumed already.
int parse_string(struct Parser *parser)
{
    struct List *lst = list();
    char c, prevc;
    prevc = 'i'; // choosing a random non-escapy-y char for initialization

    while (has_next(parser))
    {
        c = next(parser);
        if (c == '\n')
        {
            parser->error = NO_NEWLINE_IN_STRING;
            goto error;
        }
        else if (c == '"' && prevc != '\\')
        {
            parser->value = vstring(lst);
            return PARSE_SUCCESS;
        }
        else if (prevc == '\\') 
        {
            if (c == 'n' || c == 't' || c == '\\' || c == '"')
            {
                append(lst, vcharacter(c));
            }
            else 
            {
                parser->error = UNKNOWN_ESCAPE_SEQUENCE;
                goto error;
            }
        }
        else
        {
            append(lst, vcharacter(c));
        }
        prevc = c;
    }
    parser->error = UNTERMINATED_STRING;
error:
    delete_list(lst);
    return PARSE_SUCCESS;
}


// Helper function for parsing #\space and #\newline characters
int check_word(struct Parser *parser, const char *str, unsigned int length, char to_return)
{
    for (unsigned int i = 0; i < length; i++)
    {
        if (!has_next(parser) 
                || is_terminal(peek(parser)) 
                || str[i] != next(parser)) 
        {
            parser->error = INVALID_CHAR;
            return PARSE_FAILURE;
        }
    }
    parser->value = vcharacter(to_return);
    return PARSE_SUCCESS;
}

int parse_character(struct Parser *parser)
{
    if (!has_next(parser))
    {
        parser->error = UNEXPECTED_END_OF_CHAR;
        return PARSE_FAILURE;
    }

    // Make this less stupid in the future(?)
    // Maybe it's fine, there aren't that many weird escape characters
    char c = next(parser);

    if (has_next(parser) && !is_terminal(peek(parser)))
    {
        if (c == 's')
        {
            char str[] = {'p', 'a', 'c', 'e'};
            return check_word(parser, str, 4, ' ');
        }
        else if (c == 't')
        {
            char str[] = {'a', 'b'};
            return check_word(parser, str, 2, '\t');
        }
        else if (c == 'n')
        {
            char str[] = {'e', 'w', 'l', 'i', 'n', 'e'};
            return check_word(parser, str, 6, '\n');
        }
    }
    parser->value = vcharacter(c);
    return PARSE_SUCCESS;
}

// Assumes we're starting with a string stripped of leading spaces
// Assumes there are more characters to be parsed
int parse_atom(struct Parser *parser)
{
    if (!has_next(parser))
    {
        parser->error = UNEXPECTED_END_OF_INPUT;
        return PARSE_FAILURE;
    }

    char c = peek(parser);
    if (c == '"')
    {
        next(parser);
        return parse_string(parser);
    }
    else if (c == '#')
    {
        next(parser);
        if (!has_next(parser))
        {
            parser->error = UNEXPECTED_END_OF_HASH;
            return PARSE_FAILURE;
        }
        c = next(parser);
        if (c == 'f')
        {
            parser->value = vboolean(0);
            return PARSE_SUCCESS;
        }
        else if (c == 't')
        {
            parser->value = vboolean(1);
            return PARSE_SUCCESS;
        }
        else if (c == '\\')
        {
            return parse_character(parser);
        }
        else
        {
            parser->error = INVALID_CHAR;
            return PARSE_FAILURE;
        }
    }
    else if (c == '\'')
    {
        // TODO handle quoting
    }
    else if (c >= '0' && c <= '9')
    {
        // Technically scheme allows identifiers that start with numbers, 
        // but that feels stupid, so I'm not going to allow it

        // TODO handle decimal, negatives, & overflow checks
        // Also need to figure out if this method actually works for 
        // double values greater than INT_MAX since floating point 
        // stuff is weird
        double num = 0;

        while (1)
        {
            c = peek(parser);
            if (!(c >= '0' && c <= '9'))
            {
                parser->error = INVALID_NUM_CHAR;
                return PARSE_FAILURE;
            }
            num *= 10;
            num += (c - '0');
            next(parser);
            if (!has_next(parser) || is_terminal(peek(parser)))
                break;
        } 
        parser->value = vnumber(num);
        return PARSE_SUCCESS;
    }

    // Pretty much anything that isn't one of the above is an identifier
    char content[MAXIMUM_SYMBOL_LENGTH];
    unsigned int i = 0;
    for (i = 0; i < MAXIMUM_SYMBOL_LENGTH; i++)
    {
        if (!has_next(parser) || is_terminal(peek(parser)))
        {
            char *varname = calloc(i+1, sizeof(char));
            strncpy(varname, content, i);
            parser->value = vsymbol(varname);
            return PARSE_SUCCESS;
        }
        content[i] = next(parser);
    }
    if (i == MAXIMUM_SYMBOL_LENGTH + 1)
    {
        parser->error = MAXIMUM_SYMBOL_LENGTH_EXCEEDED;
        return PARSE_FAILURE;
    }

    parser->error = UNHANDLED_DATA_TYPE;
    return PARSE_FAILURE;
}


// parse a scheme list
int parse_list(struct Parser *parser)
{
    spaces(parser);
    if (!has_next(parser))
    {
        parser->error = UNEXPECTED_END_OF_INPUT;
        return PARSE_FAILURE;
    }
    else if (peek(parser) != '(')
    {
        parser->error = EXPECTED_OPEN_PAREN;
        return PARSE_FAILURE;
    }

    next(parser);
    spaces(parser);

    struct List *lst = list();

    while (has_next(parser) && peek(parser) != ')')
    {
        if (!parse(parser))
        {
            goto err;
        }
        append(lst, parser->value);
        spaces(parser);
    }

    if (!has_next(parser))
    {
        parser->error = UNEXPECTED_END_OF_LIST;
        goto err;
    }
    next(parser);
    parser->value = vlist(lst);
    return PARSE_SUCCESS;

err:
    delete_list(lst);
    return PARSE_FAILURE;
}

int parse(struct Parser *parser)
{
    if (has_next(parser) && peek(parser) == '(')
    {
        if (parse_list(parser))
        {
            return PARSE_SUCCESS;
        }
        else 
        {
            return PARSE_FAILURE;
        }
    }
    else if (parse_atom(parser))
    {
        return PARSE_SUCCESS;
    }
    else 
    {
        return PARSE_FAILURE;
    }
}
