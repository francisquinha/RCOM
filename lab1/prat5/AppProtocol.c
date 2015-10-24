#include "AppProtocol.h"
//faltam includes


//================================================================================================================
//AUX FUNCS
//================================================================================================================

/*ESTOU A PRESSUPOR QUE O ARRAY DATA-BUFFER E DINAMICO (USA ALLOCS)*/
int update_received_data(char* data_buffer, int data_buffer_length, char* newdata_buffer, int newdata_buffer_length) {
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

//================================================================================================================
//MAIN FUNCS
//================================================================================================================

#define CD 0b00000000 // campo de controlo no caso de um pacote de dados
#define CS 0b00000001 // campo e controlo no caso de um pacote de start
#define CE 0b00000010 // campo e controlo no caso de um pacote de end
#define TSIZE 0b00000000 // type para pacote de controlo no caso de ser tamanho do ficheiro
#define TNAME 0b00000001 // type para pacote de controlo no caso de ser nome do ficheiro
#define START 1
#define END 2
#define L2
#define L1

struct applicationLayer {
	int fd; /*Descritor correspondente à porta série*/
	int status; /*TRANSMITTER 0 | RECEIVER 1*/
};

int getControlPacket(char type, unsigned int size, unsigned char nameSize, char *name, char *controlPacket) {
	char fileSize[32];
	char n = 0;
	while (size != 0) {
		fileSize[n] = (char) size;
		size >>= 8; // next byte
	    n++;
	}
	controlPacket[0] = ((type == START) ? CS : CE);
	controlPacket[1] = TSIZE;
	controlPacket[2] = n;
	unsigned int i;
	for (i = 0; i < n; i++) controlPacket[3 + i] = fileSize[i];		
	controlPacket[3 + n] = TNAME;
	controlPacket[4 + n] = nameSize;
	for (i = 0; i < nameSize; i++) controlPacket[5 + n + i] = fileSize[i];		
	return 5 + n + nameSize; 	// 1 byte for C, 2 bytes for T and L, n bytes for size, 2 bytes for T and L, nameSize bytes for name
}
	
int getInfoPacket(unsigned char N, char *info, char *infoPacket) {
	infoPacket[0] = CD;
	infoPacket[1] = N;
	infoPacket[2] = L2;			// size of info = L2 * 256 + L1
	infoPacket[3] = L1;
	unsigned int i;
	for (i = 0; i < L2 * 256 + L1; i++) infoPacket[4 + i] = info[i];		
	return 4 + L2 * 256 + L1;	// 1 byte for C, 1 byte for N, 2 bytes for L2 and L1, L2 * 256 + L1 bytes for info
}

int sendControlPacket(int fd, char *controlPacket, int sizeControlPacket) {
	if (llwrite(fd, controlPacket, sizeControlPacket)) > 0) return OK;
	return -1;
}

int sendInfoPacket() {
	return OK;
}

int sendFile(int fd) {
	char controlPacket[264]; /* maximum size of control packet: 1 byte for C, 2 bytes for T and L, 4 bytes for size, 2 bytes for T and L, 255 bytes for name */
	int sizeControlPacket;
	char infoPacket[65540]; /* maximum size of info packet: 1 byte for C, 1 byte for N, 2 bytes for L2 and L1, 255 * 256 + 255 bytes for info */
	int sizeControlPacket;
	unsigned int fileSize = getFileSize();
	char fileName[255];
	unsigned char fileNameSize = getFileName(fileName);
	sizeControlPacket = getControlPacket(START, fileSize, fileNameSize, fileName, controlPacket);
	sendControlPacket(fd, controlPacket, sizeControlPacket);
	
	
}



int receiveControlPacket()
{
	return OK;
}

int receiveInfoPacket()
{
	return OK;
}