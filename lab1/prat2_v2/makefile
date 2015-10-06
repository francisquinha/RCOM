DESTPATH='/home/ubfeup/Área de Trabalho/tmp1/comp/'
CC=gcc
CFLAGS=-Wall
EXECUTABLE=rcom_pratica_1

all:noncanonical writenoncanonical clean_o

writenoncanonical: writenoncanonical.o Protocol.o
	$(CC) $(addprefix $(DESTPATH),$?) -o $(DESTPATH)$@ $(CFLAGS)

writenoncanonical.o: writenoncanonical.c Protocol.h utilities.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@

noncanonical: noncanonical.o Protocol.o
	$(CC) $(addprefix $(DESTPATH),$?)  -o $(DESTPATH)$@ $(CFLAGS)

noncanonical.o: noncanonical.c Protocol.h utilities.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@
	
Protocol.o: Protocol.c Protocol.h utilities.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@

clean_o:
	-rm $(DESTPATH)*.o

clean:
	-rm $(DESTPATH)*.o
	-rm $(DESTPATH)noncanonical
	-rm $(DESTPATH)writenoncanonical
