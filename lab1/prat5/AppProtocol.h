#ifndef APPPROTOCOL
#define APPPROTOCOL

#define CD 0b00000000 // campo de controlo no caso de um pacote de dados
#define CS 0b00000001 // campo e controlo no caso de um pacote de start
#define CE 0b00000010 // campo e controlo no caso de um pacote de end
#define TSIZE 0b00000000 // type para pacote de controlo no caso de ser tamanho do ficheiro
#define TNAME 0b00000001 // type para pacote de controlo no caso de ser nome do ficheiro
#define START 1
#define END 2
#define L2 255
#define L1 255
#define MAX_CTRL_P 264 /* maximum size of control packet: 1 byte for C, 2 bytes for T and L, 4 bytes for size, 2 bytes for T and L, 255 bytes for name */
#define MAX_INFO_P 65540 /* maximum size of info packet: 1 byte for C, 1 byte for N, 2 bytes for L2 and L1, 255 * 256 + 255 bytes for info */
#define MAX_TRY 3

int getControlPacket(char control, unsigned int size, unsigned char nameSize, char *name, char *controlPacket);
	
int getInfoPacket(unsigned char N, unsigned int infoSize, char *info, char *infoPacket);

int sendControlPacket(int fd, char *controlPacket, int sizeControlPacket);

int sendInfoPacket(int fd, char *infoPacket, int sizeInfoPacket);

int sendFile(int fd, unsigned char fileNameSize, char *fileName);

int receiveControlPacket(int fd, char *controlPacket, int *sizeControlPacket);

int receiveInfoPacket(int fd, char *infoPacket, int *sizeInfoPacket);

int receiveFile(int fd);

#endif /*APPPROTOCOL*/