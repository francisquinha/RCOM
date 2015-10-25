#ifndef APPPROTOCOL
#define APPPROTOCOL

int getControlPacket(char control, unsigned int size, unsigned char nameSize, char *name, char *controlPacket);
	
int getInfoPacket(unsigned char N, unsigned int infoSize, char *info, char *infoPacket);

int sendControlPacket(int fd, char *controlPacket, int sizeControlPacket);

int sendInfoPacket(int fd, char *InfoPacket, int sizeInfoPacket);

int sendFile(int fd, unsigned char fileNameSize, char *fileName);

int receiveControlPacket(int fd, char *controlPacket, int *sizeControlPacket);

int receiveInfoPacket(int fd, char *infoPacket, int *sizeInfoPacket);

int receiveFile(int fd);

#endif /*APPPROTOCOL*/