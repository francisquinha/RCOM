#include <sys/stat.h>
#include <stdio.h>

#include "utilities.h"
#include "DataLinkProtocol.h"
#include "AppProtocol.h"
//faltam includes


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

int getControlPacket(char control, unsigned int size, unsigned char nameSize, char *name, char *controlPacket) {
	char fileSize[32];
	unsigned int n = 0;
	while (size != 0) {
		fileSize[n] = (char) size;
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

int sendFile(int fd, unsigned char fileNameSize, char *fileName) {
	char controlPacket[MAX_CTRL_P];
	int sizeControlPacket;
	char infoPacket[MAX_INFO_P];
	int sizeInfoPacket;
	char info[L2 * 256 + L1]; 		// maximum size of info
	unsigned int infoSize;
	unsigned int maxInfoSize = L2 * 256 + L1;
	unsigned char N = 0;
	struct stat st;
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
		}
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
	else return -1;	
}

int receivePacket(int fd, char **packet, int *sizePacket) {
	*sizePacket = llread(fd, packet);
	if (*sizePacket > 0) return OK;
	else return -1;
}

int receiveFile(int fd) {
	char* packet;
	int sizePacket;
	unsigned int infoSize;
	unsigned char N = 0;
	char fileName[255];
	unsigned char fileNameSize;
	unsigned int fileSize;
	if (receivePacket(fd, &packet, &sizePacket) == OK) {
		if (packet[0] == CS) {
			if (packet[1] == TSIZE) {
				unsigned char sizeFileSize = packet[2];
				fileSize = 0;
				unsigned int multiply = 1;
				unsigned int i;
				for (i = 0; i < sizeFileSize; i++) {
					fileSize += (unsigned int) packet[3 + i] * multiply;
					multiply *= 256;
				}
				if (packet[3 + sizeFileSize] == TNAME) {
					fileNameSize = packet[4 + sizeFileSize];
					for (i = 0; i < fileNameSize; i++) 
						fileName[i] = packet[5 + sizeFileSize + i];
					FILE *file;
					file = fopen(fileName, "w"); // new clean file
					fclose(file);
					file = fopen(fileName, "a"); // append
					while (receivePacket(fd, &packet, &sizePacket) == OK) {
						if (packet[0] == CD) {
							if (packet[1] == N) {
								infoSize = packet[2] * 256 + packet[3];
								for (i = 0; i < infoSize; i++)
									fputc((int) packet[4 + i], file);
								N++;
							}
							else return -1;
						}
						else if (packet[0] == CE) {
							fclose(file);
							if (packet[1] == TSIZE) {
								if (packet[2] == sizeFileSize) {
									unsigned int finalFileSize = 0;
									multiply = 1;
									for (i = 0; i < sizeFileSize; i++) {
										finalFileSize += (unsigned int) packet[3 + i] * multiply;
										multiply *= 256;
									}
									if (finalFileSize == fileSize) {
										if (packet[3 + sizeFileSize] == TNAME) {
											if (packet[4 + sizeFileSize] == fileNameSize) {
												for (i = 0; i < fileNameSize; i++) 
													if (packet[5 + sizeFileSize + i] != fileName[i]) return -1;
												return OK;
											}
											else return -1;
										}
										else return -1;
									}
									else return -1;
								}
								else return -1;
							}
							else return -1;
						}
					}
				}
				else return -1;
			}
			else return -1;			
		}
		else return -1;
	}
	return -1;
}