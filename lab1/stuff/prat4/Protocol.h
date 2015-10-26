#ifndef TYPEDEF_BOOLEAN_DECLARED_
#define TYPEDEF_BOOLEAN_DECLARED_
typedef char bool;//in case utilities is not included first...
#endif /* TYPEDEF_BOOLEAN_DECLARED_*/

#ifndef PROTOCOL
#define PROTOCOL

//===============================================================================
//basic definitions
//===============================================================================

#define MAX_FRAME_SIZE 64
struct linkLayer {
	char port[20]; /*Dispositivo /dev/ttySx, x = 0, 4*/
	int baudRate; /*Velocidade de transmissão*/
	unsigned int sequenceNumber; /*Número de sequência da trama: 0, 1*/
	int timeout; /*Valor do temporizador: 1 s*/
	 int numTransmissions; /*Número de tentativas em caso de
								   falha*/
	char frame[MAX_FRAME_SIZE]; /*trama*/
};

typedef char app_status_type;//in case utilities is not included first...
#define APP_STATUS_TRANSMITTER		 0
#define APP_STATUS_RECEIVER			 1

typedef char message_type;
#define MESSAGE_SET		1
#define MESSAGE_DISC	2
#define MESSAGE_UA		3
#define MESSAGE_RR		4
#define MESSAGE_REJ		5
#define MESSAGE_I		6

void set_basic_definitions( int timeout_in_seconds,  int number_of_tries_when_failing, char* port, int boudrate);

/*int pack_data(char data);*/

//===============================================================================
//BASIC STUFF
//===============================================================================

/*
* @brief open conection
@param tio_fd -> point to file descrictor used in conection. can be left empty, set it to 0/NULL if so is desired
...etc
*/
int open_tio(int* tio_fd,int vtime, int vmin);//Vmin could be placed on linklayer

/*
@brief close conection
*/
int close_tio(int tio_fd);

//===============================================================================
//PROTOCOL MAIN FUNCS
//===============================================================================

int llopen(int fd, app_status_type status);

int llwrite();

int llread();

int llclose();

#endif /* !PROTOCOL */