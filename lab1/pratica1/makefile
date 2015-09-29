DESTPATH='/home/ubfeup/Área de Trabalho/rcom_prat1/'
CC=gcc
CFLAGS=-Wall
EXECUTABLE=rcom_pratica_1

all: noncanonical writenoncanonical

writenoncanonical: writenoncanonical.o pratica1framework.o
	$(CC) $(DESTPATH)writenoncanonical.o $(DESTPATH)pratica1framework.o -o $(DESTPATH)writenoncanonical $(CFLAGS)

writenoncanonical.o: writenoncanonical.c pratica1framework.h
	$(CC) -c writenoncanonical.c $(CFLAGS) -o $(DESTPATH)writenoncanonical.o

noncanonical: noncanonical.o pratica1framework.o
	$(CC) $(DESTPATH)noncanonical.o $(DESTPATH)pratica1framework.o -o $(DESTPATH)noncanonical $(CFLAGS)

noncanonical.o: noncanonical.c pratica1framework.h
	$(CC) -c noncanonical.c $(CFLAGS) -o $(DESTPATH)noncanonical.o
	
pratica1framework.o: pratica1framework.c pratica1framework.h
	$(CC) -c pratica1framework.c $(CFLAGS) -o $(DESTPATH)pratica1framework.o

clean:
	-rm $(DESTPATH)*.o
	-rm $(DESTPATH)noncanonical
	-rm $(DESTPATH)writenoncanonical
