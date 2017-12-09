#include "csapp.h"

void get_filetype( char *filename, char *filetype );

void handler(int sig)
{
	printf("*******client connection has closed.***********\n");
	pthread_exit(NULL);
	printf("thread has killed, this shouldn't show up\b");
}

void 
serve_static( int fd, char *filename, int filesize )
{
	int srcfd;
	char *srcp, filetype[MAXLINE], buf[MAXLINE];

	if (signal(SIGPIPE, handler) == SIG_ERR)
		unix_error("SIGPIPE ERROR!!!");

	//Send response headers to client
	get_filetype( filename, filetype );
	sprintf( buf, "HTTP/1.0 200 OK\r\n" );
	sprintf( buf, "%sServer: Tiny Web Server\r\n", buf );
	sprintf( buf, "%sContent-length: %d\r\n", buf, filesize );
	sprintf( buf, "%sContent-type: %s\r\n\r\n", buf, filetype );
	printf("---------writen header-------------\n");
	Rio_writen( fd, buf, strlen(buf) );

	//Send response body to client
	srcfd = Open( filename, O_RDONLY, 0 );
	srcp = Mmap( 0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0 );
	Close( srcfd );
	printf("-----%d---write body-------\n", (int)pthread_self());	
	Rio_writen( fd, srcp, filesize );
	printf("---%d----body writed-----\n", (int)pthread_self());
	Munmap( srcp, filesize );
}

void 
get_filetype( char *filename, char *filetype )
{
	if( strstr( filename, ".html" ) )
		strcpy( filetype, "text/html" );
	else if( strstr( filename, ".gif" ) )
		strcpy( filetype, "image/gif" );
	else if( strstr( filename, ".jpg" ) )
		strcpy( filetype, "image/jpeg" );
	else
		strcpy( filetype, "text/plain" );
}

