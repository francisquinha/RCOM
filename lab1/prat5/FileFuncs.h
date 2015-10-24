#ifndef FILEFUNCS
#define FILEFUNCS

//copies file data to *dest_buf array. dest_buf must be dynamic and not initialized
//returns the length of the dest_buf
long getFileBytes(char* filename, char** dest_buf);

int save2File(char* data, long data_size, const char* filename);

#endif /*FILEFUNCS*/