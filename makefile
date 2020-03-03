##
## Curly
## makefile
##
## jenra
## March 3 2020
##

CC = gcc
CFLAGS = -Wall -O0 -ggdb3 -I'curly-comb/src/parse'
LDFLAGS = 
LIBS = -Lcurly-comb -lcomb

CODE = src/

all: *.o libcomb.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o curly *.o $(LIBS)

*.o: src

libcomb.a:
	cd curly-comb && make -f makefile && cd ..

src: $(CODE)*.c
	$(CC) $(CFLAGS) -c $?

clean:
	-rm *.o
