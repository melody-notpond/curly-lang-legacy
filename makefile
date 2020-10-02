##
## Curly
## makefile
##
## jenra
## March 3 2020
##

CC = gcc
CPPC = g++
CFLAGS = -Wall -O0 -ggdb3 $(shell llvm-config --cflags)
CPPFLAGS = $(shell llvm-config --cxxflags --ldflags --libs all)
ifeq ($(shell uname), Darwin)
	CPPFLAGS += -lm -lz -lcurses -lxml2
else
	CPPFLAGS += $(shell llvm-config --system-libs)
endif
LIBS = -ledit

CODE = src/

all: *.o
	$(CPPC) $(CPPFLAGS) $(LIBS) -o curly *.o

debug: *.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o curly *.o $(LIBS)

*.o: main compiler utils

main: $(CODE)*.c
	$(CC) $(CFLAGS) -c $?

compiler: frontend backends

frontend: $(CODE)compiler/frontend/*/*.c
	$(CC) $(CFLAGS) -c $?

backends: $(CODE)compiler/backends/*/*.c
	$(CC) $(CFLAGS) -c $?

utils: $(CODE)utils/*.c
	$(CC) $(CFLAGS) -c $?

clean:
	-rm *.o
