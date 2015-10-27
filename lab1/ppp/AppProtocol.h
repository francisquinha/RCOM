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
#define MAX_TRY 1

/**
@return 0(OK) if no errors/problems. -1 if failed to send everything, -2 failed on sending start
*/
int sendFile(int fd, unsigned char fileNameSize, const char *fileName, unsigned  int image_bytes_length, const char *image_bytes, volatile unsigned  int* out_already_sent_bytes);

/**
@param out_imagename will get the name of the file to be used later. must be a 255 char array
@param out_imagebuffer will get the image bytes. Must be a dynamic array and be freed outside ths method
@return 0(OK) if no errors/problems. something else otherwise. -1 failed to receive everything, -1 failed to recive start
*/
int receiveFile(int fd, char* out_imagename, char** out_imagebuffer, volatile unsigned int* out_image_buffer_length, volatile unsigned  int* out_already_received_imgbytes);

#endif /*APPPROTOCOL*/