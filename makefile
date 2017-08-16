
CC=gcc
#CFLAGS=-m64 -c -Wall -g -O0 -fPIC 
CFLAGS=-c -Wall -g -O0 -fPIC 
#CFLAGS=-c -Wall -03
LDFLAGS=  -lreadline -lcurses -lm 
SOURCES=calc.c cnode.c vnode.c msgNode.c parse.c funcNode.c
OBJECTS=$(SOURCES:.c=.o)
#EXECUTABLE=calc
#this command makes a shared library out of the backend files
#calc.c should be able to call it as a single library
#make sure to delet calc.o before calling the below command
#gcc *.o -shared -fPIC -o libparseeqn.so

all: $(SOURCES) calc

calc: $(OBJECTS) 
	$(CC) $(DEBUG) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(DEBUG) $(CFLAGS) $*.c
	$(CC) $(DEBUG) $(CFLAGS) $< -o $@

shared:
	$(CC) $(OBJECTS) -shared -fPIC $(LDFLAGS) -o libparseeqn.so


clean:
	rm $(OBJECTS) calc

debug:
	$(MAKE) $(MAKEFILE) DEBUG="-DDEBUG"
