#ifndef TYPEDEF_BOOLEAN_DECLARED_
#define TYPEDEF_BOOLEAN_DECLARED_
typedef char bool;//in case utilities is not included first...
#endif /* TYPEDEF_BOOLEAN_DECLARED_*/

#ifndef DATALINKPROTOCOL
#define DATALINKPROTOCOL

//===============================================================================
//basic definitions
//===============================================================================

//should be relative to a single transmission
struct occurrences_Log{

	unsigned long num_of_Is;//sent if host(only counts when confirmation is received) | received by client
	unsigned long total_num_of_timeouts;
	unsigned long num_of_REJs;//received if host | sent if client
};

typedef struct occurrences_Log* occurrences_Log_Ptr;

typedef char app_status_type;//in case utilities is not included first...
#define APP_STATUS_TRANSMITTER		 0
#define APP_STATUS_RECEIVER			 1

void set_basic_definitions(/*int timeout_in_seconds*/int number_of_tries_when_failing, char* port, int boudrate, int packetSize);

//self explanatory
occurrences_Log_Ptr get_occurrences_log();

//===============================================================================
//PROTOCOL MAIN FUNCS
//===============================================================================

int llopen(app_status_type status);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char** buffer);

int llclose(int fd);

#endif /* DATALINKPROTOCOL */