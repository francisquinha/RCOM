#ifndef USER_INTERFACE
#define USER_INTERFACE

typedef struct 
{
	int estimate_recBytesPerSec; //only counts data bytes
	int total_excepted;
	int total_received_or_sent;
}Emission_data;

int main_menu(bool receiver);

/*@brief display menu configuration
* @param apply_options must be a pointer to a func that receives a pointer to an array of 4 chars. The chosen options will be sent to ths method that should apply them.
*/
int select_config(void(*apply_options) (char, char, char, int));


/*@brief displays emission data. Speed of emission, amount of data received, etc...
* @param msg message to be displayed at the top
*/
void show_progress(char* msg, Emission_data* data);


/*
//the params maybe sent as references to the protocol to be updated
//or the data could be defined in protocol instead. the later seems to make more sense

struct Prog_Stats
{	
	//debug related
	long total_bytes_received;
	int num_of_disconnections;
	int total_num_of_errors_found;//packets that had error
	//...
};

void show_prog_stats();
*/

#endif /*USER_INTERFACE*/