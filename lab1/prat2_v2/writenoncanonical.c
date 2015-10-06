/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Protocol.h"
/*
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source 
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;
*/

struct applicationLayer {
	int fileDescriptor; /*Descritor correspondente à porta série*/
	int status; /*TRANSMITTER 0 | RECEIVER 1*/
};

int main(int argc, char** argv)
{
  	//struct applicationLayer app;
	//app.status = 0;
  
    //char buf[256];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS4", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0\n");
      exit(1);
    }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
   
   	/*
	set_basic_definitions
    open_tio
	llopen
    close_tio
	*/

    return 0;
}
