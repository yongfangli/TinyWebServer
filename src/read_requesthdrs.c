#include "csapp.h"

void 
read_requesthdrs( rio_t *rp )
{
	char buf[MAXLINE];

	Rio_readlineb( rp, buf, MAXLINE );
	while( strcmp( buf, "\r\n" ) ){
		Rio_readlineb( rp, buf, MAXLINE );
		printf( "%s", buf );
	}
	
	return;
}
