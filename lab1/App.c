#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

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
	unsigned char l2;	/* info packet size = l2 * 256 + l1 */
	unsigned char l1;	/* defaults are L2 and L1 defined in AppProtocol.h */	
};

struct applicationLayer app;

occurrences_Log_Ptr datalink_log;

bool conection_open = FALSE;

pthread_t display_thread;
bool show_display;

//bool image_loaded = NO; //check with image bytes length nstead
unsigned int image_already_bytes;//num of image's bytes sent or received
char* image_bytes;
unsigned int image_bytes_length;
char image_name[255]; //name is not path!!!
unsigned char image_name_length = 0;

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

void setPacketSize(int packetSize) {
	app.l2 = (unsigned char) (packetSize / 256);
//	printf("l2: %u\n", app.l2);
	app.l1 = (unsigned char) (packetSize % 256);
//	printf("l1: %u\n", app.l1);
}

void config(char baud, char recon, char timeo, int packetSize)
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
		reconect_tries = 1; break;
	case 'b':
		reconect_tries = 3; break;
	case 'c':
		reconect_tries = 5; break;
	case 'd':
		reconect_tries = 7; break;
	default: break;
	}
	/*
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
*/
	set_basic_definitions(/*timeout,*/ reconect_tries, 0, baudrate, packetSize);

	setPacketSize(packetSize);

}

//return -1 if failed to send complete image, -2 if not even start was sent
int sendImage() {
	//display 
	void* args[] = { &show_display, &(app.status), &image_bytes_length, &image_already_bytes };
	//show_display = YES;
/*	if (pthread_create(&display_thread, NULL, show_progress, (void*)args) != OK)
		printf("\nProccess state display thread failed to init.\n");
*/
	//send
	int ret = 0;
	image_already_bytes = 0;
	ret = sendFile(app.l2, app.l1, app.fd, image_name_length, image_name, image_bytes_length, image_bytes, &image_already_bytes);

	show_display = NO;//join after to avoid delays

	//if(ret==-1) can_reconect=YES;

	return ret;
}

//return 0 if ok, -1 if image was not received, -2 start faled, -3 if connection failed on disk
int receiveImage() {
	//display 
	void* args[] = { &show_display, &(app.status), &image_bytes_length, &image_already_bytes };
	//show_display = YES;
/*	if (pthread_create(&display_thread, NULL, show_progress, (void*)args) != OK)
		printf("\nProccess state display thread failed to init.\n");
*/
	//receive
	int ret = 0;
	image_already_bytes = 0;
	ret = receiveFile(app.fd, image_name, &image_bytes, &image_bytes_length, &image_already_bytes);

	//if(ret==-1) can_reconect=YES;

	show_display = NO;//join after to avoid delays

	//receive disk(do this before saving image to avoid delays)
	char* packet; int llread_result = 0;
	llread_result = llread(app.fd, &packet);
	if (llread_result != -2)//read returns -2 when receives disk
	{
		if (llread_result > 0) free(packet);
		ret = -3;
	}

	return ret;
}


int reconnect()
{
	if (image_already_bytes == 0 || image_already_bytes == image_bytes_length)
	{
		if (app.status) printf("\nNot possible:There is nothing to re-send");
		else printf("\nNot possible:There is no data already received or all the data as already been received.");
		return OK;
	}

	return OK;
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
	
	app.l2 = L2;	/* info packet size = l2 * 256 + l1 */
	app.l1 = L1; 	/* defaults are L2 and L1 defined in AppProtocol.h */

	image_bytes_length = 0;
	set_basic_definitions(3, argv[1], BAUDRATE, L2 * 256 + L1);

	char anws = ' ';
	while (anws != 'f'){
		anws = main_menu(app.status);
		switch (anws){

		case 'a':
			if (app.status == APP_STATUS_TRANSMITTER && image_bytes_length <= 0)
			{
				printf("\nNO IMAGE SELECTED!");
			}
			else if (connect() == 0)
			{
				conection_open = TRUE;

				if (
					(app.status == APP_STATUS_TRANSMITTER ?
					sendImage() : receiveImage()) == 0)
				{
					show_display = NO;
					llclose(app.fd);

					//save file if receiver
					if (app.status == APP_STATUS_RECEIVER){
						if (save2File(image_bytes, image_bytes_length, image_name) != OK){
//							pthread_join(display_thread, NULL);
							printf("\nImage was not saved sucessfully.\n");
							return -1;
						}
						printf("\nImage was saved sucessfully.\n");
					}

				}
				close_tio(app.fd);
				conection_open = FALSE;

				show_display = NO;
//				pthread_join(display_thread, NULL);
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
				image_bytes_length = selectNload_image(&image_bytes, image_name, &image_name_length);
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
