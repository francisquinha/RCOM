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
#include "Protocol.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source 
/*#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;
*/

struct applicationLayer {
	int fileDescriptor; /*Descritor correspondente � porta s�rie*/
	int status; /*TRANSMITTER 0 | RECEIVER 1*/
};

int main(int argc, char** argv)
{
  struct applicationLayer app;
  app.status = 1;
  //char buf[256];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS4", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0\n");
      exit(1);
    }

    set_basic_definitions(3, 3, argv[1], BAUDRATE);
    if(open_tio(&app.fileDescriptor,0,0)!=OK)
    {
      printf("\nERROR:Couldnot open terminal\n");
      exit(1);
    }
    
    llopen( app.fileDescriptor , APP_STATUS_RECEIVER);
    
    close_tio(app.fileDescriptor);


    return 0;
}
