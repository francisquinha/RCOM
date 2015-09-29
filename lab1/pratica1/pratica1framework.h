#define FALSE 0
#define TRUE 1

/*
 * @brief open conection
   @param tio_fd -> point to file descrictor used in conection. can be left empty, set it to 0/NULL if so is desired
   ...etc
 */
int open_tio(int* tio_fd,char* serial_port,int vtime, int vmin);

/*
 @brief close conection
 */
int close_tio(); 



int read_message(char* buf);

int write_message(char* message, int message_lenght);