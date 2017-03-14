# Makefile for 'lib' module
#
# Max Zhuang, Raunak Bhojwani, Samuel Ching April 2016


OBJS = memory.o message.o agent.o team.o codedrop.o jhash.o hashtable.o list.o latlongdist.o randomHexGen.o
LIB = lib.a
LLIBS = 

CFLAGS = -Wall -pedantic -std=c11 -ggdb -lm
CC = gcc
MAKE = make

$(LIB): $(OBJS) $(LLIBS)
	ar cr $@ $^

# add other targets for the other .o files

message.o: message.c message.h memory.h
agent.o: agent.c agent.h memory.h
team.o: team.c team.h agent.h list.h memory.h
memory.o: memory.c memory.h
codedrop.o: codedrop.c codedrop.h memory.h
hashtable.o: hashtable.c hashtable.h jhash.c jhash.h list.h
list.o: list.c list.h
latlongdist.o: latlongdist.c latlongdist.h
randomHexGen.o: randomHexGen.c randomHexGen.h memory.h



# add cleanup commands for other subdirectories
clean:
	rm -f *~
	rm -f *.o
	rm -f $(LIB)
