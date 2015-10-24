#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "utilities.h"
#include "user_interface.h"

//should have a start bigger than zero
int getIntPositiveRange(int start, int end) {
	int num;
	char get[50];
	int done = NO;

	while (!done) {
		printf("(%d,%d)", start, end);

		gets(get);
		num = atoi(get);

		if (num != 0 && start <= num && num <= end)
			done = YES;
		else printf("Invalid input/range, please select again:\n==>");
	}

	return num;
}

char getAnswer(int numOfChoices)
{
	char get[200];
	do {
		gets(get);
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

	if (receiver != 0) printf("\nd) Choose picture to send");

	printf("\ne) Quit\n\n==>");

	return getAnswer(5);
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