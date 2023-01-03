
CFLAGS = -std=c99 -Wall -Wextra -g

OBJECTS = utils.o repl.o datatype.o namespace.o parser.o eval.o

scheme : main.o $(OBJECTS)
	cc $(CFLAGS) -o scheme main.o $(OBJECTS)

datatype.o : datatype.h
namespace.o : namespace.h datatype.h
eval.o : eval.h namespace.h datatype.h error.h
parser.o : parser.h datatype.h error.h
main.o : repl.h datatype.h
test.o : repl.h parser.h error.h utils.h datatype.h
repl.o : repl.h parser.h error.h utils.h namespace.h datatype.h eval.h

clean : 
	rm -rf *.o test scheme
