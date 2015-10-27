DESTPATH='./'
CC=gcc
CFLAGS=-D_REENTRANT -lrt -lpthread -Wall

all:App clean_o

App: App.o DataLinkProtocol.o user_interface.o FileFuncs.o AppProtocol.o
	$(CC) $(addprefix $(DESTPATH),$?) -o $(DESTPATH)$@ $(CFLAGS)

App.o: App.c DataLinkProtocol.h utilities.h user_interface.h 
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@
	
DataLinkProtocol.o: DataLinkProtocol.c DataLinkProtocol.h utilities.h 
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@

AppProtocol.o: AppProtocol.c AppProtocol.h DataLinkProtocol.h utilities.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@

user_interface.o: user_interface.c user_interface.h utilities.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@

FileFuncs.o: FileFuncs.c FileFuncs.h utilities.h
	$(CC) -c $*.c $(CFLAGS) -o $(DESTPATH)$@

clean_o:
	-rm $(DESTPATH)*.o

clean:
	-rm $(DESTPATH)*.o
	-rm $(DESTPATH)App
