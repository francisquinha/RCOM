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
#include "DataLinkProtocol.h"

//X=X=X=X   general debug stuff
#if (1)
//===============================================================================

#define DEBUG_READ_BYTES 	1
int total_read; /*variable used for debug purposes, increment when reading bytes*/

//send a bcc eith 0 to tests the answer when errors occur
bool gen0bcc = TRUE;
#define DEBUG_REJ_WITHWRONG_BCCS 	1

#define DEBUG_PRINT_SECTION_NUM 1
#endif//==========================================================================

//X=X=X=X   basic definitions
#if (1)
//===============================================================================

//#define BAUDRATE B38400
//#define MODEMDEVICE "/dev/ttyS1"
//#define _POSIX_SOURCE 1 /* POSIX compliant source */

struct linkLayer link_layer_data; /*contains data related to the link layer*/

struct termios oldtio, newtio; /*termios old and new configuration*/
//int private_tio_fd;			   /*termios file descriptor*/

struct occurrences_Log occ_log;
occurrences_Log_Ptr get_occurrences_log()
{
	return &occ_log;
}

app_status_type app_status;//indicates i fits transmisser or receiver

int OPENED_TERMIOS = FALSE; /*indicates if termios is open. could be used in handlers if anything goes wrong*/

int NS = 0;
int NR = 0;
//int CONECTION_TYPE;/*emissor or receptor*/

#endif//==========================================================================

//X=X=X=X   ALARM related
#if (1)//==========================================================================

//#define TIMEOUTS_ALLOWED 3
//int timeout_inseconds;
volatile int STOP = FALSE; //STOP s used to repeat a prcedure before the alarm handler rings/*volatile:used in cycles that could be interrupted from another thread or interrupt*/
volatile int timeouts_done;

void timeout_alarm_handler()                   // atende alarme
{
	printf("\nalarme # %d\n", timeouts_done);
	timeouts_done++;
	STOP = TRUE;
	/*update occurrences log*/++occ_log.total_num_of_timeouts;
}


void startAlarm() {
	STOP = FALSE;
	alarm(link_layer_data.timeout);
}

void stopAlarm() {
	STOP = TRUE;
	alarm(0);
}

#endif//==========================================================================

//X=X=X=X   tramas
#if (1)
//===============================================================================

#define FLAG 0b01111110 // FLAG no inicio e fim da trama
#define ATRANS 0b00000011 // A se for o emissor a enviar a trama e o receptor a responder
#define ARECEI 0b00000001 // A se for o receptor a enviar a trama e o emissor a responder
#define SET 0b00000111 // C se for uma trama de setup
#define DISC 0b00001011	// C se for uma trama de disconnect
#define UA 0b00000011 // C se for uma trama de unumbered acknowledgement
#define RR0 0b00000001 // C se RR se S = 1, pede mensagem seguinte, com R = 0
#define RR1 0b00100001 // C se RR se S = 0, pede mensagem seguinte, com R = 1
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
#if (1)
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
	printf("\nnow sleeping for 2 seconds before closing fd...\n");
	sleep(2);//just a precaution

	printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
	printf("\ntotal: %d\nattemp to close fd...", total_read);
	if (tcsetattr(/*private_*/tio_fd, TCSANOW, &oldtio) == -1) {
		perror("\ntcsetattr in close_tio");
		close(/*private_*/tio_fd);
		exit(-1);
	}
	close(/*private_*/tio_fd);
	printf("\nfd closed without problems.");
	printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
	return OK;
}

#endif//==========================================================================

//X=X=X=X   PROTOCAL AUX FUNCS
#if (1)
//================================================================================


//-----   GET MSGS STATE MACHINE - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if (1)

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

appStatus	 indica o tipo de estado da aplicação (transmissor ou receptor)
adressStatus indica o tipo de campo de endereço
state		 o estado actual a atualizar
msgExpectedType		 o tipo de mensaem que se esta a espera de receber (antes de receber o C)
rcv			 o que caracter que foi lido da porta serie

