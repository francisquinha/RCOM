#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <errno.h> 
#include <sys/types.h>
#include "utilities.h"

//VARS AND STRUCTS --------------------------------------------------------------------------------------
#define MAX_STRING_SIZE 200
struct /*???*/Info{
  char username[MAX_STRING_SIZE];
  char password[MAX_STRING_SIZE];
  char host_name[MAX_STRING_SIZE];
  char url_path[MAX_STRING_SIZE];
  char filename[MAX_STRING_SIZE];
  char ip[MAX_STRING_SIZE];
};

int control_socket_fd; 
int data_socket_fd;

//AUX FUNCS CODE --------------------------------------------------------------------------------------
int parse(char *str, struct Info* info) {
  
  //http://docs.roxen.com/pike/7.0/tutorial/strings/sscanf.xml
	if(4 != sscanf(str, "ftp://[%[^:]:%[^@]@]%[^/]/%s\n", info->username, info->password, info->host_name, info->url_path)) {
		return 1;
	}

  //http://stackoverflow.com/questions/32822988/get-the-last-token-of-a-string-in-c
      char *last = strrchr(info->url_path, '/') ;
      if(last!=NULL) memcpy(info->filename, last+1, strlen(last)+1);
      else strcpy(info->filename,info->url_path);
      
	return 0;
}

int get_ip(struct Info* info) {
	struct hostent* host;

	if ((host = gethostbyname(info->host_name)) == NULL) {
		herror("gethostbyname");
		return 1;
	}

	char* ip = inet_ntoa(*((struct in_addr *) host->h_addr));
	strcpy(info->ip, ip);

	printf("Host name  : %s\n", host->h_name);
	printf("IP Address : %s\n", info->ip);
	
	return 0;
}


//MAIN --------------------------------------------------------------------------------------
#define DEBUG_ALL 1
int main(int argc,char **argv)
{
  struct Info info;
  
  // ftp message composition: ftp://[<user>:<password>@]<host>/<url-path>
  //ftp://[up201306619:thisisaverylongpassword9]gnomo.fe.up.pt/Music/test.txt
  
	// URL

	//parse
	if(parse(argv[1],&info)!=OK)
	{
	  printf("\nINVALID ARGUMENT! couldn't be parsed properly.\n");
	  return 1;
	}
	DEBUG_SECTION(DEBUG_ALL,
	printf("\nuser:%s\n",info.username);
	printf("pass:%s\n",info.password);
	printf("host:%s\n",info.host_name);
	printf("urlpath:%s\n",info.url_path);
	printf("filename:%s\n",info.filename);
	);
  
	// get ip
	get_ip(&info);
	
	//done step 1...

	// FTP steps (got from watching prev works)

	// Verifying username and password

	// Sending credentials to server

	// Changing directory

	// Entry in passive mode

	// Begins transmission of a file

	// file transfer

	// Disconnecting from server


	return 0;
}