#ifndef USER_INTERFACE
#define USER_INTERFACE

char main_menu(bool receiver);

/*@brief display menu configuration
* @param apply_options must be a pointer to a func that receives a pointer to an array of 4 chars. The chosen options will be sent to ths method that should apply them.
*/
int select_config(void(*apply_options) (char, char, char, int));


//void* show_progress(void* args);


void show_prog_stats(unsigned long num_of_Is,
	unsigned long total_num_of_timeouts,
	unsigned long num_of_REJs, int appstatus);

unsigned int selectNload_image(char** image_buffer, char* out_image_name, unsigned char* out_image_name_length);


#endif /*USER_INTERFACE*/