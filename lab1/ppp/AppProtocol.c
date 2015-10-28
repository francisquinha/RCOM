#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h> 

#include "utilities.h"
#include "DataLinkProtocol.h"
#include "AppProtocol.h"
#include "FileFuncs.h"

//================================================================================================================
//AUX DISPLAY
//================================================================================================================
//will call show progress if packet send/received and if at least <UPDATE_DISPLAY_MIN_TIME_INTERVAL> seconds have elapsed
time_t start, end;//get elapsed time to avoid spamming interface
double timedif;//avoid interface spam
#define UPDATE_DISPLAY_MIN_TIME_INTERVAL 0.3f
int progress_icon_state = 0;
const int NUMBER_OF_BARS_IN_PROGRESS_BAR = 20;
const char progress_bar_character = '#';
char progress_icon = 0;
void show_progress(int appstatus,unsigned int image_already_bytes, unsigned int image_bytes_length)
{

		switch (progress_icon_state){
		case 0: progress_icon = '~'; break;
		case 1: progress_icon = '\\'; break;
		case 2: progress_icon = '|'; break;
		case 3: progress_icon = '/'; break;
		default: progress_icon = ' ';
		}
		progress_icon_state = (progress_icon_state + 1) % 4;

		//-  - - - - - - - - - - - - - - - - - - - - - - - - -
		system("clear");


		/*if (data->estimate_recBytesPerSec > 1000)
		printf("\n Rate:%d KB/sec", data->estimate_recBytesPerSec / 1000);
		else
		printf("\n Rate:%d B/sec", data->estimate_recBytesPerSec);
		*/

		if (appstatus) printf("Received ");
		else printf("Sent ");

		printf("\n %dKB of %dKB", (image_already_bytes) / 1000, (image_bytes_length) / 1000);

		printf("\n-----------------------------------------");
		printf("\n      PROGRESS %c", progress_icon);
		printf("\n<");
		int number_of_block_2_print = image_already_bytes / (image_bytes_length / NUMBER_OF_BARS_IN_PROGRESS_BAR);
		int num_of_blanks_2_print = NUMBER_OF_BARS_IN_PROGRESS_BAR - number_of_block_2_print;
		for (; number_of_block_2_print > 0; --number_of_block_2_print) printf("%c", progress_bar_character);
		for (; num_of_blanks_2_print > 0; --num_of_blanks_2_print) printf(" ");
		printf(">\n");
	//}

	//return 0;
}

//================================================================================================================
//MAIN FUNCS
//================================================================================================================

int getControlPacket(char control, unsigned int size, unsigned char nameSize, const char *name, char *controlPacket) {
	char fileSize[32];
	unsigned int n = 0;
	while (size != 0) {
		fileSize[n] = (char)size;
		size >>= 8; // next byte
		n++;
	}
	controlPacket[0] = ((control == START) ? CS : CE);
	controlPacket[1] = TSIZE;
	controlPacket[2] = n;
	unsigned int i;
	for (i = 0; i < n; i++) controlPacket[3 + i] = fileSize[i];
	controlPacket[3 + n] = TNAME;
	controlPacket[4 + n] = nameSize;
	for (i = 0; i < nameSize; i++) controlPacket[5 + n + i] = name[i];
	return 5 + n + nameSize; 	// 1 byte for C, 2 bytes for T and L, n bytes for size, 2 bytes for T and L, nameSize bytes for name
}

int getInfoPacket(unsigned char N, unsigned int infoSize, char *info, char *infoPacket) {
	infoPacket[0] = CD;
	infoPacket[1] = N;
	infoPacket[2] = (unsigned char) (infoSize / 256);
	infoPacket[3] = (unsigned char) (infoSize % 256);
	unsigned int i;
	for (i = 0; i < infoSize; i++) infoPacket[4 + i] = info[i];
	return 4 + infoSize;	// 1 byte for C, 1 byte for N, 2 bytes for L2 and L1, L2 * 256 + L1 bytes for info
}