SO VAI TRANSITAR SE APANHAR DO TIPO ESPERADO,
EXCEPTO O READ Q PODE APANHAR SETS
E O WRITE PODE APANHAR REJ MSM ESPERANDO RRs

use received_C_type to confirm the type of packet received or check S/R for possible missing packet
*/
//int R_S = 1;//to be used as the S or R bit, not yet implemented
message_type received_C_type = -1;//not sure if the most correct approach but simplifies the state machine a lot, indicates type of message received and is reused outside ethod when _STOP state is reached
char received_C=0;
#define DEBUG_LLO_STATE_MACHINE 1
#define DEBUG_STATE_MACHINE_GETC 0
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
		if (msgExpectedType == MESSAGE_I)//no READ c/ tramas I pra apanhar possiveis sets ou discs
		{
			if (rcv == getC(MESSAGE_I, 0) || rcv == getC(MESSAGE_I, 1)) { *state = STATE_MACHINE_C_RCV;  received_C_type = MESSAGE_I; }
			/*catch set on read()*/else if (appStatus == APP_STATUS_RECEIVER && rcv == getC(MESSAGE_SET, 0)) { *state = STATE_MACHINE_C_RCV; received_C_type = MESSAGE_SET; }
			else if (appStatus == APP_STATUS_RECEIVER && rcv == getC(MESSAGE_DISC, 0)) { *state = STATE_MACHINE_C_RCV; received_C_type = MESSAGE_DISC; }
			else *state = STATE_MACHINE_START;
		}
		//WRITE  RR/REJ
		else if (msgExpectedType == MESSAGE_RR || msgExpectedType == MESSAGE_REJ)
		{
			if (rcv == getC(MESSAGE_RR, 0) || rcv == getC(MESSAGE_RR, 1)) { *state = STATE_MACHINE_C_RCV;  received_C_type = MESSAGE_RR; }
			else if (rcv == getC(MESSAGE_REJ, 0) || rcv == getC(MESSAGE_REJ, 1)) { *state = STATE_MACHINE_C_RCV;  received_C_type = MESSAGE_REJ; }
			else *state = STATE_MACHINE_START;
		}
		//SET,GET or DISC
		else  if (rcv == getC(msgExpectedType, 0)) {
			*state = STATE_MACHINE_C_RCV;
			received_C_type = msgExpectedType;
		}
		else *state = STATE_MACHINE_START;
		received_C=rcv;
		return OK;

	case STATE_MACHINE_C_RCV:
		//note:C type already received so it is a valid msg type (if it wasn't state would go back to START) unless we have an error in which bcc will hopefully fail
	  //BBC1 is not generating REJ, only BBC2. should I change this behaviour???
	  if (rcv == getBCC1(adressStatus, received_C_type,
		  /*will only work on RR and REJ*/(received_C>>5 & 0b00000001)) 
	     )       *state = STATE_MACHINE_BCC_RCV;
		else *state = STATE_MACHINE_START;
		
		DEBUG_SECTION(DEBUG_STATE_MACHINE_GETC,
		char aux = getBCC1(adressStatus, received_C_type,(received_C>>5 & 0b00000001));
		printf("\n--" PRINTBYTETOBINARY , BYTETOBINARY(aux));
		);
		
		return OK;

	case STATE_MACHINE_BCC_RCV://also goes 2 this state after STATE_MACHINE_ESCAPE
		if (rcv == FLAG) *state = STATE_MACHINE_STOP;
		else if (received_C_type == MESSAGE_I)
		{
			if (rcv == ESCAPE) *state = STATE_MACHINE_ESCAPE;
		}
		else *state = STATE_MACHINE_START;
		return OK;

	case STATE_MACHINE_ESCAPE://could be done outside but makes more sense to be here
		*state = STATE_MACHINE_BCC_RCV;
		return OK;

	default:
		printf("\nWARNING(usm2):Not valid/expected state (%d) reached in --> int state_machine(app_status_type status) => from => Protocol.c\n", *state);
		return 1;
	}

	return 1;
}

#endif /*GET MSGS STATE MACHINE*/

