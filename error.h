#ifndef ERROR
#define ERROR

/* Error values for parse / evaluation errors */
enum Error
{
    NO_ERROR,

    /* parse errors */
    NO_NEWLINE_IN_STRING,
    UNKNOWN_ESCAPE_SEQUENCE,
    UNTERMINATED_STRING,
    INVALID_CHAR,
    UNEXPECTED_END_OF_CHAR,
    UNEXPECTED_END_OF_INPUT,
    UNEXPECTED_END_OF_HASH,
    UNHANDLED_DATA_TYPE,
    INVALID_NUM_CHAR,
    MAXIMUM_SYMBOL_LENGTH_EXCEEDED,
    UNCLOSED_PARENS,
    EXTRA_PARENS,
    UNEXPECTED_END_OF_LIST,
    EXPECTED_OPEN_PAREN,

    /* eval errors */ 
    FIRST_NOT_PROC,
    INCORRECT_NUMBER_OF_ARGS,
    SYMBOL_NOT_BOUND,
    UNDEFINED,
    CANT_OPEN_FILE,

    /* type errors */
    EXPECTED_SYMBOL,
    EXPECTED_BOOLEAN,
    EXPECTED_NUMBER,
    EXPECTED_CHAR,
    EXPECTED_PROC,
    EXPECTED_STRING,
    EXPECTED_LIST,
    EXPECTED_PAIR
};

/* Convert Error to friendly error message */
static inline char *parse_error_to_string(enum Error e)
{
    switch(e)
    {
        case NO_ERROR:
            return "no error";

        /* parse errors */
        case NO_NEWLINE_IN_STRING:
            return "newlines are not allowed in string literals";
        case UNKNOWN_ESCAPE_SEQUENCE:
            return "unknown escape sequence";
        case UNTERMINATED_STRING:
            return "unterminated string before end of file";
        case INVALID_CHAR:
            return "invalid character";
        case UNEXPECTED_END_OF_CHAR:
            return "unexpected end of character constant";
        case UNEXPECTED_END_OF_INPUT:
            return "unexpected end of input";
        case UNEXPECTED_END_OF_HASH:
            return "unexpected end of hash-prefixed value";
        case UNHANDLED_DATA_TYPE:
            return "unhandled data type";
        case INVALID_NUM_CHAR:
            return "invalid character in number";
        case MAXIMUM_SYMBOL_LENGTH_EXCEEDED:
            return "maximum length of symbol exceeded";
        case UNCLOSED_PARENS:
             return "unclosed parentheses";
        case EXTRA_PARENS:
             return "extraneous parentheses";
        case UNEXPECTED_END_OF_LIST:
             return "unexpected end of list";
        case EXPECTED_OPEN_PAREN:
             return "expected openning parenthesis";

        /* eval errors */
        case FIRST_NOT_PROC:
             return "first element of list must be a procedure";
        case INCORRECT_NUMBER_OF_ARGS:
             return "incorrect number of arguments passed to proceedure";
        case SYMBOL_NOT_BOUND:
             return "symbol is not bound to a value";
        case UNDEFINED:
             return "expression returned undefined";
        case CANT_OPEN_FILE:
             return "could not open file";

        /* type errors */
        case EXPECTED_SYMBOL:
             return "expected symbol";
        case EXPECTED_BOOLEAN:
             return "expected boolean";
        case EXPECTED_NUMBER:
             return "expected a numeric value";
        case EXPECTED_CHAR:
             return "expected a character";
        case EXPECTED_STRING:
             return "expected a string";
        case EXPECTED_PAIR:
             return "expected a pair";
        case EXPECTED_LIST:
             return "expected a list";
        case EXPECTED_PROC:
             return "expected a procedure";
    }
}

#endif
