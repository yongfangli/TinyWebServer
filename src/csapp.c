#include "csapp.h"

void
unix_error( char *msg )
{
	fprintf( stderr, "**********%s: %s***********\n", msg, strerror(errno) );
	exit(0);
}

void 
posix_error( int code, char *msg)
{
	fprintf(stderr, "*********%s:%s***************\n", msg, strerror(code));
	exit(0);
}

/********************************
 * DNS interface wrappers
 * *****************************/

void 
dns_error( char *msg )
{
	fprintf(stderr, "***********%s: DNS error %d*********\n", msg, h_errno);
	exit(0);
}

struct hostent 
*Gethostbyaddr( const char *addr, int len, int type ){
	struct hostent *p;

	if( (p = gethostbyaddr(addr, len, type)) == NULL )
		dns_error("Gethostbyaddr error");
	return p;
}


void
app_error( char *msg )
{
	fprintf( stderr, "***********%s**********\n", msg );
	exit(0);
}

char 
*Fgets( char *buff, int n, FILE *stream )
{
	char *buf;

	if( ((buf = fgets(buff, n, stream)) == NULL) && ferror(stream) )
		app_error( "Fgets error" );

	return buf;
}

pid_t 
Fork( void )
{
	pid_t pid;

	if( (pid = fork()) <0 )
		unix_error( "Fork error");
	return pid;
}

void 
Execve( const char *filename, char *const argv[], char *const envp[] )
{
	if( execve( filename, argv, envp ) < 0 )
		unix_error("Execve error");
}

pid_t  
Wait( int *status )
{
	pid_t pid;

	if( ( pid = wait(status) ) < 0 )
		unix_error("Wait error");
	return pid;
}

void
Kill( pid_t pid, int signum )
{
	int result;
	if( (result = kill( pid, signum )) < 0 )
		unix_error( "Kill error" );
	exit( 0 );
}

void
Pause( void )
{
	pause();
}

handler_t
*Signal( int signum, handler_t handler )
{
	struct sigaction action, oldaction;
	
	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);	//Block sigs of type being handled
	action.sa_flags = SA_RESTART;	//restart syscalls if possible

	if( sigaction(signum, &action, &oldaction) < 0 )
		unix_error("Signal error");

	return (oldaction.sa_handler);
}

/*************************************
 * Wrappers for Unix I/O routines
 * ***********************************/

int 
Open( char *filename, int flag, mode_t mode )
{
	int rc;
	if( (rc = open( filename, flag, mode )) < 0 )
		unix_error("Open error");
	return rc;
}

ssize_t 
Read( int fd, void *buf, size_t n )
{
	ssize_t rc;
	if( (rc = read(fd, buf, n)) < 0 )
		unix_error("Read error");
	return rc;
}

ssize_t
Write( int fd, const void *buf, size_t n )
{
	ssize_t rc;
	if( (rc = write(fd, buf, n)) < 0 )
		unix_error("Write error");
	return rc;
}

void Close( int fd )
{
	int rc;
	if( (rc = close(fd)) < 0 )
		unix_error("Close error");
}

ssize_t
rio_readn( int fd, void *usrbuf, size_t n )
{
	size_t nleft = n;
	ssize_t nread;
	char *bufp = usrbuf;

	while( nleft > 0 ){
		if( (nread = read( fd, bufp, nleft )) < 0 ){
			if( errno == EINTR )	
				nread = 0;
			else
				return -1;	//errno set by read
		}
		else if( nleft == 0 )
			break;		//all readed
		nleft -= nread;
		bufp += nread;
	}
	return  n - nleft;
}

ssize_t Rio_readn(int fd, void *usrbuf, size_t n)
{
	ssize_t rs;
	if( (rs = rio_readn(fd, usrbuf, n) < 0) )
		unix_error("Rio_readn error\n");
	return rs;
}

ssize_t
rio_writen( int fd, void *usrbuf, size_t n )
{
	size_t nleft = n;
	ssize_t nwrite;
	char *bufp = usrbuf;

	while( nleft > 0 ){
		if( (nwrite = write(fd, bufp, nleft)) < 0 ){
			if( errno == EINTR )
				nwrite = 0;
			else
				return -1;
		}
		else if( nleft == 0 )
			break;
		nleft -= nwrite;
		bufp += nwrite;
	}
	return (n-nleft);
}	

int
Dup2( int fd1, int fd2 )
{
	int rc;
	if( (rc = dup2( fd1, fd2 ) < 0 ) )
		unix_error("Dup2 error");
	return rc;
}

/****************************
 * 带I/O缓冲的RIO版本
 * ************************/
void 
rio_readInitb( rio_t *rp, int fd )
{
	rp->rio_fd = fd;
	rp->rio_cnt = 0;
	rp->rio_bufptr = rp->rio_buf;
}


static ssize_t 
rio_read( rio_t *rp, char *usrbuf, size_t n )
{
	int cnt;
	while( rp->rio_cnt <= 0 ){
		rp->rio_cnt = read( rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf) );
		if( rp->rio_cnt < 0 ){
			if( errno != EINTR )	//if EINTR, then restart, else return -1
				return -1;
		}
		else if( rp->rio_cnt == 0 )		//EOF
			return 0;
		else
			rp->rio_bufptr = rp->rio_buf;	//Reset buffer ptr
	}
	/********************************
	 * Copy min(n, rp->cnt) to user buf
	 * *******************************/
	cnt = n;
	if( rp->rio_cnt < n )
		cnt = rp->rio_cnt;
	memcpy( usrbuf, rp->rio_bufptr, cnt );
	rp->rio_bufptr += cnt;
	rp->rio_cnt -= cnt;
	return cnt;
}
		
