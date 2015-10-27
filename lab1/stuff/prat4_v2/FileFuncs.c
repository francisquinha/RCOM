#include <stdio.h>
#include <stdlib.h>
#include "FileFuncs.h"

int getFileBytes(char* filename,char** dest_buf )
{
	FILE *pFile;

	/*from cplusplus.com*/

	pFile = fopen(filename, "rb");//read,binary
	if (pFile == NULL) { fputs("File error", stderr); exit(1); }

	// obtain file size:
	fseek(pFile, 0, SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);

	// allocate memory to contain the whole file:
	*dest_buf = (char*)malloc(sizeof(char)*lSize);
	if (*dest_buf == NULL) { perror("\ngetFileBytes(1):"); return -1; }

	// copy the file into the buffer:
	size_t result = fread(buffer, 1, lSize, pFile);
	if (result != lSize) {
		perror("\ngetFileBytes(2):"); return -1;
	}

	//close file
	if (fclose(pFile) != OK)
	{
		perror("\nsave2File:");
		return -1;
	}

	return OK;
}


int save2File(char* data,int data_size , char* filename)
{
	FILE *pFile;

	//write: Create an empty file for output operations.
	//If a file with the same name already exists, its contents are discarded and the file is treated as a new empty file.
	if ((pFile = fopen(filename, "wb")) == NULL)  //write,binary
	{
		perror("\nsave2File:");
		return -1;
	}

	fwrite(data, sizeof(char), data_size, pFile); // write 10 bytes to our buffer

	if (fclose(pFile) != OK)
	{
		perror("\nsave2File:");
		return -1;
	}

	return OK;
}