int sendControlPacket(int fd, char *controlPacket, int sizeControlPacket) {
	unsigned char try = 0;
	while (try < MAX_TRY) {
		if (llwrite(fd, controlPacket, sizeControlPacket) > 0) return OK;
		try++;
	}
	return -1;
}

int sendInfoPacket(int fd, char *infoPacket, int sizeInfoPacket) {
	unsigned char try = 0;
	while (try < MAX_TRY) {
		if (llwrite(fd, infoPacket, sizeInfoPacket) > 0) return OK;
		try++;
	}
	return -1;
}



int sendFile(unsigned char l2, unsigned char l1, int fd, unsigned char fileNameSize, const char *fileName, unsigned int image_bytes_length, const char *image_bytes, unsigned int* out_already_sent_bytes) {
	timedif = 0;
	char controlPacket[MAX_CTRL_P];
	int sizeControlPacket;
	char infoPacket[MAX_INFO_P];
	int sizeInfoPacket;
	unsigned int infoSize;
	unsigned int maxInfoSize = l2 * 256 + l1;
	//printf("packet size: %u\n", maxInfoSize);
	char info[maxInfoSize];
	unsigned char N = 0;

	//(unsigned int)image_bytes_length: this casting is not the ideal solution but works for now
	sizeControlPacket = getControlPacket(START, (unsigned int)(image_bytes_length - *out_already_sent_bytes), fileNameSize, fileName, controlPacket); // START control packet
	
	if (sendControlPacket(fd, controlPacket, sizeControlPacket) != OK)
		return -2;

	//printf("\n--fileName;%s\nimglength:%l\n", fileName, image_bytes_length);
	long i = 0;
	while (1) {

		time(&start);//avoid interface spam

		infoSize = 0;
		while (i < image_bytes_length && infoSize < maxInfoSize) {
			info[infoSize] = image_bytes[i];
			infoSize++;
			i++;
		}
		if (infoSize == 0) break;
		sizeInfoPacket = getInfoPacket(N, infoSize, info, infoPacket);
		if (sendInfoPacket(fd, infoPacket, sizeInfoPacket) == OK) {
			*out_already_sent_bytes+=infoSize;
			N++;

			//interface stuff
			time(&end);//avoid interface spam
			timedif += difftime(end, start);
			if (timedif >= UPDATE_DISPLAY_MIN_TIME_INTERVAL){
				timedif = 0;
				show_progress(1, *out_already_sent_bytes, image_bytes_length);
			}

		}
		else {
			return -1;
		}
	}
	controlPacket[0] = CE; // END control packet
	if (sendControlPacket(fd, controlPacket, sizeControlPacket) == OK)
		return OK;

	return -1;
}

int receivePacket(int fd, char **packet, int *sizePacket) {
	*sizePacket = llread(fd, packet);
	if (*sizePacket > 0) return OK;
	//else if (*sizePacket == -2) return -2;/*received disk*/
	else return -1;
}

