DESTPATH='./'
CC=gcc
CFLAGS=-D_REENTRANT -lrt -lpthread -Wall

all:downloader clean_o

downloader: downloader.o ftp.o socket.o
	$(CC) $(addprefix $(DESTPATH),$?) -o $(DESTPATH)$@ $(CFLAGS)

downloader.o: downloader.c utilities.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@

ftp.o: ftp.c ftp.h utilities.h socket.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@

socket.o: socket.c socket.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@
	
clean_o:
	-rm $(DESTPATH)*.o

clean:
	-rm $(DESTPATH)*.o
	-rm $(DESTPATH)downloader
