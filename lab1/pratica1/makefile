CC=gcc
CFLAGS=-Wall
EXECUTABLE=rcom_pratica_1

all: noncanonical writenoncanonical

writenoncanonical: writenoncanonical.o pratica1framework.o
	$(CC) writenoncanonical.o pratica1framework.o -o writenoncanonical $(CFLAGS)

writenoncanonical.o: writenoncanonical.c pratica1framework.h
	$(CC) -c writenoncanonical.c $(CFLAGS)

noncanonical: noncanonical.o pratica1framework.o
	$(CC) noncanonical.o pratica1framework.o -o noncanonical $(CFLAGS)

noncanonical.o: noncanonical.c pratica1framework.h
	$(CC) -c noncanonical.c $(CFLAGS)
	
pratica1framework.o: pratica1framework.c pratica1framework.h
	$(CC) -c pratica1framework.c $(CFLAGS) -o pratica1framework.o

clean:
	rm *.o
	rm noncanonical
	rm writenoncanonical
