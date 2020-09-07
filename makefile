##
## Curly
## makefile
##
## jenra
## March 3 2020
##

CC = gcc
CFLAGS = -Wall -O0 -ggdb3 $(shell llvm-config --cflags)
LDFLAGS = $(shell llvm-config --ldflags)
LIBS = $(shell llvm-config --libs) $(shell llvm-config --system-libs) # -lreadline

CODE = src/

all: *.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o curly *.o $(LIBS)

debug: *.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o curly *.o $(LIBS)

*.o: main compiler utils # vm

main: $(CODE)*.c
	$(CC) $(CFLAGS) -c $?

compiler: frontend # backends

frontend: $(CODE)compiler/frontend/*/*.c
	$(CC) $(CFLAGS) -c $?

backends: $(CODE)compiler/backends/*/*.c
	$(CC) $(CFLAGS) -c $?

utils: $(CODE)utils/*.c
	$(CC) $(CFLAGS) -c $?

vm: $(CODE)vm/*.c
	$(CC) $(CFLAGS) -c $?

clean:
	-rm *.o