//-----   STTUFING AND DESTUFFING - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if (1)
/*buf deve entrar ja com o dobro do tamanho dos caracteres que tem
pode ser dinamico ou nao
data_size deve ser o tamanho efectivo ocupado
assim poupasse processamento extra usando a soluçao em 1) ou 2) e evitasse possiveis erros de memoria
*/
int apply_stuffing(char* buf, int /*bufSize*/data_size)
{
	int newSize = data_size;
	int i = 0;

	/*
	//1)count prev 2 avoid multiple reallocs
	for (; i < bufSize; ++i)/
		if (*buf[i] == FLAG || *buf[i] == ESCAPE)
			++newSize;
	*buf = (char)realloc(*buf, newSize);
	*/

	i = 0;
	for (; i < newSize; ++i)
		if (buf[i] == FLAG || buf[i] == ESCAPE)
		{
			///*2)using multiple reallocs*/*buf = (char)realloc(*buf,++newSize);
			++newSize;
			memmove(buf + i + 1, buf + i, newSize - i);
			buf[i] = ESCAPE;
			buf[i + 1] ^= 0x20;
			i++;			
		}

	return newSize;
}

//buf nao precisa de ser array dinamico
#define DEBUG_DESTUFFING 1
int apply_destuffing(char* buf, int bufSize)
{
	//int newSize = bufSize;
	int i = 0;
	for (; i < bufSize; ++i)
		if (buf[i] == ESCAPE)
		{
		  	--bufSize;
			memmove(buf + i, buf + i + 1, bufSize - i);
			buf[i] ^= 0x20;
			
			DEBUG_SECTION(DEBUG_DESTUFFING,
			printf("\n");
			printf("ZZ-");
			int a=0;
			for(;a<bufSize;++a)
			printf(PRINTBYTETOBINARY "  ", BYTETOBINARY(buf[a])););
		}

	return bufSize;
}
#endif /*STTUFING AND DESTUFFING*/

//-----   BCC2 GENERATION AND VALIDATION - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if (1)
#define BCC2_EVEN 0
#define BCC2_ODD  1
#define DEBUG_GENBBC2 1
char genBCC2(char* buf, int bufsize) {
	
	/*send (almost always) invalid bcc*/if (gen0bcc) return 0;

	char BCC2;
	if (BCC2_EVEN) BCC2 = 0b11111111;
	else		 BCC2 = 0b00000000;

	int i = 0;
	for (; i < bufsize; i++)
	{
	  BCC2 ^= buf[i];
	  DEBUG_SECTION(DEBUG_GENBBC2,printf("\ngenbcc2debug(i:%d):" PRINTBYTETOBINARY,i ,BYTETOBINARY(BCC2)););
	}

	return BCC2;
}

#define DEBUG_VALIDATEBBC2 1
char validateBCC2(char* buf, int bufsize) {
	char valid = buf[0];
	int i = 1;
	for (; i < bufsize; i++)
		valid ^= buf[i];

	DEBUG_SECTION(DEBUG_VALIDATEBBC2,printf("\nvalbcc2debug:" PRINTBYTETOBINARY, BYTETOBINARY(valid)););
	
	if (BCC2_EVEN) { if (valid == 0b11111111) return OK; }
	else if (valid == 0b00000000) return OK;

	return 1;
}
#endif /*BCC2 GENERATION AND VALIDATION */

//-----   WRITE AND READ MSGS AUX - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if (1)
void write_UorS(app_status_type adressStatus, message_type msg_type, int SorR, int fd)
{
	char msg[4];
	getMessage(adressStatus, msg_type, SorR, msg);
	if (write(fd, msg, 4) != 4)
	{
		perror("write_UorS():");
	}
	if (write(fd, msg, 1) != 1)
	{
		perror("write_UorS():");
	}
}

