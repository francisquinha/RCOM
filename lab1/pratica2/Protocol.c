#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <termios.h>
#include <unistd.h>
#include <string.h>

#include "utilities.h"

//===============================================================================
//debug stuff
//===============================================================================

#define DEBUG_READ_BYTES 1
debug_section(DEBUG_READ_BYTES,
	int total_read; /*variable used for debug purposes, increment when reading bytes*/
)

//===============================================================================
//basic definitions
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

void set_basic_definitions(unsigned int timeout_in_seconds, unsigned int number_of_tries_when_failing, char* port, int boudrate )
{
	signal(SIGALRM, timeout_alarm_handler);
	link_layer_data.timeout = timeout_inseconds;
	link_layer_data.numTransmissions = number_of_tries_when_failing;
	link_layer_data.port = strcpy(port, port);
	link_layer_data.baudRate=baudrate;
	//link_layer_data.sequenceNumber;
	//link_layer_data.frame;
}

//===============================================================================
//handlers related
//===============================================================================

//#define TIMEOUTS_ALLOWED 3
//int timeout_inseconds;
volatile int timeouts_done;

void timeout_alarm_handler()                   // atende alarme
{
	printf("alarme # %d\n", timeouts_done);
	timeouts_done++;
}


//===============================================================================
//tramas
//===============================================================================

#define FLAG 0b01111110 // FLAG no inicio e fim da trama
#define ATRANS 0b00000011 // A se for o emissor a enviar a trama e o receptor a responder
#define ARECEI 0b00000001 // A se for o receptor a enviar a trama e o emissor a responder
#define SET 0b00000111 // C se for uma trama de setup
#define DISC 0b00001011	// C se for uma trama de disconnect
#define UA 0b00000011 // C se for uma trama de unumbered acknowledgement
#define RR0 0b00000001 // RR se R = 0 // nao tenho a certeza disto, nao sei se o R age desta forma ou ao contrario
#define RR1 0b00100001 // RR se R = 1 // nao tenho a certeza disto, nao sei se o R age desta forma ou ao contrario
#define REJ0 0b00000101 // RR se R = 0 // nao tenho a certeza disto, nao sei se o R age desta forma ou ao contrario
#define REJ1 0b00100101 // RR se R = 1 // nao tenho a certeza disto, nao sei se o R age desta forma ou ao contrario

/* C se for uma trama de positive acknowledgment */
int getRR(int R) {
	if (R == 0) return RR0;
	else return RR1;
}

/* C se for uma trama de negative acknowledgment, R identifica a trama a que estamos a responder */
int getREJ(int R) {
	if (R == 0) return REJ0;
	else return REJ1;
}

/* A depende do sentido da trama original */
int getA(app_status_type status) {
	if (status == APP_STATUS_TRANSMITTER) return ATRANS; // A se for o emissor a enviar a trama e o receptor a responder
	else return ARECEI; // A se for o receptor a enviar a trama e o emissor a responder
}

/* C depende do tipo de trama e, se for positive ou negative acknowledgment, do numero da mensagem R */
int getC(message_type message, int R) {
	if (message == MESSAGE_SET) return SET;
	else if (message == MESSAGE_DISC) return DISC;
	else if (message == MESSAGE_UA) return UA;
	else if (message == MESSAGE_RR) return getRR(R);
	else return getREJ(R);
}

/* BCC1 codigo de verificacao, depende de A e de C: BCC1 = A ^ C (A ou exclusivo C) */
int getBCC1(app_status_type status, int set_disc_ua_rr_rej, int R) {
	return getA(status) ^ getC();
}

/* F | A | C | BCC1 | F */
int getTrama(app_status_type status, int set_disc_ua_rr_rej, int R) {
	return FLAG | getA(status) | getC(set_disc_ua_rr_rej, R) | getBCC1(status, set_disc_ua_rr_rej, R) | FLAG;
}


//===============================================================================
//OPEN AND CLOSE TERMIOS
//===============================================================================

