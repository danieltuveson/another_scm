
CFLAGS = -std=c99 -Wall -Wextra -g

OBJECTS = utils.o repl.o datatype.o parser.o eval.o

scheme : main.o $(OBJECTS)
	cc $(CFLAGS) -o scheme main.o $(OBJECTS)

datatype.o : datatype.h
eval.o : datatype.h error.h
parser.o : datatype.h error.h
main.o : repl.h datatype.h
test.o : repl.h parser.h error.h utils.h datatype.h
repl.o : repl.h parser.h error.h utils.h datatype.h eval.h

clean : 
	rm -rf *.o test scheme scheme_debug
