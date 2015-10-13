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
#define RR0 0b00100001 // C se RR se R = 0, pede mensagem seguinte, com R = 1
#define RR1 0b00000001 // C se RR se R = 1, pede mensagem seguinte, com R = 0
#define REJ0 0b00000101 // C se REJ se R = 0, pede novamente mensagem com R = 0
#define REJ1 0b00100101 // C se REJ se R = 1, pede novamente mensagem com R = 1
#define I0 0b00000000 //C se I se S = 0
#define I1 0b00100000 //C se I se S = 1
#define ESCAPE 0b01111101 //usado no stuffing e destuffing

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

/* C se for uma trama de I, S  */
char getI(int S) {
	if (S == 0) return I0;
	else return I1;
}

/* A depende do sentido da trama original */
char getA(app_status_type status) {
	if (status == APP_STATUS_TRANSMITTER) return ATRANS; // A se for o emissor a enviar a trama e o receptor a responder
	else return ARECEI; // A se for o receptor a enviar a trama e o emissor a responder
}

/* C depende do tipo de trama e, se for positive ou negative acknowledgment, do numero da mensagem R (S se for I)*/
char getC(message_type message, int R) {
	if (message == MESSAGE_SET) return SET;
	else if (message == MESSAGE_DISC) return DISC;
	else if (message == MESSAGE_UA) return UA;
	else if (message == MESSAGE_I) return getI(R);
	else if (message == MESSAGE_RR) return getRR(R);
	else return getREJ(R);
}

/* BCC1 codigo de verificacao, depende de A e de C: BCC1 = A ^ C (A ou exclusivo C) */
char getBCC1(app_status_type status, message_type message, int R) {
	return getA(status) ^ getC(message, R);
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
bool port_name_was_set = NO;
void set_basic_definitions(int timeout_in_seconds, int number_of_tries_when_failing, char* port, int baudrate)
{
	DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
		printf("\n-section1-");
	sleep(1);
	);

	signal(SIGALRM, timeout_alarm_handler);
	link_layer_data.timeout = timeout_in_seconds;
	link_layer_data.numTransmissions = number_of_tries_when_failing;
	if (port_name_was_set == NO) { strcpy(link_layer_data.port, port); port_name_was_set = YES; }
	link_layer_data.baudRate = baudrate;
	//link_layer_data.sequenceNumber;
	//link_layer_data.frame;
}

int open_tio(int* tio_fd, int vtime, int vmin)
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

typedef int state_machine_state;
#define STATE_MACHINE_START			1
#define STATE_MACHINE_FLAG_RCV		2
#define STATE_MACHINE_A_RCV			3
#define STATE_MACHINE_C_RCV			4
#define STATE_MACHINE_BCC_RCV		5
#define STATE_MACHINE_STOP			6
#define STATE_MACHINE_ESCAPE		7
/*
#define STATE_MACHINE_INFO_RCV		9
#define STATE_MACHINE_C_RCV_NACK	13
#define STATE_MACHINE_NOTVALID_I	14*/

