#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <termios.h>
#include <unistd.h>
#include <string.h>

#include "utilities.h"
#include "Protocol.h"

//X=X=X=X   general debug stuff
#if(1)
//===============================================================================

#define DEBUG_READ_BYTES 	1
	int total_read; /*variable used for debug purposes, increment when reading bytes*/
	
#define DEBUG_PRINT_SECTION_NUM 1
#endif//==========================================================================

//X=X=X=X   basic definitions
#if(1)
//===============================================================================

//#define BAUDRATE B38400
//#define MODEMDEVICE "/dev/ttyS1"
//#define _POSIX_SOURCE 1 /* POSIX compliant source */

struct linkLayer link_layer_data; /*contains data related to the link layer*/

struct termios oldtio, newtio; /*termios old and new configuration*/
//int private_tio_fd;			   /*termios file descriptor*/

int OPENED_TERMIOS = FALSE; /*indicates if termios is open. could be used in handlers if anything goes wrong*/

volatile int STOP = FALSE; /*used in cycles that could be interrupted from another thread or interrupt*/
//int CONECTION_TYPE;/*emissor or receptor*/

#endif//==========================================================================

//X=X=X=X   handlers related
#if(1)//==========================================================================

//#define TIMEOUTS_ALLOWED 3
//int timeout_inseconds;
volatile int timeouts_done;

void timeout_alarm_handler()                   // atende alarme
{
	printf("\nalarme # %d\n", timeouts_done);
	timeouts_done++;
	STOP = TRUE;
}
#endif//==========================================================================

//X=X=X=X   tramas
#if(1)
//===============================================================================

#define FLAG 0b01111110 // FLAG no inicio e fim da trama
#define ATRANS 0b00000011 // A se for o emissor a enviar a trama e o receptor a responder
#define ARECEI 0b00000001 // A se for o receptor a enviar a trama e o emissor a responder
#define SET 0b00000111 // C se for uma trama de setup
#define DISC 0b00001011	// C se for uma trama de disconnect
#define UA 0b00000011 // C se for uma trama de unumbered acknowledgement
#define RR0 0b00100001 // RR se R = 0, pede mensagem seguinte, com R = 1
#define RR1 0b00000001 // RR se R = 1, pede mensagem seguinte, com R = 0
#define REJ0 0b00000101 // REJ se R = 0, pede novamente mensagem com R = 0
#define REJ1 0b00100101 // REJ se R = 1, pede novamente mensagem com R = 1


#define BCC_ON_SET 0b00000100//A TRANS ^ SET
#define BBC_ON_UA 0b00000000//A RECEI ^ UA
const char SET_MSG[5] = { FLAG, ATRANS, SET, BCC_ON_SET, FLAG };
const char UA_MSG[5] = { FLAG, ATRANS, UA, BBC_ON_UA, FLAG };

/* C se for uma trama de positive acknowledgment */
char getRR(int R) {
	if (R == 0) return RR0;
	else return RR1;
}

/* C se for uma trama de negative acknowledgment, R identifica a trama a que estamos a responder */
char getREJ(int R) {
	if (R == 0) return REJ0;
	else return REJ1;
}

/* A depende do sentido da trama original */
char getA(app_status_type status) {
	if (status == APP_STATUS_TRANSMITTER) return ATRANS; // A se for o emissor a enviar a trama e o receptor a responder
	else return ARECEI; // A se for o receptor a enviar a trama e o emissor a responder
}

/* C depende do tipo de trama e, se for positive ou negative acknowledgment, do numero da mensagem R */
char getC(message_type message, int R) {
	if (message == MESSAGE_SET) return SET;
	else if (message == MESSAGE_DISC) return DISC;
	else if (message == MESSAGE_UA) return UA;
	else if (message == MESSAGE_RR) return getRR(R);
	else return getREJ(R);
}

/* BCC1 codigo de verificacao, depende de A e de C: BCC1 = A ^ C (A ou exclusivo C) */
char getBCC1(app_status_type status, message_type message, int R) {
	return getA(status) ^ getC(message,R);
}

