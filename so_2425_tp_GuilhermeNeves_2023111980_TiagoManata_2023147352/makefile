C= gcc


all: compilado clean

compilado :manager feed

broker : manager.o 
	$C manager.o -o manager

manager.o : manager.c util.h 
	$C manager.c -c

feed: feed.o
	$C  feed.o -o feed

feed.o: feed.c util.h
	$C feed.c -c


clean: 
	rm *-o