#include "csapp.h"

void uri_router( char *uri, char *filename );

int 
parse_uri( char *uri, char *filename, char *cgiargs )
{
	char *ptr;

	if( !strstr( uri, "cgi-bin" ) ){
		strcpy( cgiargs, "" );
		uri_router( uri, filename );
		strcat( filename, uri );
		if( uri[strlen(uri)-1] == '/' )
			strcpy( filename, "../html/home.html" );
		return 1;
	}
	else{
		ptr = index( uri, '?' );
		if( ptr ){
			strcpy( cgiargs, ptr+1 );
			*ptr = '\0';
		}
		else
			strcpy( cgiargs, "" );
		strcpy( filename, "." );
		strcat( filename, uri );
		return 0;
	}
}

void uri_router( char *uri , char *filename ){
	memset(filename, 0, sizeof(filename));
	if( strstr( uri, ".html" ) )
		strcpy( filename, "../html" );
	else 
		strcpy( filename, ".." );
}

