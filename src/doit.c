#include "csapp.h"
#include "server.h"

void doit( int fd )

{
	int is_static;
	struct stat sbuf;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	rio_t rio;

	//Read request line and headers
	rio_readInitb( &rio, fd );
	Rio_readlineb( &rio, buf, MAXLINE );
	sscanf( buf, "%s %s %s", method, uri, version );
	if( strcasecmp( method, "GET" ) ){
		clienterror( fd, method, "501", "NOT Implemanted", 
				"Tiny does not implement this method" );
		return;
	}
	read_requesthdrs( &rio );

	//Parse URI from GET request
	is_static = parse_uri( uri, filename, cgiargs );
	if( stat( filename, &sbuf ) < 0 ){
		clienterror( fd, filename, "404", "Not found", 
				"Tiny couldn't find this file" );
		return;
	}

	if( is_static ){
		if( !( S_ISREG(sbuf.st_mode) ) || !( S_IRUSR & sbuf.st_mode ) ){
			clienterror( fd, filename, "403", "Forbidden",
					"Tiny couldn't read the file" );
			return;
		}
		serve_static( fd, filename, sbuf.st_size );
	}
	else{
		if( !( S_ISREG(sbuf.st_mode) ) || !( S_IXUSR & sbuf.st_mode ) ){
			clienterror( fd, filename, "403", "Forbidden",
					"Tiny couldn't run the CGI program" );
			return;
		}
		serve_dynamic( fd, filename, cgiargs );
	}

}	