#define DEBUG_WRITE_I 1
//data must come in without stuffing
int write_I(int SorR, int fd, char* data, int data_size)
{
	char msg[4];
	getMessage(APP_STATUS_TRANSMITTER, MESSAGE_I, SorR, msg);
	if (write(fd, msg, 4) != 4)
	{
		perror("write_I():");
		return -1;
	}

		DEBUG_SECTION(DEBUG_WRITE_I,
		      printf("\n");
		      int i=0;
		      for(;i<data_size;++i)
	printf(PRINTBYTETOBINARY "  ", BYTETOBINARY(data[i]));
	);
	
	char finalMessage2Send[(data_size+1)*2];//char* finalMessage2Send = (char*)malloc(data_size + 1);
	memcpy(finalMessage2Send, data, data_size);
	finalMessage2Send[data_size] = genBCC2(data, data_size);

	DEBUG_SECTION(DEBUG_REJ_WITHWRONG_BCCS,
		gen0bcc = gen0bcc? FALSE: TRUE;);

	data_size = apply_stuffing(finalMessage2Send, data_size + 1);

	DEBUG_SECTION(DEBUG_WRITE_I,
		      printf("\nDEBUG WRITTE I SECT 2, data_size=%d",data_size);
		      printf("\n");
		      int i=0;
		      for(;i<data_size;++i)
	printf(PRINTBYTETOBINARY "  ", BYTETOBINARY(finalMessage2Send[i]));
	);
	
	//set size to max available 
	if (write(fd, finalMessage2Send, data_size) != data_size)
	{
		perror("write_I():");
		return -1;
	}

	if (write(fd, msg, 1) != 1)
	{
		perror("write_I():");
		return -1;
	}

	//free(finalMessage2Send);

	return 5 + data_size;
}
#endif /*WRITE AND READ MSGS AUX*/

// - - - LLOPEN AUXS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if (1)
#define DEBUG_LLOPEN_RECEIVER_OVERFLOW 1
int llopen_receiver(int fd)
{
	state_machine_state state = STATE_MACHINE_START;
	char buf[20];//used to receive stuff
	int res;//number of bytes read or sent
	//- - - - - - - - - -
	//receive SET
	while (state != STATE_MACHINE_STOP || received_C_type != MESSAGE_SET)
	{
		if ((res = read(fd, buf, 20)) < 0) perror("llopen_receiver:");

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
	write_UorS(APP_STATUS_TRANSMITTER, MESSAGE_UA, 0, fd);

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
	bool QUIT = NO;

	DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
		printf("\n-section2-");
	sleep(1);
	);

	while (QUIT == NO && timeouts_done < link_layer_data.numTransmissions)
	{
		write_UorS(APP_STATUS_TRANSMITTER, MESSAGE_SET, 0, fd);

		startAlarm();//ready alarm for possible timeout

		DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
			printf("\n-section3-");
		sleep(1);
		);

		while (STOP == FALSE)
		{
			//get respose
			if ((res = read(fd, buf, 20)) < 0) perror("llopen_transmitter:");

			DEBUG_SECTION(DEBUG_PRINT_SECTION_NUM,
				printf("\n-section4-");
			sleep(1);
			);

			DEBUG_SECTION(DEBUG_LLOPEN_TRANSMITTER_OVERFLOW,
				if (res > 5)
					printf("\nWARNING:llopen_transmitter received more than 5 characters at 1 time\n");
			);

			//update state machine and stop loop if valid
			int i = 0;
			for (; i < res && (state != STATE_MACHINE_STOP || received_C_type != MESSAGE_UA); ++i)
				update_state_machine(APP_STATUS_TRANSMITTER, APP_STATUS_TRANSMITTER, MESSAGE_UA, buf[i], &state);

			if (state == STATE_MACHINE_STOP && received_C_type == MESSAGE_UA)
			{
				stopAlarm(); QUIT = YES; break;
			}

			DEBUG_SECTION(DEBUG_LLOPEN_TRANSMITTER_OVERFLOW,
				if (res > i)
					printf("\nWARNING:llopen_transmitter received more characters than expected\n");
			);
		}

	}

	//END AND RETURN 
	//if did not receive confirmation UA
	if (state != STATE_MACHINE_STOP || received_C_type != MESSAGE_UA)
	{
		printf("\nllopen: No confirmation of the reception was received!");
		if (timeouts_done >= link_layer_data.numTransmissions)
			printf("\nllopen: Max tries (timeouts) reached.\n");
		return -1;
	}
	//or else
	printf("\nllopen: Connection established.\n");
	return OK;//replace with fd later
}

