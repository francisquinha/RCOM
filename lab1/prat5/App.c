#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utilities.h"
#include "user_interface.h"
#include "DataLinkProtocol.h"
#include "FileFuncs.h"
#include "AppProtocol.h"

#define BAUDRATE B38400 //default baudrate
#define _POSIX_SOURCE 1 // POSIX compliant source 
/*#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;
*/

//=======================================================================
// PROGRAM VARIABLES
//=======================================================================

struct applicationLayer {
	int fd; /*Descritor correspondente à porta série*/
	int status; /*TRANSMITTER 0 | RECEIVER 1*/
};

struct applicationLayer app;

occurrences_Log_Ptr datalink_log;

bool conection_open = FALSE;

//bool image_loaded = NO; //check with image bytes length nstead
char* image_bytes;
long image_bytes_length;

//=======================================================================
// PROGRAM FUNCS
//=======================================================================

int connect()
{
	if (open_tio(&app.fd, 0, 0) != OK)
	{
		printf("\nERROR: Could not open terminal\n");
		exit(1);
	}


	if (llopen(app.fd, app.status) < 0) return -1;

	return 0;

}

void config(char baud, char recon, char timeo, int frame)
{
	int baudrate = -1;
	switch (baud) {
	case 'a':
		baudrate = B0; break;
	case 'b':
		baudrate = B50; break;
	case 'c':
		baudrate = B75; break;
	case 'd':
		baudrate = B110; break;
	case 'e':
		baudrate = B134; break;
	case 'f':
		baudrate = B150; break;
	case 'g':
		baudrate = B200; break;
	case 'h':
		baudrate = B300; break;
	case 'i':
		baudrate = B600; break;
	case 'j':
		baudrate = B1200; break;
	case 'k':
		baudrate = B1800; break;
	case 'l':
		baudrate = B2400; break;
	case 'm':
		baudrate = B4800; break;
	case 'n':
		baudrate = B9600; break;
	case 'o':
		baudrate = B19200; break;
	case 'p':
		baudrate = B38400; break;
	case 'q':
		baudrate = B57600; break;
	case 'r':
		baudrate = B115200; break;
	case 's':
		baudrate = B230400; break;
	case 't':
		baudrate = B460800; break;
	default: break;
	}

	int reconect_tries = -1;
	switch (recon) {
	case 'a':
		reconect_tries = 0; break;
	case 'b':
		reconect_tries = 1; break;
	case 'c':
		reconect_tries = 3; break;
	case 'd':
		reconect_tries = 5; break;
	case 'e':
		reconect_tries = 7; break;
	default: break;
	}

	int timeout = -1;
	switch (timeo) {
	case 'a':
		timeout = 2; break;
	case 'b':
		timeout = 3; break;
	case 'c':
		timeout = 5; break;
	case 'd':
		timeout = 8; break;
	default: break;
	}

	set_basic_definitions(timeout, reconect_tries, 0, baudrate);
	//missing frame
}

int testread()
{
	char *receive;
	int rec_size = llread(app.fd, &receive);

	//catch info
	//char endchar = receive[rec_size - 1];
	//receive[rec_size - 1] = 0;
	printf("\n");
	//printf("%s%c - %d", receive, endchar, rec_size);test string
	int i = 0;
	for (; i < rec_size; ++i)
		printf(PRINTBYTETOBINARY " - ", BYTETOBINARY(receive[i]));
	printf("%d", rec_size);
	//gets(receive);
	if (rec_size>0)free(receive);

	//catch DISC
	rec_size = llread(app.fd, &receive);
	if (rec_size == -2) printf("\ngot disk");

	return 0;
}

int testwriter()
{
	/*set_basic_definitions(3, 3, argv[1], BAUDRATE);
	if (open_tio(&app.fd, 0, 0) != OK)
	{
	printf("\nERROR:Couldnot open terminal\n");
	exit(1);
	}
	*/
	//if (llopen(app.fd, APP_STATUS_TRANSMITTER) == 0)
	//{
	//sleep(1);
	char controlPacket[MAX_CTRL_P];
	int sizeControlPacket;
	unsigned int fileSize = 123456789;
	unsigned char fileNameSize = 4;
	char fileName[4] = "ola";
	sizeControlPacket = getControlPacket(START, fileSize, fileNameSize, fileName, controlPacket); // START control packet
	if (sendControlPacket(app.fd, controlPacket, sizeControlPacket) == OK) {
		printf("\ndone");
		//llclose(app.fd);
	}

	//close_tio(app.fd);
	//}

	return 0;
}


//=======================================================================
// MAIN
//=======================================================================

int main(int argc, char** argv)
{
	if ((argc < 3) ||
		((strcmp("/dev/ttyS0", argv[1]) != 0) &&
		(strcmp("/dev/ttyS4", argv[1]) != 0))
		|| ((strcmp("t", argv[2]) != 0) && (strcmp("r", argv[2]) != 0))
		)
	{
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0 \nAppStatus: t (=transmitter) or r (=receiver)\n");
		exit(1);
	}

	if (strcmp("t", argv[2]) == 0) app.status = APP_STATUS_TRANSMITTER;
	else app.status = APP_STATUS_RECEIVER;

	image_bytes_length = 0;
	set_basic_definitions(3, 3, argv[1], BAUDRATE);

	char anws = ' ';
	while (anws != 'f'){
		anws = main_menu(app.status);
		switch (anws){

		case 'a':
			if (connect() == 0)
			{
				conection_open = TRUE;
				if (
					( app.status == APP_STATUS_TRANSMITTER ?
					testwriter() : testread()) == 0)
				{
					llclose(app.fd);
					//if (llclose() == OK)
					//{
				}
				close_tio(app.fd);
				conection_open = FALSE;
				sleep(9);
			}
			break;
		case 'b':printf("\nNOT IMPLEMENTED");//reconnect();
			break;
		case 'c':select_config(config);
			break;
		case 'd':
			if (app.status == APP_STATUS_TRANSMITTER)
			{
				if (image_bytes_length > 0) free(image_bytes);
				image_bytes_length = selectNload_image(&image_bytes);
			}
			else printf("\nNOT IMPLEMENTED");
			break;
		case 'e':
			datalink_log = get_occurrences_log();
			show_prog_stats(datalink_log->num_of_Is, datalink_log->total_num_of_timeouts, datalink_log->num_of_REJs, app.status);
			break;
		case 'f':printf("\nNow exiting...\n"); sleep(1);
			break;
		default: printf("\nNo valid command recognized."); sleep(1); break;
		}
	}

	//devia de ser feito um exit handler com isto para caso haja uma terminaçao inesperada
	if (conection_open) close_tio(app.fd);
	if (image_bytes_length > 0) free(image_bytes);

	return 0;
}
