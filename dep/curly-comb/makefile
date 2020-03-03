##
## Curly parser combinator
## makefile
##
## jenra
## February 25 2020
##

CC = gcc
CFLAGS = -Wall -O3
LIBOUT = libcomb.a
CFLAGS_TEST = -Wall -O0 -ggdb3

CODE = src/

all: parse
	ar -rc $(LIBOUT) *.o
	ranlib $(LIBOUT)

parse: $(CODE)parse/*.c
	$(CC) $(CFLAGS) -c $?

test: test_main test_parse
	$(CC) $(CFLAGS_TEST) $(LDFLAGS) -o test_combs.out *.o $(LIBS)

test_main: $(CODE)test.c
	$(CC) $(CFLAGS_TEST) -c $?

test_parse: $(CODE)parse/*.c
	$(CC) $(CFLAGS_TEST) -c $?

clean:
	-rm *.o