#endif /*LLOPEN AUXS*/

// - - - LLCLOSE AUXS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if (1)

int llclose_receiver(int fd)
{
	int res;
	state_machine_state state = STATE_MACHINE_START;
	timeouts_done = 0;
	char buf[20];//used to receive stuff
	bool QUIT = NO;

	while (QUIT == NO && timeouts_done < link_layer_data.numTransmissions)
	{
		//done in read when disc is received
		//write_UorS(APP_STATUS_RECEIVER, MESSAGE_DISC, 0, fd);

		startAlarm();//ready alarm for possible timeout

		while (STOP == FALSE)
		{
			//get respose
			if ((res = read(fd, buf, 20)) < 0) perror("llclose_receiver:");

			//update state machine and stop loop if valid
			int i = 0;
			for (; i < res && (state != STATE_MACHINE_STOP || received_C_type != MESSAGE_UA); ++i)
				update_state_machine(APP_STATUS_TRANSMITTER, APP_STATUS_RECEIVER, MESSAGE_UA, buf[i], &state);

			//quit if received confirmation from transmitter
			if (state == STATE_MACHINE_STOP && received_C_type == MESSAGE_UA)
			{
				stopAlarm(); QUIT = YES; break;
			}
		}
	}

	//END AND RETURN 
	//if did not receive confirmation UA
	if (state != STATE_MACHINE_STOP || received_C_type != MESSAGE_UA)
	{
		printf("\nllclose: No confirmation of the reception was received!");
		if (timeouts_done >= link_layer_data.numTransmissions)
			printf("\nllclose: Max tries (timeouts) reached.\n");
		return -1;
	}
	//or else
	printf("\nllclose: Connection was properly closed.\n");
	return OK;
}

int llclose_transmitter(int fd)
{
	int res;
	state_machine_state state = STATE_MACHINE_START;
	timeouts_done = 0;
	char buf[20];//used to receive stuff
	bool QUIT = NO;

	while (QUIT == NO && timeouts_done < link_layer_data.numTransmissions)
	{
		write_UorS(APP_STATUS_TRANSMITTER, MESSAGE_DISC, 0, fd);

		startAlarm();//ready alarm for possible timeout

		while (STOP == FALSE)
		{
			//get respose
			if ((res = read(fd, buf, 20)) < 0) perror("llclose_receiver:");

			//update state machine and stop loop if valid
			int i = 0;
			for (; i < res && (state != STATE_MACHINE_STOP || received_C_type != MESSAGE_DISC); ++i)
				update_state_machine(APP_STATUS_TRANSMITTER, APP_STATUS_RECEIVER, MESSAGE_DISC, buf[i], &state);

			if (state == STATE_MACHINE_STOP && received_C_type == MESSAGE_DISC)
			{
				stopAlarm(); QUIT = YES; break;
			}
		}
	}

	//END AND RETURN 
	//if did not receive confirmation UA
	if (state != STATE_MACHINE_STOP || received_C_type != MESSAGE_DISC)
	{
		printf("\nllclose: No confirmation of the reception was received!");
		if (timeouts_done >= link_layer_data.numTransmissions)
			printf("\nllclose: Max tries (timeouts) reached.\n");
		return -1;
	}
	//or else
	write_UorS(APP_STATUS_RECEIVER, MESSAGE_UA, 0, fd);
	printf("\nllclose: Connection was properly closed.\n");
	return OK;
}

#endif /*LLCLOSE AUXS*/

#endif//==========================================================================

//X=X=X=X   PROTOCAL MAIN FUNCS
#if (1)
//===============================================================================