/*TODO !!! FALTA  anotar contagem de erros para as estatisticas e/ou debug*/
/*msgExpectedType SET; UA ; DISC ; RR ; I (DONT USE REJ IT WILL BE CONSIDERED WHEN USING RR)
!!! !!! !!! IMPORTANT: AFTER REACHING TERMINAL STATES THE STATE MUST BE RESETED FROM OUTSIDE !!! !!! !!!
TERMINAL STATES ARE ==> STATE_MACHINE_STOP ; STATE_MACHINE_INFO_RCV ; STATE_MACHINE_NOTVALID_I

appStatus	 indica o tipo de estado da aplicação (transmissor ou receptor)
adressStatus indica o tipo de campo de endereço
state		 o estado actual a atualizar
msgExpectedType		 o tipo de mensaem que se esta a espera de receber (antes de receber o C)
rcv			 o que caracter que foi lido da porta serie

SO VAI TRANSITAR SE APANHAR DO TIPO ESPERADO,
EXCEPTO O READ Q PODE APANHAR SETS
E O WRITE PODE APANHAR REJ MSM ESPERANDO RRs

use received_C_type to confirm the type of packet received
*/
int R_S = 1;//to be used as the S or R bit, not yet implemented
message_type received_C_type = -1;//not sure if the most correct approach but simplifies the state machine a lot, indicates type of message received and is reused outside ethod when _STOP state is reached
#define DEBUG_LLO_STATE_MACHINE 1
int update_state_machine(app_status_type appStatus, app_status_type adressStatus, message_type msgExpectedType, char rcv, state_machine_state* state)
{
	DEBUG_SECTION(DEBUG_LLO_STATE_MACHINE,
		printf("\nappstatus:%d  tramastatus(A):%d  msgtype:%d   state:%d  rcv:" PRINTBYTETOBINARY, appStatus, adressStatus, msgExpectedType, *state, BYTETOBINARY(rcv));
	);

	if (*state < STATE_MACHINE_BCC_RCV)
	{
		if (rcv == FLAG)
		{
			*state = STATE_MACHINE_FLAG_RCV;
			return OK;
		}
	}

	switch (*state)
	{
	case STATE_MACHINE_START:return OK;/*flag checked before switch*/

	case STATE_MACHINE_FLAG_RCV:
		if (rcv == getA(adressStatus)) *state = STATE_MACHINE_A_RCV;
		else *state = STATE_MACHINE_START;
		return OK;

	case STATE_MACHINE_A_RCV:
		if (msgExpectedType == MESSAGE_I)//no READ c/ tramas I pra apanhar possiveis sets
		{
			if (rcv == getC(MESSAGE_I, R_S)) { *state = STATE_MACHINE_C_RCV;  received_C_type = MESSAGE_I; }
			/*catch set on read()*/else if (appStatus == APP_STATUS_RECEIVER && rcv == getC(MESSAGE_SET, 0)) { *state = STATE_MACHINE_C_RCV; received_C_type = MESSAGE_SET; }
			else *state = STATE_MACHINE_START;
		}
		//WRITE  RR/REJ
		else if (msgExpectedType == MESSAGE_RR || msgExpectedType == MESSAGE_REJ)
		{
			if (rcv == getC(MESSAGE_RR, R_S)) { *state = STATE_MACHINE_C_RCV;  received_C_type = MESSAGE_RR; }
			else if (rcv == getC(MESSAGE_REJ, R_S)) { *state = STATE_MACHINE_C_RCV;  received_C_type = MESSAGE_REJ; }
			else *state = STATE_MACHINE_START;
		}
		//SET,GET or DISC
		else  if (rcv == getC(msgExpectedType, 0)) {
			*state = STATE_MACHINE_C_RCV;
			received_C_type = msgExpectedType;
		}
		else *state = STATE_MACHINE_START;

		return OK;

	case STATE_MACHINE_C_RCV:
		//note:C type already received so it is a valid msg type (if it wasn't state would go back to START) unless we have an error in which bcc will hopefully fail
		if (rcv == getBCC1(adressStatus, received_C_type, 0)) *state = STATE_MACHINE_BCC_RCV;
		else *state = STATE_MACHINE_START;
		return OK;

	case STATE_MACHINE_BCC_RCV://also goes 2 this state after STATE_MACHINE_ESCAPE
		if (received_C_type == MESSAGE_I && rcv == ESCAPE) *state = STATE_MACHINE_ESCAPE;
		if (rcv == FLAG) *state = STATE_MACHINE_STOP;
		else *state = STATE_MACHINE_START;

	case STATE_MACHINE_ESCAPE://could be done outside but makes more sense to be here
		*state = STATE_MACHINE_BCC_RCV;
		return OK;

	default:
		printf("\nWARNING(usm2):Not valid/expected state (%d) reached in --> int state_machine(app_status_type status) => from => Protocol.c\n", *state);
		return 1;
	}

	return 1;
}



/*estou a admitir que buf tem sempre espaço suficiente, podemos alterar pra usar reallocs com arrays dinamicos
da forma como esta temos que garantir que o array tem tamanho 2x que o seu preenchimento p/ evitar erros*/
/*int apply_stuffing(char* buf, int size)//size e o "preenchimento", estou a pressupor que sobra espaço!!!
{
int newSize = size;
int i = 0;
for (; i < bufSize; ++i)
if (buf[i] == FLAG || buf[i] == ESCAPE)
{
++newSize;
memmove(buf+i+1, buf+i, size-i);
buf[i] = ESCAPE;
buf[i+1] ^= 0x20;
}

return newSize;
}*/