ssize_t 
rio_readlineb( rio_t *rp, void *usrbuf, size_t maxlen )
{
	int n, rc;
	char c, *bufp = usrbuf;

	for( n=1; n < maxlen; n++ ){
		if( (rc = rio_read(rp, &c, 1 ) == 1) ){
			*bufp++ = c;
			if( c == '\n' )
				break;
		}
		else if( rc == 0 )
			if( n == 1 )
				return 0;	//EOF, no data read
			else
				break;		//EOF, some data read
		else
			return -1;		//Error
	}
	
	*bufp = 0;		//Set bufp=null
	return n;
}

ssize_t Rio_readlineb(rio_t *rp, void *userbuf, size_t maxlen)
{
	ssize_t rc;
	if( (rc = rio_readlineb(rp, userbuf, maxlen)) < 0 )
		unix_error("Rio_readlineb error\n");
	return rc;
}

ssize_t 
rio_readnb( rio_t *rp, void *usrbuf, size_t n )
{
	size_t nleft = n;
	ssize_t nread;
	char *bufp = usrbuf;

	while( nleft > 0 ){
		if( (nread = rio_read( rp, bufp, nleft )) < 0 ){
			if( errno == EINTR )
				nread = 0;
			else
				return -1;
		}
		else if( nread == 0 )	//EOF
			break;
		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft);
}



void Rio_writen(int fd, void *usrbuf, size_t n)
{
	int rc;
	if((rc = rio_writen(fd, usrbuf, n))!=n){
		printf("************Rio_writen error**************\n");
		pthread_exit(NULL);	//if client closed connect then cancel this thread	
		printf("*********thread %d closed, so this shouldn't show********\n", (unsigned long int)pthread_self());
	}
}

/*********************************************
 * 网络套接字编程
 * ******************************************/
int 
open_clientfd( char *hostname, int port )
{
	int clientfd;
	struct hostent *hp;
	struct sockaddr_in serveraddr;

	if( (clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		return -1;	//Check errno for cause of error

	//Fill in the server's IP and port
	if( (hp = gethostbyname(hostname)) == NULL )
		return -2;	//Check h_errno for cause of error

	bzero( (char *) &serveraddr, sizeof(serveraddr) );
	bcopy( (char *) hp->h_addr_list[0],
			(char *) &serveraddr.sin_addr,
			hp->h_length);
	serveraddr.sin_port = htons(port);

	//Establish a connection with the server
	if( connect(clientfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0 )
	   return -1;
	return clientfd;
}	

int 
open_listenfd( int port )
{
	int listenfd, optval = 1;
	struct sockaddr_in serveraddr;

	//Create a socket descriptor
	if( (listenfd = socket( AF_INET, SOCK_STREAM, 0)) < 0 )
		return -1;
	//Eliminates "Address already in use" error from bind
	if( setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR,
				(const void *)&optval, sizeof(int)) < 0 )
		return -1;
	bzero( (char *) &serveraddr, sizeof(serveraddr) );
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons( (unsigned short) port );
	if( bind(listenfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0 )
		return -1;

	//Make it a listening socket ready to accept connection requests
	if( listen(listenfd, LISTENQ) < 0 )
		return -1;
	return listenfd;
}

int 
Open_listenfd( int port )
{
	int rc;

	if( (rc = open_listenfd(port)) < 0 )
		unix_error("Open_listenfd error");
	return rc;
}

int
Accept( int s, struct sockaddr *addr, socklen_t *addrlen )
{
	int rc;

	if( (rc = accept( s, addr, addrlen )) < 0 )
		unix_error("Accept error");
	return rc;
}

/****************************************
 * Wrapper for memory mapping functions
 * *************************************/
void 
*Mmap( void *addr, size_t len, int prot, int flags, int fd, off_t offset )
{
	void *ptr;

	if( (ptr = mmap( addr, len, prot, flags, fd, offset )) == ( (void *)-1 ) )
		unix_error("mmap error");
	return ptr;
}
void 
Munmap( void *start, size_t length )
{
	if( munmap( start, length ) < 0 )
		unix_error("munmap error");
}

/************************************************************
 * Wrappers for Pthreads thread control functions
 * ********************************************************/
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
		void * (*routine)(void *), void *argp)
{
		int rc;

		if( (rc = pthread_create(tidp, attrp, routine, argp))!=0)
			posix_error(rc, "Pthread_create error");
}

void Ptread_cancel(pthread_t tid)
{
		int rc;

		if( (rc = pthread_cancel(tid)) != 0 )
			posix_error(rc, "Pthread_cancel error");
}

void Pthread_detach(pthread_t tid)
{
	int rc;
	if( (rc = pthread_detach(tid)!=0) )
		posix_error(rc, "Pthread_detach error");
}
			
void Pthread_join(pthread_t tid, void **thread_return)
{
		int rc;

		if( (rc = pthread_join(tid, thread_return)) )
			posix_error(rc, "Pthread_join error");
}
/*****************************************************
 * Wrappers for dynamic storage allocation functions
 * ***************************************************/
 void *Malloc(size_t size)
{
	void *p;
	if( (p = malloc(size)) == NULL ){
		unix_error("Malloc error");
		exit(0);
	}
	return p;
}

void *Calloc(size_t nmemb, size_t size)
{
	void *p;

	if( (p = calloc(nmemb, size)) == NULL)
		unix_error("Calloc error");
	return p;
}

void Free(void *ptr)
{
	free(ptr);
}

/******************************************************
 * Wrapper for Posix semaphore
 * ***************************************************/
void P(sem_t *sem)
{
	if( sem_wait(sem) < 0 )
		unix_error("P error");
}

void V(sem_t *sem)
{
	if(sem_post(sem) < 0)
		unix_error("V error");
}
