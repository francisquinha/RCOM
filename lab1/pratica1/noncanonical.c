/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pratica1framework.h"
/*
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source 
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;
*/

int main(int argc, char** argv)
{
    char buf[256];

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
   
    open_tio(0,argv[1],0,1);
    printf("New termios structure set\n");

    read_message(buf);

    //just in case... sleeping for 1 second so the writer can start reading...
    sleep(1);
    
    printf("\nDigit Message to send:\n");
    gets(buf);
    int lenght2send = strlen(buf) + 1;
    write_message(buf, lenght2send);
    
    close_tio();

    return 0;
}
