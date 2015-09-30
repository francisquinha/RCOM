include <stdlib.h>
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

//===============================================================================
//handlers related
//===============================================================================

#define TIMEOUTS_ALLOWED 3
int timeout_inseconds;

timeouts_done;

void timeout_alarm_handler()                   // atende alarme
{
	printf("alarme # %d\n", timeouts_done);
	timeouts_done++;
}


//===============================================================================
//basic definitions
//===============================================================================

int CONECTION_TYPE;

void set_basic_definitions(int con_type)
{
  CONECTION_TYPE = con_type;
   signal(SIGALRM, timeout_alarm_handler); 
}

int pack_message(char data)
{
  if(CONECTION_TYPE==CONECTION_TYPE_WRITER);// & ou | xpto
  else if(CONECTION_TYPE==CONECTION_TYPE_RECEIVER);
  else; //erro
}

//===============================================================================
//terminal comunication related
//===============================================================================

struct termios oldtio,newtio;
int private_tio_fd;
int total_read;

volatile int STOP=FALSE;

//t=vtime*0.1s ... timeout=seconds to time out so 10*timeout=vtime
int open_tio(int* tio_fd,char* serial_port,int timeout, int vmin)
{
  total_read=0;
  timeout_inseconds=timeout;
  
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
    newtio.c_cc[VTIME]    = timeout*10;   /* inter-character timer unused */
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
    printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -")
    printf("\nnow sleeping for 2 seconds before closing conection...\n");
    sleep(2);//just a precaution
  
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
  
  //send message received confirmation
  pack_message();
  //write_message();
  printf("- - - confirmation received - - -\n");
  
  return 0;
}


int write_message(char* message, int message_lenght)
{
  timeouts_done=0;
  STOP=FALSE;
  
  int res;
 
  //repeat message until confirmation or max timouts reached
  while(STOP==FALSE && timeouts_done<TIMEOUTS_ALLOWED)
  {
   res = write(private_tio_fd,message,message_lenght);
   
   //receive confirmation stuff... 
   alarm(3);
   res = read(private_tio_fd,buf,256);   
   if (res>0 ){STOP=TRUE;}
  }
  
  printf("%d bytes written.", res);
  if(STOP==FALSE) printf("No confirmation of the reception was received!\n");
  else printf("Reception was confirmed.\n");

  return 0;
}