#define DEBUG_RECEIVE_STEPS 1
int receiveFile(int fd, char* out_imagename, char** out_imagebuffer, unsigned int* out_image_buffer_length, unsigned int* out_already_received_imgbytes) {
	timedif = 0;
	char* packet;
	int sizePacket;
	unsigned int infoSize;
	unsigned char N = 0;
	char fileName[255];
	unsigned char fileNameSize;
	unsigned int fileSize;
	if (receivePacket(fd, &packet, &sizePacket) == OK) {
		if (packet[0] != CS) {
			free(packet);
				return -2;
		}
		if (packet[1] != TSIZE){
			free(packet);
			return -2;
		}

		unsigned char sizeFileSize = packet[2];
		fileSize = 0;
		unsigned int multiply = 1;
		unsigned int i;
		for (i = 0; i < sizeFileSize; i++) {
			//printf("\n>>> %u\n", ((unsigned int)packet[3 + i]));
			/*char to uint faz signal extend!!!*/
			fileSize += (0x00ff&((unsigned int)packet[3 + i])) * multiply;
			multiply *= 256;
		}

		*out_already_received_imgbytes = 0;
		*out_image_buffer_length = fileSize;
		*out_imagebuffer = (char*)malloc(fileSize);
		DEBUG_SECTION(DEBUG_RECEIVE_STEPS,
			printf("\nfileSize: %d\n", fileSize););

		if (packet[3 + sizeFileSize] != TNAME) {
			free(packet); return -2;
		}

		fileNameSize = packet[4 + sizeFileSize];
		for (i = 0; i < fileNameSize; i++)
			fileName[i] = packet[5 + sizeFileSize + i];

		memmove(out_imagename, fileName, (int) fileNameSize);
		out_imagename[(int)((int)fileNameSize>255 ? 255 : fileNameSize)] = 0;//some extra precaution

		DEBUG_SECTION(DEBUG_RECEIVE_STEPS,
			printf("\nfileName: %s\n", out_imagename););

		free(packet);
		time(&start);//avoid interface spam
		while (receivePacket(fd, &packet, &sizePacket) == OK) {
			if (packet[0] == CD) {
				if ( ((unsigned char) packet[1]) == N) {

					infoSize = (((unsigned int) packet[2]) & 0x00ff) * 256 + (((unsigned int) packet[3]) & 0x00ff);
					for (i = 0; i < infoSize; i++)
					{
						if (fileSize <= (*out_already_received_imgbytes) )
						{
							printf("\nERROR: receiveFile(...) received more data bytes than expected\n");
							free(packet);
							return -1;
						}
						(*out_imagebuffer)[(*out_already_received_imgbytes)] = packet[4 + i];
						(*out_already_received_imgbytes)++;//counts received bytes
					}
					N++;

					//interface stuff
					time(&end);//avoid interface spam
					timedif += difftime(end, start);
					if (timedif >= UPDATE_DISPLAY_MIN_TIME_INTERVAL){
						timedif = 0;
						show_progress(1, *out_already_received_imgbytes, *out_image_buffer_length);
					}

				}
				else {
					printf("\nERROR: receiveFile(...) not valid N\n");
					free(packet);
					return -1;
				}
			}
			else if (packet[0] == CE) {

				if (packet[1] != TSIZE) { printf("\nERROR: receiveFile(...) got CE with non valid TSIZE\n"); free(packet); return -1; }

				if (packet[2] != sizeFileSize) { printf("\nERROR: receiveFile(...) got CE with non  valid sizeFileSize\n"); free(packet); return -1; }

				unsigned int finalFileSize = 0;
				multiply = 1;
				for (i = 0; i < sizeFileSize; i++) {
					finalFileSize += (0x00ff & ((unsigned int)packet[3 + i])) * multiply;
					multiply *= 256;
				}

				if (finalFileSize != fileSize) { printf("\nERROR: receiveFile(...) got CE with non  valid fileSize\n"); free(packet); return -1; }
				if (packet[3 + sizeFileSize] != TNAME)  { printf("\nERROR: receiveFile(...) got CE with non  valid TNAME\n"); free(packet); return -1; }
				if (packet[4 + sizeFileSize] == fileNameSize) {
					for (i = 0; i < fileNameSize; i++)
						if (packet[5 + sizeFileSize + i] != fileName[i]) return -1;
					free(packet);
					return OK;
				}
				else {
					printf("\nERROR: receiveFile(...) END file name != START file name\n");
					free(packet);
					return -1;
				}
			}
			else{
				printf("\nERROR: receiveFile(...) end CE not received\n");
				free(packet);
				return -1;
			}

			free(packet);

			time(&start);//avoid interface spam
		}//---(receeive packets loop) endwhile---
		return -1;
	}
	printf("\nERROR:receiveFile(...) ???\n");
	return -1;
}