# Makefile for 'gameserver.c' module
#
# Max Zhuang, Raunak Bhojwani and Samuel Ching May 2016

PROG = guideagent
OBJS = guideagent.o
LIBS =
LLIBS = ../lib/lib.a

CFLAGS = -Wall -pedantic -std=c11 -ggdb -lm
CC = gcc
MAKE = make

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

# build the server
$(PROG): $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@



# server source dependencies; add others as needed

# build the library

../lib/lib.a:
	cd ../lib; $(MAKE)

guideagent.o: guideagent.c ../lib/lib.a


clean:
	rm -f *~
	rm -f *.o
	rm -f $(PROG)
	rm -f core.*
	rm -f vgcore.*
	rm -f indexfile
	cd ../lib; $(MAKE) $@
