#include "ftp.h"
#include "socket.h"
#include "utilities.h"

#define MAX_STRING_SIZE 500

int control_socket_fd; 
int data_socket_fd;

//------------------------------------------------------------------------
// CONECT AND DISCONECT
#if 1

int ftp_connect( const char* ip, int port) {
	
	int socket_fd;
	char read_bytes[MAX_STRING_SIZE];

	//open control socket
	if ((socket_fd = connect_socket_TCP(ip, port)) < 0)
	{
		printf("ftp_connect: Failed to connect socket\n");
		return 1;
	}

	control_socket_fd = socket_fd;
	data_socket_fd 	  = 0;

	//Try to read with control socket
	if (ftp_read(read_bytes, sizeof(read_bytes)))
	{
		printf("ftp_connect: Failed to read\n");
		return 1;
	}

	return 0;
}

int ftp_disconnect() {
	char aux[MAX_STRING_SIZE];

	//read discnnect
		if (ftp_read(aux, sizeof(aux))) {
		printf("ftp_disconnect: Failed to disconnect\n");
		return 1;
	}
	//send disconnect 
	sprintf(aux, "QUIT\r\n");
	if (ftp_send(aux, strlen(aux))) {
		printf("ftp_disconnect: Failed to output QUIT");
		return 1;
	}
	
	close(control_socket_fd);

	return 0;
}

#endif

//------------------------------------------------------------------------
// READ AND SEND
#if 1

int ftp_read(char* str,unsigned long str_total_size)
{
//no need for active waiting and user won't notice (-__-) i think...
//may fail if the response doesn't come right away
//could use a loop instead, but when should it be terminated ???
sleep(1);
		
int num_bytes_read=0;
	if ( (num_bytes_read = read(control_socket_fd, str, str_size)<0)
	{
		printf("ftp_read: Failed to read from data socket\n");
		return -1;
	}
	return num_bytes_read;
}

int ftp_send( const char* str,unsigned long str_size)
{
	int bytes;

	if ((bytes = write(control_socket_fd, str, str_size)) < 0) {
		perror("ftp_send: Failed to write bytes\n");
		return 1;
	}
	return bytes;
}

#endif

//------------------------------------------------------------------------
// MAIN OPERATIONS
#if 1

int ftp_login( const char* user, const char* password) {
	
	char aux[MAX_STRING_SIZE];

	//send username
	sprintf(aux, "user %s\r\n", user);
	if (ftp_send( aux, strlen(aux))) {
		printf("ERROR: ftp_send failure.\n");
		return 1;
	}
	//receive answer to username
	if (ftp_read( aux, sizeof(aux))) {
		printf(	"ftp_login:Bad response to user\n");
		return 1;
	}

	//send password
	memset(aux, 0, sizeof(aux));//reuse 2send
	sprintf(aux, "pass %s\r\n", password);
	if (ftp_send( aux, strlen(aux))) {
		printf("ftp_login: failed to send password.\n");
		return 1;
	}
	//receive answer to password
	if (ftp_read( aux, sizeof(aux))) 
	{
		printf(	"ftp_login:Bad response to pass\n");
		return 1;
	}

	return 0;
}

int ftp_changedir(const char* path) {
	
	char aux[MAX_STRING_SIZE];

	//send cwd command
	sprintf(aux, "CWD %s\r\n", path);
	if (ftp_send(aux, strlen(aux))) {
		printf("ftp_changedir:Failed to send\n");
		return 1;
	}

	//get response
	if (ftp_read(aux, sizeof(aux))) {
		printf("ftp_changedir:Failed to get a valid response\n");
		return 1;
	}

	return 0;
}

#define DEBUG_PASV 1
int ftp_pasv() {

	char aux[MAX_STRING_SIZE] = "PASV\r\n";
	
	//send pasv msg
	if (ftp_send(aux, strlen(aux))) {
		printf("ftp_pasv: Failed to enter in passive mode\n");
		return 1;
	}
	
	//receive response
	if (ftp_read(aux, sizeof(aux))) {
		printf("ftp_pasv: Failed to receive information to enter passive mode\n");
		return 1;
	}

		DEBUG_SECTON(DEBUG_PASVprintf("pasv():received:%s\n",aux);
	);
	
	// info was received. scan it
	int ip_bytes[4];
	int ports[2];
		
	if ((sscanf(aux, "%*[^(](%d,%d,%d,%d,%d,%d)",
	ip_bytes,ip_bytes+1, ip_bytes+2, ip_bytes+3, ports, ports+1)) 
		!=6 ) 
	{
		printf("ftp_pasv: Cannot process received data, must receive 6 bytes\n");
		return 1;
	}
	
	// reuse aux and get ip 
	memset(aux, 0, sizeof(aux));
	if ((sprintf(aux, "%d.%d.%d.%d",
	ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]))
		!= 4) 
	{
		printf("ftp_pasv: Cannot compose ip address\n");
		return 1;
	}

		DEBUG_SECTON(DEBUG_PASV,printf("pasv():ip:%s\n",aux);
	);
	
	// calculate port
	int portResult = ports[0] * 256 + ports[1];

	printf("IP: %s\n", aux);
	printf("PORT: %d\n", portResult);

	if ((data_socket_fd = connectSocket(aux, portResult)) < 0) {
		printf(	"ftp_pasv: Failed to connect data socket\n");
		return 1;
	}

	return 0;
}

int ftp_retr(const char* filename) {
	char aux[MAX_STRING_SIZE];

	//send retr
	sprintf(aux, "RETR %s\r\n", filename);
	if (ftpSend(ftp, aux, strlen(aux))) {
		printf("ftp_retr: Failed to send \n");
		return 1;
	}

	//get respones
	if (ftpRead(ftp, aux, sizeof(aux))) {
		printf("ftp_retr: Failed to get response\n");
		return 1;
	}
	
	return 0;
}

int ftp_download(ftp* ftp, const char* filename) {
	
	FILE* file;
	int bytes;

	//create n open file
	if (!(file = fopen(filename, "w"))) {
		printf("ftp_download: Failed to create/open file\n");
		return 1;
	}

	//read received data in data socket until there is nothing more 2 read
	//??? is it possible for the stream to stop temporarily and recive 0 as result
	//without outputting everything ??? hmmm...
	char buf[MAX_STRING_SIZE];
	while ((bytes = read(data_socket_fd, buf, sizeof(buf)))) {
		if (bytes < 0) {
			perror("ftp_download: Failed to receive from data socket\n");
			fclose(file);
			return 1;
		}
		
		//output received bytes to file
		if ((bytes = fwrite(buf, bytes, 1, file)) < 0) {
			perror("ftp_download: Failed to write data in file\n");
			return 1;
		}
	}

	//close file and data socket
	fclose(file);
	close(data_socket_fd);

	return 0;
}

void ftp_abort()
{
	//... not really useful thus far ...
	if(data_socket_fd) close(data_socket_fd);
	if(control_socket_fd) close(control_socket_fd);
	
}

#endif
