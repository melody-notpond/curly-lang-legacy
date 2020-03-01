##
## Curly parser combinator
## makefile
##
## jenra
## February 25 2020
##

CC = gcc
CFLAGS = -Wall -O0 -ggdb3
LDFLAGS = 
LIBS = 

CODE = src/

all: *.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o test_combs.out $? $(LIBS)

*.o: parse test

parse: $(CODE)parse/*.c
	$(CC) $(CFLAGS) -c $?

test: $(CODE)test.c
	$(CC) $(CFLAGS) -c $?

clean:
	-rm *.o
