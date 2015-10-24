#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"
#include "FileFuncs.h"

#define OK 0

/*dest_buf must be freed outside!*/
long getFileBytes(char* filename,char** dest_buf )
{
	FILE *pFile;

	/*adapted from cplusplus.com examples*/

	pFile = fopen(filename, "rb");//read,binary
	if (pFile == NULL) { fputs("File error", stderr); exit(1); }

	// obtain file size:
	fseek(pFile, 0, SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	
	// allocate memory to contain the whole file:
	*dest_buf = (char*)malloc(sizeof(char)*lSize);
	if (*dest_buf == NULL) { perror("\ngetFileBytes(1):"); return -1; }

	printf("\nCCC\n");
	
	// copy the file into the buffer:
	size_t result = fread(*dest_buf, 1, lSize, pFile);
	if (result != lSize) {
		perror("\ngetFileBytes(2):"); return -1;
	}

	//close file
	if (fclose(pFile) != OK)
	{
		perror("\nsave2File:");
		return -1;
	}

	return lSize;
}


int save2File(char* data,long data_size , const char* filename)
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

int main(int argc, char** argv)
{
  printf("\n-1-\n");
  char get_path[100];
  gets(get_path);
  char* image_path;
  while(TRUE){
  image_path=realpath(get_path,NULL);
  if(image_path!=NULL) break;
  printf("\nInvalid path/file. Please choose another.\n");
    gets(get_path);
  }
  
   printf("\n-2-\n") ;
  char* image_bytes;
  
  printf("\n%s\n",image_path);
  //free(image_path);
  //return 0;
long imageSize;
  if( (imageSize=getFileBytes(image_path,&image_bytes))<0)
    exit(1);
    printf("\n-3-\n");

  
  char copyname[]="copy.jpg";
  if(save2File(image_bytes,imageSize, copyname)!=OK)
    exit(1);
    printf("\n-4-\n");
  free(image_bytes);
  
  return 0;
}