int open_tio(int* tio_fd, char* serial_port, int vmin)
{
	total_read = 0;

	private_tio_fd = open(link_layer_data.baudrate.port, O_RDWR | O_NOCTTY);
	if (private_tio_fd < 0) { perror(serial_port); exit(-1); }

	if (tcgetattr(private_tio_fd, &oldtio) == -1) { /* save current port settings */
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = link_layer_data.baudrate | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = OPOST;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = link_layer_data.baudrate.timeout * 10;   /* inter-character timer unused */
	newtio.c_cc[VMIN] = vmin;   /* blocking read until X chars received */


	tcflush(private_tio_fd, TCIFLUSH);
	if (tcsetattr(private_tio_fd, TCSANOW, &newtio) == -1) {
		perror("tcsetattr in open_tio");
		exit(-1);
	}

	//if (tio_fd != 0) *tio_fd = private_tio_fd;

	return OK;
}

int close_tio(int tio_fd)
{
	printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -")
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


//===============================================================================
//PROTOCAL AUX FUNCS
//===============================================================================

typedef char state_machine_state;
#define STATE_MACHINE_START		1
#define STATE_MACHINE_FLAG_RCV	2
#define STATE_MACHINE_A_RCV		3
#define STATE_MACHINE_C_RCV		4
#define STATE_MACHINE_BCC_RCV	5
#define STATE_MACHINE_STOP		6

/*subtituir XPTOs por char respectivo*/
#define XPTO_FLAG 0
#define XPTO 0 

int update_state_machine_receiver(state_machine_state* state, char rcv)
{
	if (*state < STATE_MACHINE_BCC_RCV)
	{
		if (rcv == XPTO_FLAG)
		{
			*state = STATE_MACHINE_FLAG_RCV;
			return OK;
		}
		else
		{
			switch (*state)
			{

			STATE_MACHINE_FLAG_RCV: if (rcv == XPTO) *state += 1;  else *state = STATE_MACHINE_START; return OK;
			STATE_MACHINE_A_RCV:	if (rcv == XPTO) *state += 1; else *state = STATE_MACHINE_START; return OK;
			STATE_MACHINE_C_RCV:	if (rcv == XPTO) *state += 1; else *state = STATE_MACHINE_START; return OK;

			default:
				printf("\nWARNING(usm2):Not valid/expected state (%d) reached in --> int state_machine(app_status_type status) => from => Protocol.c\n", *state);
				return 1;
			}
		}
	}
	else if (*state == STATE_MACHINE_BCC_RCV && rcv == XPTO) {
		*state = STATE_MACHINE_STOP; return OK;
	}
}

int update_state_machine_transmitter(state_machine_state* state, char rcv)
{
		if (*state < STATE_MACHINE_BCC_RCV)
		{
			if (rcv == XPTO_FLAG)
			{
				*state = STATE_MACHINE_FLAG_RCV;
				return OK;
			}
			else
			{
				switch (*state)
				{

				STATE_MACHINE_FLAG_RCV: if (rcv == XPTO) *state += 1;  else *state = STATE_MACHINE_START; return OK;
				STATE_MACHINE_A_RCV:	if (rcv == XPTO) *state += 1; else *state = STATE_MACHINE_START; return OK;
				STATE_MACHINE_C_RCV:	if (rcv == XPTO) *state += 1; else *state = STATE_MACHINE_START; return OK;

				default:
					printf("\nWARNING(usm1):Not valid/expected state (%d) reached in --> int state_machine(app_status_type status) => from => Protocol.c\n",*state);
					return 1;
				}
			}
		} 
		else if (*state == STATE_MACHINE_BCC_RCV && rcv == XPTO) {
			*state = STATE_MACHINE_STOP; return OK;
		}	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int llopen_receiver(int fd)
{

	return OK;
}

//should send set and receive UA
int llopen_transmitter(int fd)
{
	state_machine_state state = STATE_MACHINE_START;
	timeouts_done = 0;
	STOP = FALSE;
	while (STOP == FALSE && timeouts_done<TIMEOUTS_ALLOWED)
	{
		int sent = write(private_tio_fd,message,message_lenght);//??? ??? ???
		if (sent != XPTO);//??? ??? ???

		//receive UA
		alarm(timeout_inseconds);
		//receive UA missing... receives everything or some parts(doing many reads) and decomposes , or reads 1 by 1 with vmin =1?
		//could try to implement both ways, not sure how 2 go about it
		//update_state_machine_transmitter(&state, rcv);
		//if(state==STATE_MACHINE_STOP) STOP==TRUE;
	}

	printf("%d bytes written.", res);
	if (STOP == FALSE) printf("No confirmation of the reception was received!\n");
	else printf("Reception was confirmed.\n");

	return OK;
}

//===============================================================================
//PROTOCAL MAIN FUNCS
//===============================================================================

int llopen(int fd, app_status_type status)
{
	if (status == APP_STATUS_TRANSMITTER) return llopen_transmitter(fd);
	else if (status == APP_STATUS_RECEIVER) return llopen_receiver(fd);
	else
	{
		printf("\nWARNING(llo):invalid app_status found on llopen().\n")
		return 1;
	}
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