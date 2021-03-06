/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utilities.h"
#include "user_interface.h"
#include "DataLinkProtocol.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source 
/*#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;
*/

//=======================================================================
// PROGRAM VARIABLES
//=======================================================================

struct applicationLayer {
	int fd; /*Descritor correspondente � porta s�rie*/
	int status; /*TRANSMITTER 0 | RECEIVER 1*/
};

struct applicationLayer app;

//=======================================================================
// PROGRAM FUNCS
//=======================================================================

void connect()
{
      if(open_tio(&app.fd,0,0)!=OK)
    {
      printf("\nERROR:Couldnot open terminal\n");
      exit(1);
    }
  
  
      llopen( app.fd , APP_STATUS_RECEIVER);

      //send or read missing
      


}

void config(char baud, char recon, char timeo, int frame)
{
	int baudrate = -1;
	switch (baud) {
	case 'a':
		baudrate = B0; break;
	case 'b':
		baudrate = B50; break;
	case 'c':
		baudrate = B75; break;
	case 'd':
		baudrate = B110; break;
	case 'e':
		baudrate = B134; break;
	case 'f':
		baudrate = B150; break;
	case 'g':
		baudrate = B200; break;
	case 'h':
		baudrate = B300; break;
	case 'i':
		baudrate = B600; break;
	case 'j':
		baudrate = B1200; break;
	case 'k':
		baudrate = B1800; break;
	case 'l':
		baudrate = B2400; break;
	case 'm':
		baudrate = B4800; break;
	case 'n':
		baudrate = B9600; break;
	case 'o':
		baudrate = B19200; break;
	case 'p':
		baudrate = B38400; break;
	case 'q':
		baudrate = B57600; break;
	case 'r':
		baudrate = B115200; break;
	case 's':
		baudrate = B230400; break;
	case 't':
		baudrate = B460800; break;
	default: break;
	}

	int reconect_tries=-1;
	switch (recon) {
	case 'a':
		reconect_tries = 0; break;
	case 'b':
		reconect_tries = 1 ; break;
	case 'c':
		reconect_tries = 3; break;
	case 'd':
		reconect_tries = 5; break;
	case 'e':
		reconect_tries = 7; break;
	default: break;
	}

	int timeout=-1;
	switch (timeo) {
	case 'a':
		timeout = 2; break;
	case 'b':
		timeout = 3; break;
	case 'c':
		timeout = 5; break;
	case 'd':
		timeout = 8; break;
	default: break;
	}
	
	set_basic_definitions(timeout, reconect_tries, 0, baudrate);

}

void testread()
{
	char *receive;
	int rec_size = llread(app.fd, &receive);
	
	char endchar = receive[rec_size - 1];
	//receive[rec_size - 1] = 0;
	printf("\n");
	//printf("%s%c - %d", receive, endchar, rec_size);test string
	int i=0;
	for(;i<rec_size;++i)
	printf(PRINTBYTETOBINARY " - " , BYTETOBINARY(receive[i]));
	printf("%d",rec_size);
	gets(receive);
	free(receive);
}

//=======================================================================
// MAIN
//=======================================================================

int main(int argc, char** argv)
{
  app.status = 1;
  //char buf[256];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS4", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0\n");
      exit(1);
    }

        set_basic_definitions(3, 3, argv[1], BAUDRATE);
    
    char anws=' ';
    	while (anws != 'e'){
	   anws=main_menu(APP_STATUS_RECEIVER);
	  		switch (anws){

		case 'a':
			connect();
			testread();
			close_tio(app.fd);
			break;
		case 'b':printf("\nNOT IMPLEMENTED");//reconnect();
			break;
		case 'c':select_config(config);
			break;
		case 'd': printf("\nNOT IMPLEMENTED");//if (receiver != 0) choosePicture2Send();
			break;
		case 'e':printf("\nNow exiting...\n"); sleep(1);
			break;
		default: printf("\nNo valid command recognised."); sleep(1); break;
			}
	}

    return 0;
}
