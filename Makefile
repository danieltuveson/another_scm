
CFLAGS_DEBUG = -std=c99 -Wall -Wextra -g
CFLAGS_PROD = -std=c99 -O2

OBJECTS = utils.o repl.o datatype.o parser.o eval.o

debug : main.o $(OBJECTS)
	cc $(CFLAGS_DEBUG) -o scheme_debug main.o $(OBJECTS)

prod : main.o $(OBJECTS)
	cc $(CFLAGS_PROD) -o scheme main.o $(OBJECTS)

test : test.o $(OBJECTS)
	cc $(CFLAGS_TEST) -o test test.o $(OBJECTS) && ./test

datatype.o : datatype.h
eval.o : datatype.h error.h
parser.o : datatype.h error.h
main.o : repl.h datatype.h
test.o : repl.h parser.h error.h utils.h datatype.h
repl.o : repl.h parser.h error.h utils.h datatype.h eval.h

clean : 
	rm -rf *.o test scheme scheme_debug
