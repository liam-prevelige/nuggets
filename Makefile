# Makefile for 'nuggets'
#
# Lily Scott May 2021

L = libcs50
S = support
LLIBS = $L/libcs50-given.a
OBJS = game.o player.o grid.o $S/message.o $S/log.o
LIBS =

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(TESTING) -I$L -I$S 
CC = gcc
MAKE = make

all: server gridtest gametest playertest

server: server.o $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LLIBS) $(LIBS) -o $@

gridtest: gridtest.o player.o grid.o $S/message.o $S/log.o
	$(CC) $(CFLAGS) $^ $(LLIBS) -o $@

gametest: gametest.o $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LLIBS) $(LIBS) -o $@

playertest: playertest.o $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LLIBS) $(LIBS) -o $@

server.o: game.h $S/message.h $S/log.h grid.h
player.o: player.h $S/message.h
gametest.o: game.h $S/message.h $S/log.h
gridtest.o: grid.h $S/message.h $L/file.h
playertest.o: player.h $S/message.h
message.o: $S/message.h
log.o: $S/log.h
grid.o: grid.h $L/file.h
game.o: game.h grid.h $S/message.h

.PHONY: test valgrind clean

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f vgcore.*
	rm -f server
	rm -f gametest
	rm -f gridtest
	rm -f playertest
	rm -f core
