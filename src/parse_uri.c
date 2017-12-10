#include "csapp.h"

#define ROOT "/Users/l047/Code/git/TinyWebServer"

int 
parse_uri( char *uri, char *filename, char *cgiargs )
{
	char *ptr;

	if( !strstr( uri, "cgi-bin" ) ){
		strcpy( cgiargs, "" );
		strcpy( filename, ROOT);
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


