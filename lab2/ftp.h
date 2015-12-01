#ifndef FTP
#define FTP

int ftp_connect( const char* ip, int port);
int ftp_disconnect();

/*read returns the numbers of bytes read and expects the total size of the str array as input*/
int ftp_read(char* str, unsigned long str_total_size);
/*send returns the numbers of bytes written and expects the size of the output msg as input*/
int ftp_send(const char* str, unsigned long str_size);

int ftp_login( const char* user, const char* password);
int ftp_changedir( const char* path);
int ftp_pasv();
int ftp_retr( const char* filename);
int ftp_download( const char* filename);

void ftp_abort();//in case something fails call this funcs to free/close everything related to ftp

#endif