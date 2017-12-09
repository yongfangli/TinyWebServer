/******************************************************
 * tiny.c : A simple, iterative HTTP/1.0 Web server
 * that uses the GET method to server static and
 * dynamic content
 * **************************************************/

#include "csapp.h"
#include "sbuf.h"

#define NTHREADS 300
#define SBUFSIZE 600

void *thread(void *vargp);

void doit( int fd );

sbuf_t sbuf;

int
main( int argc, char **argv )
{
	int i, listenfd, connfd, port, clientlen;
	struct sockaddr_in clientaddr;
	pthread_t tid;
	//Check command line args
	if( argc != 2 ){
		fprintf( stderr, "usage: %s <port>\n", argv[0] );
		exit(1);
	}
	port = atoi( argv[1] );
	sbuf_init(&sbuf, SBUFSIZE);
	listenfd = Open_listenfd( port );

	for(i = 0; i < NTHREADS; i++)
		Pthread_create(&tid, NULL, thread, NULL);

	while(1){
		clientlen = sizeof( clientaddr );
		connfd = Accept( listenfd, (SA *)&clientaddr, &clientlen );
		printf("client %s connected\n", inet_ntoa(clientaddr.sin_addr));
		sbuf_insert(&sbuf, connfd);
	}	
}

void *thread(void *vargp)
{
	Pthread_detach(pthread_self());
	while(1){
		int connfd = sbuf_remove(&sbuf);
		doit(connfd);
		Close(connfd);
	}
}

