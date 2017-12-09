#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/wait.h>

/* external variables */
extern char **environ;

#define MAXLINE 8192 /* Max text line length */
#define MAXBUF 8192

void unix_error( char *msg );

void app_error( char *msg );

void posix_error(int code, char *msg);
char *Fgets( char *, int, FILE * );

pid_t Fork( void );

void Kill( pid_t pid, int signum );

void Pause(void);

typedef void handler_t(int);
handler_t *Signal( int signum, handler_t *handler );

/**************************
 * DNS interface wrappers
 * ***********************/
void dns_error( char *msg );
struct hostent *Gethostbyaddr( const char *addr, int len, int type );

/*********************************
 * Wrappers for Unix I/O functions
 * *******************************/

int Open( char *filename, int flag, mode_t mode );
ssize_t Read( int fd, void *buf, size_t n );
ssize_t Write( int fd, const void *buf, size_t n );
void Close( int fd );
int Dup2( int fd1, int fd2 );
void Execve( const char *filename, char *const argv[], char *const envp[] );
pid_t Wait( int *status );
/***********************************
 * 不带缓冲的RIO版本
 * ********************************/
ssize_t rio_readn( int fd, void *buf, size_t n );
ssize_t rio_writen( int fd, void *buf, size_t n );

void Rio_writen(int fd, void *usrbuf, size_t n);
/***********************************
 * 带I/O缓冲的RIO版本
 *********************************/
#define RIO_BUFSIZE 8192
typedef struct {
	int rio_fd;		//Descritor for this internal buf
	int rio_cnt;	//Unread bytes in internal buf
	char *rio_bufptr;//Next unread byte in internal buf
	char rio_buf[RIO_BUFSIZE];//Internal buf
} rio_t;

void rio_readInitb( rio_t *rp, int fd );
static ssize_t rio_read( rio_t *rp, char *usrbuf, size_t n );
ssize_t rio_readlineb( rio_t *rp, void *usrbuf, size_t maxlen );
ssize_t Rio_readlineb( rio_t *, void *, size_t);
ssize_t rio_readnb( rio_t *rp, void *usrbuf, size_t n );
ssize_t Rio_readnb( rio_t *, void *, size_t);

/*************************
 * 网络套接字编程
 * **********************/
typedef struct sockaddr SA;
#define LISTENQ 1024
int open_clientfd( char *hostname, int port );
int Open_listenfd( int port );
int Accept( int s, struct sockaddr *addr, socklen_t *addrlen );

/***************************************
 * Wrapper for memory mapping functions
 * *************************************/
 void *Mmap( void *addr, size_t len, int prot, int flags, int fd, off_t offset );
 void Munmap( void *start, size_t length );

 /***********************************************************
  * Wrappers for Pthreads thread control functions
  * ********************************************************/
 void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
		 void * (*routine)(void *), void *argp);

void Ptread_cancel(pthread_t tid);

void Pthread_join(pthread_t tid, void **thread_return);

void Pthread_detach(pthread_t tid);


/******************************************************
 * Wrappers for dynamic storage allocation functions
 * ***************************************************/
void *Malloc(size_t size);

void Free(void *ptr);

void *Calloc(size_t nmemb, size_t size);

/***************************************************
 * Wrapper for Posix semaphore
 * ***********************************************/
void P(sem_t *sem);

void V(sem_t *sem);
