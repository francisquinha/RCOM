CC=gcc
CFLAGS=-Wall
EXECUTABLE=rcom_lab1
PATH='/home/angie/Documents/RCOM/project1/'

all: noncanonical writenoncanonical

writenoncanonical: writenoncanonical.o pratica1framework.o
	$(CC) $(PATH)writenoncanonical.o $(PATH)pratica1framework.o -o $(PATH)writenoncanonical $(CFLAGS)

writenoncanonical.o: writenoncanonical.c pratica1framework.h
	$(CC) -c writenoncanonical.c $(CFLAGS) -o $(PATH)writenoncanonical.o

noncanonical: noncanonical.o pratica1framework.o
	$(CC) $(PATH)noncanonical.o $(PATH)pratica1framework.o -o $(PATH)noncanonical $(CFLAGS)

noncanonical.o: noncanonical.c pratica1framework.h
	$(CC) -c noncanonical.c $(CFLAGS) -o $(PATH)noncanonical.o
	
pratica1framework.o: pratica1framework.c pratica1framework.h
	$(CC) -c pratica1framework.c $(CFLAGS) -o $(PATH)pratica1framework.o

clean:
	-rm $(PATH)*.o
	-rm $(PATH)noncanonical
	-rm $(PATH)writenoncanonical
