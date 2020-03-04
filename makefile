##
## Curly
## makefile
##
## jenra
## March 3 2020
##

CC = gcc
CFLAGS = -Wall -O0 -ggdb3 -I'dep/curly-comb/src/parse'
LDFLAGS = 
LIBS = -L'dep/curly-comb' -lcomb

CODE = src/

all: *.o libcomb.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o curly *.o $(LIBS)

*.o: main

libcomb.a:
	cd dep/curly-comb && make -f makefile

main: $(CODE)*.c
	$(CC) $(CFLAGS) -c $?

clean:
	-rm *.o
	-rm dep/curly-comb/*.o
