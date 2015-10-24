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


void show_prog_stats(unsigned long num_of_Is,
	unsigned long total_num_of_timeouts,
	unsigned long num_of_REJs, int appstatus);

#endif /*USER_INTERFACE*/