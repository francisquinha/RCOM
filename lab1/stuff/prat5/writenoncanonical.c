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

struct applicationLayer {
	int fd; /*Descritor correspondente à porta série*/
	int status; /*TRANSMITTER 0 | RECEIVER 1*/
};

bool image_loaded=NO;
char* image_bytes;
long image_bytes_length;


void testfunc()
{
	set_basic_definitions(3, 3, argv[1], BAUDRATE);
	if (open_tio(&app.fd, 0, 0) != OK)
	{
		printf("\nERROR:Couldnot open terminal\n");
		exit(1);
	}

	if (llopen(app.fd, APP_STATUS_TRANSMITTER) == 0)
	{
		sleep(1);
		char samplemsg[5] =//"abcde";
		{ 0b01111101, 0b01111101, 0b01111110, 0b01111110, 0b01111101 };

		if (llwrite(app.fd, samplemsg, 5) > 0) {
			printf("\nDOIN it");
			llclose(app.fd);
		}

		close_tio(app.fd);
}

int main(int argc, char** argv)
{
  struct applicationLayer app;
  app.status = 0;
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


    return 0;
}
