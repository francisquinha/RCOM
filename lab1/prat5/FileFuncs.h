#ifndef FILEFUNCS
#define FILEFUNCS

//copies file data to *dest_buf array. dest_buf must be dynamic and not initialized
//returns the length of the dest_buf
int getFileBytes(char* filename, char** dest_buf);

int save2File(char* data, int data_size, char* filename);

#endif FILEFUNCS