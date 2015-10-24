#ifndef TYPEDEF_BOOLEAN_DECLARED_
#define TYPEDEF_BOOLEAN_DECLARED_
typedef char bool;//in case utilities is not included first...
#endif /* TYPEDEF_BOOLEAN_DECLARED_*/

#ifndef DATALINKPROTOCOL
#define DATALINKPROTOCOL

//===============================================================================
//basic definitions
//===============================================================================

//#define MAX_FRAME_SIZE 64
struct linkLayer{
	char port[20]; /*Dispositivo /dev/ttySx, x = 0, 4*/
	int baudRate; /*Velocidade de transmissão*/
	unsigned int sequenceNumber; /*Número de sequência da trama: 0, 1*/
	int timeout; /*Valor do temporizador: 1 s*/
	 int numTransmissions; /*Número de tentativas em caso de
								   falha*/
	 //int Iframe_numdatabytes;
	//char frame[MAX_FRAME_SIZE]; /*trama*/
};

//should be relative to a single transmission
//could be used to restore a lost conection???
struct occurrences_Log{
	//unsigned long total_bytes_received;
	//int num_of_disconnections;
	//int total_num_of_errors_found;

	unsigned long num_of_Is;//sent if host(only counts when confirmation is received) | received by client
	unsigned long total_num_of_timeouts;
	unsigned long num_of_REJs;//received if host | sent if client
};

typedef struct occurrences_Log* occurrences_Log_Ptr;


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

void set_basic_definitions(int timeout_in_seconds, int number_of_tries_when_failing, char* port, int boudrate);

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

//self explanatory
occurrences_Log_Ptr get_occurrences_log();

//===============================================================================
//PROTOCOL MAIN FUNCS
//===============================================================================

int llopen(int fd, app_status_type status);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char** buffer);

int llclose(int fd);

#endif /* DATALINKPROTOCOL */