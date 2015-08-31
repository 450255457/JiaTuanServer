/*****************************************
> File Name : server_threaded.h
> Description : server_threaded head file
> Author : linden
> Date : 2015-08-31
*******************************************/

#ifndef _SERVER_THREADED_H
#define _SERVER_THREADED_H

/* Port to listen on. */
#define SERVER_PORT 8090
/* Connection backlog (# of backlogged connections to accept). */
#define CONNECTION_BACKLOG 8
/* Socket read and write timeouts, in seconds. */
#define SOCKET_READ_TIMEOUT_SECONDS 10
#define SOCKET_WRITE_TIMEOUT_SECONDS 10
/* Number of worker threads.  Should match number of CPU cores reported in /proc/cpuinfo. */
#define NUM_THREADS 8

class CHttpServer
{  
public:  
	CHttpServer();
	~CHttpServer();
	
	int socket_bind(const char *ip,int nPort);
	int do_epoll(int socketfd);
	void add_event( int epollfd, int fd, bool enable_et);
	void edge_trigger( epoll_event* events, int nfds, int epollfd, int socketfd );
	int setnonblocking( int fd );
	void addfd(int epollfd,int fd,bool oneshot);
	int ev_select(int socketfd);
	void reset_oneshot(int epollfd,int fd);
		
private:
	locker m_arglocker;			//传入队列的锁
};  

#endif
