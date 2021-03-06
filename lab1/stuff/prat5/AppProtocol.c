#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utilities.h"
#include "DataLinkProtocol.h"
#include "AppProtocol.h"
#include "FileFuncs.h"


//================================================================================================================
//AUX FUNCS
//================================================================================================================

/*ESTOU A PRESSUPOR QUE O ARRAY DATA-BUFFER E DINAMICO (USA ALLOCS)*/
/*int update_received_data(char* data_buffer, int data_buffer_length, char* newdata_buffer, int newdata_buffer_length) {
	if (data_buffer_length > 0)
	{
	if ((data_buffer = (char*)malloc(newdata_buffer_length)) == NULL)
	{
	perror("update_received_data:");
	return -1;
	}
	data_buffer_length = newdata_buffer_length;
	memmov(data_buffer, newdata_buffer, newdata_buffer_length);
	}
	else
	{
	data_buffer = (char*)realloc(data_buffer, data_buffer_length + newdata_buffer_length);
	memmov(data_buffer + data_buffer_length, newdata_buffer, newdata_buffer_length);
	data_buffer_length += newdata_buffer_length;
	}
	return OK;
	}
	*/
//================================================================================================================
//MAIN FUNCS
//================================================================================================================

int getControlPacket(char control, unsigned int size, unsigned char nameSize,const char *name, char *controlPacket) {
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
	infoPacket[2] = infoSize / 256;
	infoPacket[3] = infoSize % 256;
	unsigned int i;
	for (i = 0; i < infoSize; i++) infoPacket[4 + i] = info[i];
	return 4 + L2 * 256 + L1;	// 1 byte for C, 1 byte for N, 2 bytes for L2 and L1, L2 * 256 + L1 bytes for info
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

int sendFile(int fd, unsigned char fileNameSize, const char *fileName, unsigned int image_bytes_length, const char *image_bytes, unsigned  int* out_already_sent_bytes) {
	char controlPacket[MAX_CTRL_P];
	int sizeControlPacket;
	char infoPacket[MAX_INFO_P];
	int sizeInfoPacket;
	char info[L2 * 256 + L1]; 		// maximum size of info
	unsigned int infoSize;
	unsigned int maxInfoSize = L2 * 256 + L1;
	unsigned char N = 0;
	/*struct stat st;
	stat(fileName, &st);
	unsigned int fileSize = st.st_size;		// get file size from file statistics
	printf("File size: %d\n", fileSize);
	FILE *file;
	file = fopen(fileName, "r");
	sizeControlPacket = getControlPacket(START, fileSize, fileNameSize, fileName, controlPacket); // START control packet
	if (sendControlPacket(fd, controlPacket, sizeControlPacket) == OK) {
	int fileChar;
	while (1) {
	infoSize = 0;
	while ((fileChar = fgetc(file)) != EOF && infoSize < maxInfoSize) {
	info[infoSize] = (char) fileChar;
	infoSize++;
	}
	if (infoSize == 0) break;
	sizeInfoPacket = getInfoPacket(N, infoSize, info, infoPacket);
	if (sendInfoPacket(fd, infoPacket, sizeInfoPacket) == OK) N++;
	else {
	fclose(file);
	return -1;
	}
	}*/

	//(unsigned int)image_bytes_length: this casting is not the ideal solution but works for now
	sizeControlPacket = getControlPacket(START, (unsigned int)image_bytes_length, fileNameSize, fileName, controlPacket); // START control packet
	if (sendControlPacket(fd, controlPacket, sizeControlPacket) != OK)
		return -1;

	//printf("\n--fileName;%s\nimglength:%l\n", fileName, image_bytes_length);
	long i = 0;
	while (1) {
		infoSize = 0;
		while (i < image_bytes_length && infoSize < maxInfoSize) {
			info[infoSize] = image_bytes[i];
			infoSize++;
			i++;
		}
		if (infoSize == 0) break;
		sizeInfoPacket = getInfoPacket(N, infoSize, info, infoPacket);
		if (sendInfoPacket(fd, infoPacket, sizeInfoPacket) == OK)
		{
			++out_already_sent_bytes;
			N++;
		}
		else {
			//fclose(file);
			return -1;
		}
	}
	controlPacket[0] = CE; // END control packet
	if (sendControlPacket(fd, controlPacket, sizeControlPacket) == OK)
		return OK;

	return -1;
	/*
	controlPacket[0] = CE; // END control packet
	if (sendControlPacket(fd, controlPacket, sizeControlPacket) == OK) {
	fclose(file);
	return OK;
	}
	else {
	fclose(file);
	return -1;
	}
	}
	else return -1;	*/
}

int receivePacket(int fd, char **packet, int *sizePacket) {
	*sizePacket = llread(fd, packet);
	if (*sizePacket > 0) return OK;
	//else if (*sizePacket == -2) return -2;/*received disk*/
	else return -1;
}

#define DEBUG_RECEIVE_STEPS 1
int receiveFile(int fd, char* out_imagename, char** out_imagebuffer, unsigned int* out_image_buffer_length, unsigned int* out_already_received_imgbytes) {
	char* packet;
	int sizePacket;
	unsigned int infoSize;
	unsigned char N = 0;
	char fileName[255];
	unsigned char fileNameSize;
	unsigned int fileSize;
	if (receivePacket(fd, &packet, &sizePacket) == OK) {
		if (packet[0] != CS) return -1;
		if (packet[1] != TSIZE) return -1;

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
		//return -1;
		*out_already_received_imgbytes = 0;
		*out_image_buffer_length = fileSize;
		*out_imagebuffer = (char*)malloc(fileSize);
		DEBUG_SECTION(DEBUG_RECEIVE_STEPS,
			printf("\nfileSize: %d\n", fileSize););

		if (packet[3 + sizeFileSize] != TNAME)return -1;

		fileNameSize = packet[4 + sizeFileSize];
		for (i = 0; i < fileNameSize; i++)
			fileName[i] = packet[5 + sizeFileSize + i];

		memmove(out_imagename, fileName, (int) fileNameSize);
		out_imagename[(int)((int)fileNameSize>255 ? 255 : fileNameSize)] = 0;//some extra precaution

		DEBUG_SECTION(DEBUG_RECEIVE_STEPS,
			printf("\nfileName: %s\n", out_imagename););

		/*FILE *file;//------------
		file = fopen(fileName, "w"); // new clean file
		fclose(file);
		file = fopen(fileName, "a"); // append*/

		while (receivePacket(fd, &packet, &sizePacket) == OK) {
			if (packet[0] == CD) {
				if ( ((unsigned char) packet[1]) == N) {
					infoSize = packet[2] * 256 + packet[3];
					for (i = 0; i < infoSize; i++)
					{
						if (fileSize <= (*out_already_received_imgbytes) )
						{
							printf("\nERROR: receiveFile(...) received more data bytes than expected\n");
							return -1;
						}
						(*out_imagebuffer)[(*out_already_received_imgbytes)] = packet[4 + i];
						(*out_already_received_imgbytes)++;//counts received bytes
						//fputc(packet[4 + i], file);//-----------
					}
					N++;
				}
				else {
					printf("\nERROR: receiveFile(...) not valid N\n");
					return -1;
				}
			}
			else if (packet[0] == CE) {
				//fclose(file);//------------
				if (packet[1] != TSIZE) { printf("\nERROR: receiveFile(...) got CE with non valid TSIZE\n"); return -1; }

				if (packet[2] != sizeFileSize) { printf("\nERROR: receiveFile(...) got CE with non  valid sizeFileSize\n"); return -1; }

				unsigned int finalFileSize = 0;
				multiply = 1;
				for (i = 0; i < sizeFileSize; i++) {
					finalFileSize += (0x00ff & ((unsigned int)packet[3 + i])) * multiply;
					multiply *= 256;
				}

				if (finalFileSize != fileSize) { printf("\nERROR: receiveFile(...) got CE with non  valid fileSize\n"); return -1; }
				if (packet[3 + sizeFileSize] != TNAME)  { printf("\nERROR: receiveFile(...) got CE with non  valid TNAME\n"); return -1; }
				if (packet[4 + sizeFileSize] == fileNameSize) {
					for (i = 0; i < fileNameSize; i++)
						if (packet[5 + sizeFileSize + i] != fileName[i]) return -1;
					return OK;
				}
				else {
					printf("\nERROR: receiveFile(...) END file name != START file name\n");
					return -1;
				}
			}//-------
			else{
				printf("\nERROR: receiveFile(...) end CE not received\n");
				return -1;
			}
		}

	}
	printf("\nERROR:receiveFile(...) ???\n");
	return -1;
}