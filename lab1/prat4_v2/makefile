DESTPATH='./'
CC=gcc
CFLAGS=-Wall

all:noncanonical writenoncanonical clean_o

writenoncanonical: writenoncanonical.o Protocol.o user_interface.o
	$(CC) $(addprefix $(DESTPATH),$?) -o $(DESTPATH)$@ $(CFLAGS)

writenoncanonical.o: writenoncanonical.c Protocol.h utilities.h user_interface.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@

noncanonical: noncanonical.o Protocol.o user_interface.o
	$(CC) $(addprefix $(DESTPATH),$?)  -o $(DESTPATH)$@ $(CFLAGS)

noncanonical.o: noncanonical.c Protocol.h utilities.h user_interface.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@
	
Protocol.o: Protocol.c Protocol.h utilities.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@

user_interface.o: user_interface.c user_interface.h utilities.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@

clean_o:
	-rm $(DESTPATH)*.o

clean:
	-rm $(DESTPATH)*.o
	-rm $(DESTPATH)noncanonical
	-rm $(DESTPATH)writenoncanonical
