#ifndef ERROR
#define ERROR

/* Error values for parse / evaluation errors */
enum Error
{
    NO_ERROR,
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
    FIRST_NOT_SYMBOL,
    UNDEFINED_VARIABLE
};

/* Convert Error to friendly error message */
static inline char *parse_error_to_string(enum Error e)
{
    switch(e)
    {
        case NO_ERROR:
            return "no error";
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
        case FIRST_NOT_SYMBOL:
             return "first element of proceedure must be a symbol";
        case UNDEFINED_VARIABLE:
             return "variable is undefined";
    }
}

#endif
