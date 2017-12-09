#include "csapp.h"

void
serve_dynamic( int fd, char *filename, char *cgiargs )
{
	char buf[MAXLINE], *emptylist[] = { NULL };
	
	//Return first part of HTTP response
	sprintf( buf, "HTTP/1.0 200 OK\r\n" );
	rio_writen( fd, buf, strlen(buf) );
	sprintf( buf, "Server: Tiny Web Server\r\n" );
	rio_writen( fd, buf, strlen(buf) );

	if( Fork() == 0 ){
		//Real Server would set all CGI vars here
		setenv( "QUERY_STRING", cgiargs, 1 );
		Dup2( fd, STDOUT_FILENO );
		Execve( filename, emptylist, environ );
	}
	Wait(NULL);
}

