#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <termios.h>
#include <unistd.h>
#include <string.h>

#include "pratica1framework.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

struct termios oldtio,newtio;
int private_tio_fd;
int total_read;

volatile int STOP=FALSE;

int open_tio(int* tio_fd,char* serial_port,int vtime, int vmin)
{
  total_read=0;
    struct termios oldtio,newtio;
    
    if ( 
  	     ((strcmp("/dev/ttyS0", serial_port)!=0) && 
  	      (strcmp("/dev/ttyS4", serial_port)!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0\n");
      exit(1);
    }

    private_tio_fd = open(serial_port, O_RDWR | O_NOCTTY );
    if (private_tio_fd <0) {perror(serial_port); exit(-1); }

    if ( tcgetattr(private_tio_fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME]    = vtime;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = vmin;   /* blocking read until 1 chars received */


    tcflush(private_tio_fd, TCIFLUSH);
    if ( tcsetattr(private_tio_fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr in open_tio");
      exit(-1);
    }
    
    if (tio_fd!=0) *tio_fd = private_tio_fd;
    
    return 0;
}

int close_tio() 
{
    printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
    printf("\ntotal: %d\nattemp to close conection...", total_read);
        if ( tcsetattr(private_tio_fd,TCSANOW,&oldtio) == -1) {
      perror("\ntcsetattr in close_tio");
      close(private_tio_fd);
      exit(-1);
    }
    close(private_tio_fd);
     printf("\nconection closed without problems.");
     printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
     return 0;
}

/* nao contemplei os timeouts, acho que e preciso apanhalos com sinais */
int read_message(char* buf)
{
  STOP=FALSE;
  int res=0;
  printf("\n- - - receiving message - - -\n");
      while (STOP==FALSE) {      
      res = read(private_tio_fd,buf,256);   
      
      if (res>0 && buf[res-1] == 0)
	STOP=TRUE;
      
      buf[res]=0;
      printf("%s",buf);
      total_read += res;
    }
  printf("\n- - - end of received message - - -\n");
  return 0;
}

int write_message(char* message, int message_lenght)
{
  int res = write(private_tio_fd,message,message_lenght);
  printf("%d bytes written\nnow sleeping for 2 seconds...\n", res);
  sleep(2);//just a precaution
  return 0;
}