/*int apply_destuffing(char* buf, int size)
{
	int newSize = size;
	int i = 0;
	for (; i < bufSize; ++i)
		if ( buf[i] == ESCAPE)
		{
			--newSize;
			memmove(buf + i , buf + i +1, size-i-1);
			buf[i] ^= 0x20;
		}

	return newSize;
}*/

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
	while (state != STATE_MACHINE_STOP && STOP == FALSE)
	{
		res = read(fd, buf, 20);

		DEBUG_SECTION(DEBUG_LLOPEN_RECEIVER_OVERFLOW,
			if (res > 5)
				printf("\nWARNING:llopen_receiver received more than 5 characters at 1 time\n");
		);

		int i = 0;
		for (; i < res && (state != STATE_MACHINE_STOP || received_C_type != MESSAGE_SET); ++i)
			update_state_machine(APP_STATUS_RECEIVER, APP_STATUS_TRANSMITTER, MESSAGE_SET, buf[i], &state);

		DEBUG_SECTION(DEBUG_LLOPEN_RECEIVER_OVERFLOW,
			if (res > i)
				printf("\nWARNING:llopen_receiver received more characters than expected\n");
		);
	}
	//- - - - - - - - - -
	//send UA
	/*how does the receiver knows sender received UA? is verefied in next state and then comes back to receive set mode if failed?
	maybe should i have a "transition read"(shrodinger like) that can still respond UAs?*/
	char msg[4];
	getMessage(APP_STATUS_TRANSMITTER, MESSAGE_UA, 0, msg);
	res = write(fd, msg, 4);
	res = write(fd, msg, 1);

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

	while (state != STATE_MACHINE_STOP && timeouts_done < link_layer_data.numTransmissions)
	{
		char msg[4];
		getMessage(APP_STATUS_TRANSMITTER, MESSAGE_SET, 0, msg);
		res = write(fd, msg, 4);
		res = write(fd, msg, 1);

		STOP = FALSE;//to be set to true by the alarm
		alarm(link_layer_data.timeout);

		DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
			printf("\n-section3-");
		sleep(1);
		);

		while (STOP == FALSE)
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
			for (; i < res && (state != STATE_MACHINE_STOP || received_C_type != MESSAGE_UA); ++i)
				update_state_machine(APP_STATUS_TRANSMITTER, APP_STATUS_TRANSMITTER, MESSAGE_UA, buf[i], &state);

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
		//UPDATE STATE MACHINE - UPDATE STATE MACHINE - UPDATE STATE MACHINE
		int i = 0;
		for (; i < res && state != STATE_MACHINE_STOP; ++i)
			update_state_machine(APP_STATUS_RECEIVER, APP_STATUS_TRANSMITTER, MESSAGE_RR, buf[i], &state);

		//SEND UA - SEND UA - SEND UA
		char msg[4];
		if (state == STATE_MACHINE_STOP){
			if (received_C_type == MESSAGE_SET)
			{
				getMessage(APP_STATUS_TRANSMITTER, MESSAGE_UA, 0, msg);
				res = write(fd, msg, 4);
				res = write(fd, msg, 1);

			}
			else; //MISSING OTHER STATES TO CATCH INFO
		}

		DEBUG_SECTION(DEBUG_LLOPEN_RECEIVER_OVERFLOW,
			if (res > i)
				printf("\nWARNING:llopen_receiver received more characters than expected\n");
		);
	}
	//- - - - - - - - - -

	return OK;
}


int llwriter(int fd)
{
	/*
	  int res;
	  resetStateMachine();
	  timeouts_done = 0;
	  char buf[20];//used to receive stuff
	  STOP = FALSE;

	  DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
	  printf("\n-section2-");
	  sleep(1);
	  );

	  while (state != STATE_MACHINE_STOP && timeouts_done<link_layer_data.numTransmissions)
	  {
	  char msg[4];
	  getMessage(APP_STATUS_TRANSMITTER, MESSAGE_SET, 0, msg);
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
	  update_state_machine(APP_STATUS_TRANSMITTER, ,MENSAGE_RR ,buf[i], &state);

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
	  */
	return OK;
}


int llclose()
{
	return OK;
}

#endif//==========================================================================