/* F | A | C | BCC1 | F */
void getMessage(app_status_type status, message_type message, int R, char* msg) {
	msg[0] = FLAG;
	msg[1] = getA(status);
	msg[2] = getC(message, R);
	msg[3] = getBCC1(status, message, R);
}

#endif//==========================================================================

//X=X=X=X   SET BASICS, OPEN AND CLOSE TERMIOS
#if(1)
//===============================================================================

void set_basic_definitions(int timeout_in_seconds, unsigned int number_of_tries_when_failing, char* port, int baudrate )
{
  DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
  printf("\n-section1-");
  sleep(1);
  );
  
	signal(SIGALRM, timeout_alarm_handler);
	link_layer_data.timeout = timeout_in_seconds;
	link_layer_data.numTransmissions = number_of_tries_when_failing;
	strcpy(link_layer_data.port, port);
	link_layer_data.baudRate=baudrate;
	//link_layer_data.sequenceNumber;
	//link_layer_data.frame;
}

int open_tio(int* tio_fd,int vtime,int vmin)
{
	total_read = 0;

	int private_tio_fd = open(link_layer_data.port, O_RDWR | O_NOCTTY);
	if (private_tio_fd < 0) { perror(link_layer_data.port); exit(-1); }

	if (tcgetattr(private_tio_fd, &oldtio) == -1) { /* save current port settings */
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = link_layer_data.baudRate | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = OPOST;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = vtime;//link_layer_data.timeout * 10;   /* inter-character timer unused */
	newtio.c_cc[VMIN] = vmin;   /* blocking read until X chars received */


	tcflush(private_tio_fd, TCIFLUSH);
	if (tcsetattr(private_tio_fd, TCSANOW, &newtio) == -1) {
		perror("tcsetattr in open_tio");
		exit(-1);
	}

	if (private_tio_fd != 0) *tio_fd = private_tio_fd;
	
	return OK;
}

int close_tio(int tio_fd)
{
	printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
		printf("\nnow sleeping for 2 seconds before closing conection...\n");
	sleep(2);//just a precaution

	printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
	printf("\ntotal: %d\nattemp to close conection...", total_read);
	if (tcsetattr(/*private_*/tio_fd, TCSANOW, &oldtio) == -1) {
		perror("\ntcsetattr in close_tio");
		close(/*private_*/tio_fd);
		exit(-1);
	}
	close(/*private_*/tio_fd);
	printf("\nconection closed without problems.");
	printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
	return OK;
}

#endif//==========================================================================

//X=X=X=X   PROTOCAL AUX FUNCS
#if(1)
//================================================================================

typedef char state_machine_state;
#define STATE_MACHINE_START		1
#define STATE_MACHINE_FLAG_RCV	2
#define STATE_MACHINE_A_RCV		3
#define STATE_MACHINE_C_RCV		4
#define STATE_MACHINE_BCC_RCV	5
#define STATE_MACHINE_STOP		6//UA or SET RECEIVED
#define STATE_MACHINE_INFO		7
#define STATE_MACHINE_BCC2		8
#define STATE_MACHINE_INFO_RCV	9
#define STATE_MACHINE_ACK		10
#define STATE_MACHINE_NACK		10


#define DEBUG_LLO_STATE_MACHINE 1
int update_state_machine(app_status_type status, state_machine_state* state, char rcv)
{
	DEBUG_SECTION(DEBUG_LLO_STATE_MACHINE,
		printf("\nstatus:%d  state:%d  rcv:" PRINTBYTETOBINARY, status,*state, BYTETOBINARY(rcv));
		);

	if (*state < STATE_MACHINE_BCC_RCV)
	{
		if (rcv == FLAG)
		{
			*state = STATE_MACHINE_FLAG_RCV;
			return OK;
		}
		else
		{
			switch (*state)
			{
			case STATE_MACHINE_START:return OK;/*flag checked before switch*/
			case STATE_MACHINE_FLAG_RCV: if (rcv == getA(APP_STATUS_TRANSMITTER) *state += 1;  else *state = STATE_MACHINE_START; return OK;
			case STATE_MACHINE_A_RCV: if (rcv == getC(status == APP_STATUS_TRANSMITTER ? MESSAGE_UA : MESSAGE_SET, 0)) *state += 1; else *state = STATE_MACHINE_START; return OK;
			case STATE_MACHINE_C_RCV: if (rcv == getBCC1(APP_STATUS_TRANSMITTER, status == APP_STATUS_TRANSMITTER ? MESSAGE_UA : MESSAGE_SET, 0))  *state += 1; else *state = STATE_MACHINE_START; return OK;

			default:
				printf("\nWARNING(usm2):Not valid/expected state (%d) reached in --> int state_machine(app_status_type status) => from => Protocol.c\n", *state);
				return 1;
			}
		}
	}
	else if (*state == STATE_MACHINE_BCC_RCV && rcv == FLAG) {
		*state = STATE_MACHINE_STOP; return OK;
	}
	
	return 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define DEBUG_LLOPEN_RECEIVER_OVERFLOW 1
int llopen_receiver(int fd)
{
	STOP = FALSE;/*doesn't do anything right now. could define timeout later 4 receiver*/
	state_machine_state state = STATE_MACHINE_START;
	char buf[20];//used to receive stuff
	int res;//number of bytes read or sent
	//- - - - - - - - - -
	//receive SET
	while (state != STATE_MACHINE_STOP && STOP==FALSE)
	{
		res = read(fd, buf, 20);

		DEBUG_SECTION(DEBUG_LLOPEN_RECEIVER_OVERFLOW,
			if (res > 5)
				printf("\nWARNING:llopen_receiver received more than 5 characters at 1 time\n");
		);

		int i = 0;
		for (; i < res && state != STATE_MACHINE_STOP; ++i)
			update_state_machine(APP_STATUS_RECEIVER,&state, buf[i]);

		DEBUG_SECTION(DEBUG_LLOPEN_RECEIVER_OVERFLOW,
			if (res > i)
				printf("\nWARNING:llopen_receiver received more characters than expected\n");
		);
	}
	//- - - - - - - - - -
	//send UA
	/*how does the receiver knows sender received UA? is verefied in next state and then comes back to receive set mode if failed?
	maybe should i have a "transition read"(shrodinger like) that can still respond UAs?*/
	char[4] msg;
	getMessage(APP_STATUS_TRANSMITTER, MESSAGE_UA, 0, &msg);
	res = write(fd,msg,4);
	res = write(fd,msg,1);

	return OK;
}

#define DEBUG_LLOPEN_TRANSMITTER_OVERFLOW 1
//should send set and receive UA
int llopen_transmitter(int fd)
{
	int res;
	state_machine_state state = STATE_MACHINE_START;
	timeouts_done = 0;
	char buf[20];//used to receive stuff
	STOP = FALSE;
	
	  DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
  printf("\n-section2-");
  sleep(1);
  );
	
	while (state != STATE_MACHINE_STOP && timeouts_done<link_layer_data.numTransmissions)
	{
		char[4] msg;
		getMessage(APP_STATUS_TRANSMITTER, MESSAGE_SET, 0, &msg);
		res = write(fd,msg,4);
		res = write(fd,msg,1);

		STOP = FALSE;//to be set to true by the alarm
		alarm(link_layer_data.timeout);
	
		  DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
  printf("\n-section3-");
  sleep(1);
  );
		
		while (STOP==FALSE)
		{
			res = read(fd, buf, 20);

			  DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
  printf("\n-section4-");
  sleep(1);
  );
			
			DEBUG_SECTION(DEBUG_LLOPEN_TRANSMITTER_OVERFLOW,
				if (res > 5)
					printf("\nWARNING:llopen_transmitter received more than 5 characters at 1 time\n");
			);

			int i = 0;
			for (; i < res && state != STATE_MACHINE_STOP; ++i)
				update_state_machine(APP_STATUS_TRANSMITTER,&state, buf[i]);

			if (state == STATE_MACHINE_STOP) break;

			DEBUG_SECTION(DEBUG_LLOPEN_TRANSMITTER_OVERFLOW,
				if (res > i)
					printf("\nWARNING:llopen_transmitter received more characters than expected\n");
			);
		}

	}

	//printf("\n%d bytes written.", res);
	if (state != STATE_MACHINE_STOP) printf("No confirmation of the reception was received!\n");
	else printf("Reception was confirmed.\n");

	return OK;
}