int llopen(int fd, app_status_type status)
{
	occ_log.num_of_Is = 0;
	occ_log.total_num_of_timeouts= 0;
	occ_log.num_of_REJs			 = 0;
	if (status == APP_STATUS_TRANSMITTER)
	{
		app_status = APP_STATUS_TRANSMITTER;
		return llopen_transmitter(fd);
	}
	else if (status == APP_STATUS_RECEIVER)
	{
		app_status = APP_STATUS_RECEIVER;
		return llopen_receiver(fd);
	}
	else
	{
		printf("\nWARNING(llo):invalid app_status found on llopen().\n");
		return 1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define DEBUG_LLREAD_WARN_UNEXPECTED_MSG 1
#define LLREAD_AUXREADBUFFER_SIZE 128
#define LLREAD_AUXDATABUFFER_SIZE 500
//buffer must be dynamic!
int llread(int fd, char** buffer)
{
	STOP = FALSE;/*doesn't do anything right now. could define timeout later 4 receiver*/
	state_machine_state state = STATE_MACHINE_START;
	state_machine_state prevstate = 0;
	char auxReadBuf[LLREAD_AUXREADBUFFER_SIZE];//aux buffer used to receive stuff on read
	char auxReceiveDataBuf[LLREAD_AUXDATABUFFER_SIZE];//aux buffer used to store data
	int auxReceiveDataBuf_length = 0;
	int res;//number of bytes read or sent

	//devia usar 1 timeout pro read tb nao?

	//bool RECEIVE = YES;//used to cycle
	//- - - - - - - - - -
	//receive stuff
	while (TRUE)//RECEIVE)
	{
		/*clear buf*/memset(auxReadBuf, 0, LLREAD_AUXREADBUFFER_SIZE);
		res = read(fd, auxReadBuf, LLREAD_AUXREADBUFFER_SIZE);

		//UPDATE STATE MACHINE - UPDATE STATE MACHINE - UPDATE STATE MACHINE
		int i = 0;
		//int lastStart = 0;
		for (; i < res; ++i)
		{
			prevstate = state;
			update_state_machine(APP_STATUS_RECEIVER, APP_STATUS_TRANSMITTER, MESSAGE_I, auxReadBuf[i], &state);
			//if (state == STATE_MACHINE_START;) lastStart = i;

			//receive data to auxReceiveDataBuf
			if (
				(prevstate == STATE_MACHINE_BCC_RCV || prevstate == STATE_MACHINE_ESCAPE)
				&& (state == STATE_MACHINE_BCC_RCV || state == STATE_MACHINE_ESCAPE)
				&& received_C_type == MESSAGE_I)
			{
				auxReceiveDataBuf[auxReceiveDataBuf_length] = auxReadBuf[i];
				++auxReceiveDataBuf_length;
			}

			if (state == STATE_MACHINE_STOP){

				if (received_C_type == MESSAGE_I)//RECEIVED DATA; SEND RR or REJ; destuff and retrieve DATA
				{
					if (auxReceiveDataBuf_length == 0)
					{
						printf("\nllread:empty I message received! (? ? ?)");
						return -1;
					}

					auxReceiveDataBuf_length = apply_destuffing(auxReceiveDataBuf, auxReceiveDataBuf_length);
					
					if (validateBCC2(auxReceiveDataBuf,auxReceiveDataBuf_length)==OK)
					{
						NR = received_C == I0? 1:0;

						--auxReceiveDataBuf_length;//leave BCC2 out of data
						write_UorS(APP_STATUS_TRANSMITTER, MESSAGE_RR, NR, fd);
						//note: the original pointer must be updated so a pointer to the pointer must be used
						if ((*buffer = (char*)malloc(auxReceiveDataBuf_length)) == NULL)
						{
							perror("llread:");
							return -1;
						}
						memcpy(*buffer, auxReceiveDataBuf, auxReceiveDataBuf_length);

						/*update Occurrences_Log*/++occ_log.num_of_Is;

						return auxReceiveDataBuf_length;
					}
					else
					{
						write_UorS(APP_STATUS_TRANSMITTER, MESSAGE_REJ, (received_C == I0 ? 0 : 1), fd);
						state = STATE_MACHINE_START;
						/*update Occurrences_Log*/++occ_log.num_of_REJs;
					}
				}
				//RECEIVED DISC ; SEND DISC BACK
				else if (received_C_type == MESSAGE_DISC)
				{
					write_UorS(APP_STATUS_RECEIVER/*???*/, MESSAGE_DISC, 0, fd);
					return -2;//should notify the upper layer somehow
				}
				//RECEIVED SET ; SEND UA BACK
				else if (received_C_type == MESSAGE_SET)
				{
					write_UorS(APP_STATUS_TRANSMITTER, MESSAGE_UA, 0, fd);
					state = STATE_MACHINE_START;
				}
				else
				{
					DEBUG_SECTION(DEBUG_LLREAD_WARN_UNEXPECTED_MSG, printf("\nllread:received unexpected msg of type %d", received_C_type););
					state = STATE_MACHINE_START; 
					//return OK;
				}
			}

		}
	}
	//- - - - - - - - - -

	return -1;
}

/*
manda I
espera RR ou REJ
se RR sai
se REJ repete de inicio
*/
#define DEBUG_LLWRITER_BADRR_R 1
#define DEBUG_LLWRITER_REJ 1
int llwrite(int fd, char * buffer, int length)
{
	int res;
	state_machine_state state = STATE_MACHINE_START;
	timeouts_done = 0;
	char buf[20];//used to receive stuff
	//bool QUIT = FALSE;

	while (/*QUIT == NO &&*/ timeouts_done < link_layer_data.numTransmissions)
	{
		int num_of_writen_bytes = write_I(NS, fd, buffer, length);
		
		startAlarm();//ready alarm for possible timeout

		while (STOP == FALSE)
		{
			//get respose
			if ((res = read(fd, buf, 20)) < 0) perror("llwriter:");

			//update state machine and stop loop if valid
			int i = 0;
			for (; i < res && (state != STATE_MACHINE_STOP); ++i)
				update_state_machine(APP_STATUS_TRANSMITTER, APP_STATUS_TRANSMITTER, MESSAGE_RR, buf[i], &state);

			if (state == STATE_MACHINE_STOP)
			{
				if (received_C_type == MESSAGE_RR)
				{
				  //check NR 2 c if it's ok to send next I
				  if (received_C != (NS ? RR0 : RR1))
				    {
					  state = STATE_MACHINE_START;
				      stopAlarm(); 
				      //update statistics
				      
				      DEBUG_SECTION(DEBUG_LLWRITER_BADRR_R,
				    printf("\nllwriter debug: received a RR which C had not expected R"););

				      break;
				    }
				    
					stopAlarm();
					NS = NS ? 0 : 1;

					/*update Occurrences_Log*/++occ_log.num_of_Is;

					return num_of_writen_bytes;//QUIT = YES; break;
				}
				else if (received_C_type == MESSAGE_REJ)
				{
					state = STATE_MACHINE_START;
					stopAlarm(); 
					
					if(received_C != (NS ? REJ1 :REJ0) )
					{
					  //if this happens means a reject from
					  //another message was received 
					  //in this case I'm not sure what 2 do.
					  //maybe the program should be aborted
					  //because the transmission failed somewhere
					  //and we cannot trace it back
					  
					  printf("\nllwriter:REJ with R different than expected !!!"
					  "\nthis means the some of the data was lost and cannot be recovered!\n"
					    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
					  );
					  //DO SOMETHING HERE;
					}
					
					DEBUG_SECTION(DEBUG_LLWRITER_REJ,
				    printf("\nllwriter debug: received REJ"););
					
					/*update Occurrences_Log*/++occ_log.num_of_REJs;

					break;
				}
				else return 0;
			}

		}

	}

	return -1;
}


int llclose(int fd)
{
	if (app_status == APP_STATUS_RECEIVER)			return llclose_receiver(fd);
	else if (app_status == APP_STATUS_TRANSMITTER)  return llclose_transmitter(fd);
	else return -1;
}

#endif//==========================================================================