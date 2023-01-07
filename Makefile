
CFLAGS = -std=c99 -Wall -Wextra -g

OBJECTS = repl.o datatype.o namespace.o parser.o eval.o file.o

scheme : main.o $(OBJECTS)
	cc $(CFLAGS) -o scheme main.o $(OBJECTS)

test : test.o $(OBJECTS)
	cc $(CFLAGS) -o test test.o $(OBJECTS)

datatype.o : datatype.h
namespace.o : namespace.h datatype.h
eval.o : eval.h file.h namespace.h datatype.h error.h
parser.o : parser.h datatype.h error.h
main.o : repl.h datatype.h
test.o : repl.h parser.h error.h datatype.h
repl.o : repl.h parser.h error.h namespace.h datatype.h eval.h
file.o : error.h datatype.h

clean : 
	rm -rf *.o test scheme