#endif//==========================================================================

//X=X=X=X   PROTOCAL MAIN FUNCS
#if(1)
//===============================================================================

int llopen(int fd, app_status_type status)
{
	if (status == APP_STATUS_TRANSMITTER) return llopen_transmitter(fd);
	else if (status == APP_STATUS_RECEIVER) return llopen_receiver(fd);
	else
	{
		printf("\nWARNING(llo):invalid app_status found on llopen().\n");
		return 1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int llread(int fd)
{
	STOP = FALSE;/*doesn't do anything right now. could define timeout later 4 receiver*/
	state_machine_state state = STATE_MACHINE_START;
	char buf[20];//used to receive stuff
	int res;//number of bytes read or sent
	//- - - - - - - - - -
	//receive stuff
	while (TRUE)
	{
		res = read(fd, buf, 20);

/*		DEBUG_SECTION(DEBUG_LLOPEN_RECEIVER_OVERFLOW,
			if (res > 5)
				printf("\nWARNING:llopen_receiver received more than 5 characters at 1 time\n");
		);
*/
		int i = 0;
		for (; i < res && state != STATE_MACHINE_STOP; ++i)
			update_state_machine(APP_STATUS_RECEIVER,&state, buf[i]);
		char[4] msg;
		if (state == STATE_MACHINE_STOP) {
			getMessage(APP_STATUS_TRANSMITTER, MESSAGE_UA, 0, &msg);
			res = write(fd,msg,4);
			res = write(fd,msg,1);
		}
		else {
			getMessage(APP_STATUS_TRANSMITTER, )
		}

		DEBUG_SECTION(DEBUG_LLOPEN_RECEIVER_OVERFLOW,
			if (res > i)
				printf("\nWARNING:llopen_receiver received more characters than expected\n");
		);
	}
	//- - - - - - - - - -
	//send UA
	/*how does the receiver knows sender received UA? is verefied in next state and then comes back to receive set mode if failed?
	maybe should i have a "transition read"(shrodinger like) that can still respond UAs?*/

	return OK;
}

#define DEBUG_LLOPEN_TRANSMITTER_OVERFLOW 1
//should send set and receive UA
int llopen_transmitter(int fd)
{
	int res;
	state_machine_state state = STATE_MACHINE_START;
	timeouts_done = 0;
	char buf[20];//used to receive stuff
	STOP = FALSE;
	
	  DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
  printf("\n-section2-");
  sleep(1);
  );
	
	while (state != STATE_MACHINE_STOP && timeouts_done<link_layer_data.numTransmissions)
	{
		char[4] msg;
		getMessage(APP_STATUS_TRANSMITTER, MESSAGE_SET, 0, &msg);
		res = write(fd,msg,4);
		res = write(fd,msg,1);

		STOP = FALSE;//to be set to true by the alarm
		alarm(link_layer_data.timeout);
	
		  DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
  printf("\n-section3-");
  sleep(1);
  );
		
		while (STOP==FALSE)
		{
			res = read(fd, buf, 20);

			  DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
  printf("\n-section4-");
  sleep(1);
  );
			
			DEBUG_SECTION(DEBUG_LLOPEN_TRANSMITTER_OVERFLOW,
				if (res > 5)
					printf("\nWARNING:llopen_transmitter received more than 5 characters at 1 time\n");
			);

			int i = 0;
			for (; i < res && state != STATE_MACHINE_STOP; ++i)
				update_state_machine(APP_STATUS_TRANSMITTER,&state, buf[i]);

			if (state == STATE_MACHINE_STOP) break;

			DEBUG_SECTION(DEBUG_LLOPEN_TRANSMITTER_OVERFLOW,
				if (res > i)
					printf("\nWARNING:llopen_transmitter received more characters than expected\n");
			);
		}

	}

	//printf("\n%d bytes written.", res);
	if (state != STATE_MACHINE_STOP) printf("No confirmation of the reception was received!\n");
	else printf("Reception was confirmed.\n");

	return OK;
}


int llwrite()
{
	return OK;
}

int llread()
{
	return OK;
}

int llclose()
{
	return OK;
}

#endif//==========================================================================