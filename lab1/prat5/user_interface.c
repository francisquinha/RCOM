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
		printf("(%d,%d)", start, end);

		/*clean buf*/char c; while ((c = getchar()) != '\n' && c != EOF);
		fgets(get, 50, stdin); //gets(get);
		num = atoi(get);

		if (num != 0 && start <= num && num <= end)
			done = YES;
		else printf("Invalid input/range, please select again:\n==>");
	}

	return num;
}

char getAnswer(int numOfChoices)
{
	char get[2];
	do {
		/*clean buf*/char c; while ((c = getchar()) != '\n' && c != EOF);
		fgets(get, 2, stdin); //gets(get);
		if (get[0] < 'a' || get[0] >= 'a' + numOfChoices || get[1] != 0)
			printf("\nOption not recognized, please select again:");

	} while (get[0] < 'a' || get[0] >= 'a' + numOfChoices || get[1] != 0);

	return get[0];
}

int main_menu(bool receiver)
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

	printf("\nSelect max Reconect Tries: \na)0 \nb)1 \nc)3 \nd)5 \ne)7\n");
	reconectOpt = getAnswer(5);

	printf("\nSelect timeout interval: \na)2 secs \nb)3 secs \nc)5 secs \nd)8 secs \n=>");
	timetoutOpt = getAnswer(4);

	//not sure about what should be the max size of the packet.
	printf("\nInput packet size (without stuffing from 1 to 200??? not yet implemented!!!):\n");
	packetSize = getIntPositiveRange(1, 200);

	apply_options(boudOpt, reconectOpt, timetoutOpt, packetSize);

	return 0;
}

//do not spam this method
int progress_icon_state = 0;
void show_progress(char* msg, Emission_data* data)
{
	const int NUMBER_OF_BARS_IN_PROGRESS_BAR = 20;
	const char progress_bar_character = 219;
	char progress_icon = 0;
	switch (progress_icon_state){
	case 0: progress_icon = '-'; break;
	case 1: progress_icon = '\\'; break;
	case 2: progress_icon = '|'; break;
	case 3: progress_icon = '/'; break;
	default: progress_icon = ' ';
	}
	progress_icon_state = (progress_icon_state + 1) % 4;

	//-  - - - - - - - - - - - - - - - - - - - - - - - - -
	system("clear");
	printf("%s", msg);
	if (data->estimate_recBytesPerSec > 1000)
		printf("\n Rate:%d KB/sec", data->estimate_recBytesPerSec / 1000);
	else
		printf("\n Rate:%d B/sec", data->estimate_recBytesPerSec);

	printf("\nreceived %dKB of %dKB", data->total_received_or_sent / 1000, data->total_excepted / 1000);

	printf("\n-----------------------------------------");
	printf("\n               PROGRESS    %c", progress_icon);
	printf("\n<");
	int number_of_block_2_print = data->total_received_or_sent / (data->total_excepted / NUMBER_OF_BARS_IN_PROGRESS_BAR);
	for (; number_of_block_2_print > 0; --number_of_block_2_print) printf("%c", progress_bar_character);

	printf(">\n");
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
		printf("\nTotak number of REJs received: %lu", num_of_REJs);
	else /*transmitter*/printf("\nTotal number of REJs sent: %lu", num_of_REJs);

	printf("\n- - - - - - - - - - - - - - -\nPress Any key to return to pevious menu...\n");

	char get[2];
	/*clean buf*/char c; while ((c = getchar()) != '\n' && c != EOF);
	fgets(get, 2, stdin); //gets(get);
}



long selectNload_image(char** image_buffer)
{
	system("clear");
	printf("Please input image file path (relative or not):\n>");

	char get_path[100];
	/*clean buf*/char c; while ((c = getchar()) != '\n' && c != EOF);
	fgets(get_path, 100, stdin);//gets(get_path);
	char* image_path;
	while (TRUE){
		image_path = realpath(get_path, NULL);
		if (image_path != NULL) break;
		printf("\nInvalid path/file. Please choose another.\n");
		/*clean buf*/char c; while ((c = getchar()) != '\n' && c != EOF);
		fgets(get_path, 100, stdin); //gets(get_path);
	}

	long imageSize;
	if ((imageSize = getFileBytes(image_path, image_buffer)) < 0)
	{
		printf("\nFailed to load image.\n");
		return -1;
	}
	printf("\nImage sucessfully loaded.\n");
	sleep(2);

	return imageSize;
}