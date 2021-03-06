#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "utilities.h"
#include "user_interface.h"
#include "FileFuncs.h"

//should have a start bigger than zero
int getIntPositiveRange(int start, int end) {
	int num;
	char get[50];
	int done = NO;

	while (!done) {
		//printf("(%d,%d)", start, end);

		gets(get);
		//fgets(get, 50, stdin); 
		///*clean buf*/char c; while ((c = getchar()) != '\n' && c != EOF);
		num = atoi(get);

		if (num != 0 && start <= num && num <= end)
			done = YES;
		else printf("Invalid input/range, please select again:\n==>");
	}

	return num;
}

char getAnswer(int numOfChoices)
{
	char get[20];
	do {
		gets(get);
		//fgets(get, 2, stdin); 
		///*clean buf*/char c; while ((c = getchar()) != '\n' && c != EOF);
		if (get[0] < 'a' || get[0] >= 'a' + numOfChoices || get[1] != 0)
			printf("\nOption not recognized, please select again:");

	} while (get[0] < 'a' || get[0] >= 'a' + numOfChoices || get[1] != 0);

	return get[0];
}

char main_menu(bool receiver)
{
	system("clear");

	printf("\nWELCOME TO THE APP! PLEASE SELECT ONE OF THE FOLLOWING OPERATONS");

	if (receiver)
		printf("\na) Establish Conection and receive picture");
	else
		printf("\na) Establish Conection and send picture");

	printf("\nb) Try to re-establish lost conection");
	printf("\nc) Change configurations");

	if (receiver != 0) printf("\nd) Choose path to save picture");
	else printf("\nd) Choose picture to send");

	printf("\ne) Show Ocurrences Log");

	printf("\nf) Quit\n\n==>");

	return getAnswer(6);
}


int select_config(void(*apply_options) (char, char, char, int))
{
	char boudOpt, reconectOpt, timetoutOpt;
	int packetSize;

	system("clear");

	printf("Select baudrate:");
	printf("\na)B0 - Hang Up   b)B50");
	printf("\nc)B75            d)B110");
	printf("\ne)B134           f)B150");
	printf("\ng)B200           h)B300");
	printf("\ni)B600           j)B1200");
	printf("\nk)B1800          l)B2400");
	printf("\nm)B4800          n)B9600");
	printf("\no)B19200         p)B38400 - Default");
	printf("\nq)B57600         r)B115200");
	printf("\ns)B230400        t)B460800\n=>");
	boudOpt = getAnswer(20);

	printf("\nSelect max Reconect Tries: \na)1 \nb)3 \nc)5 \nd)7\n");
	reconectOpt = getAnswer(4);

/*	printf("\nSelect timeout interval: \na)2 secs \nb)3 secs \nc)5 secs \nd)8 secs \n=>");
	timetoutOpt = getAnswer(4);
*/

	printf("\nInput packet size (number of file bytes per packet 1 - 65535):\n");
	packetSize = getIntPositiveRange(1, 65535);

	apply_options(boudOpt, reconectOpt, timetoutOpt, packetSize);

	return 0;
}




void show_prog_stats(unsigned long num_of_Is,
	unsigned long total_num_of_timeouts,
	unsigned long num_of_REJs, int appstatus)
{
	system("clear");

	printf("- - - Ocurrences log - - -");

	if (appstatus/*receiver*/)
		printf("\nNumber of Is received: %lu", num_of_Is);
	else /*transmitter*/ printf("\nNumber of I's sent (RR confirmed): %lu", num_of_Is);

	printf("\nTotal number of timeouts: %lu", total_num_of_timeouts);

	if (appstatus/*receiver*/)
		printf("\nTotak number of REJs sent: %lu", num_of_REJs);
	else /*transmitter*/printf("\nTotal number of REJs received: %lu", num_of_REJs);

	printf("\n- - - - - - - - - - - - - - -\nPress Enter key to return to pevious menu...\n");

	char get[20];

	gets(get);
	//fgets(get, 2, stdin); 
	///*clean buf*/char c; while ((c = getchar()) != '\n' && c != EOF);
}



unsigned int selectNload_image(char** image_buffer, char* out_image_name, unsigned char* out_image_name_length)
{
	system("clear");
	printf("Please input image file path (relative or not):\n>");

	char get_path[100];

	gets(get_path);
	//fgets(get_path, 100, stdin);
	///*clean buf*/char c; while ((c = getchar()) != '\n' && c != EOF);

	char* image_path;
	while (TRUE){
		image_path = realpath(get_path, NULL);
		if (image_path != NULL) break;
		printf("\nInvalid path/file. Please choose another.\n");

		gets(get_path);
		//fgets(get_path, 100, stdin); 
		///*clean buf*/char c; while ((c = getchar()) != '\n' && c != EOF);
	}

	long imageSize;
	if ((imageSize = getFileBytes(image_path, image_buffer)) < 0)
	{
		printf("\nFailed to load image.\n");
		return -1;
	}

	//get name from path
	image_path = strrchr(get_path, '/');
	if (image_path==NULL)
	{
		strcpy(out_image_name, get_path);
	}
	else {
		//(image_path - get_path + 1)
		strcpy(out_image_name, image_path+1);
	}

	*out_image_name_length = strlen(out_image_name);

	printf("\nImage sucessfully loaded.<%s , %ld bytes>\n", out_image_name, imageSize);
	sleep(2);
	return (unsigned int) imageSize;
}