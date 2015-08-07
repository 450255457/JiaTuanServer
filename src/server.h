/*****************************************
> File Name : server.h
> Description : server head file
> Author : linden
> Date : 2015-08-07
*******************************************/

#ifndef _SERVER_H
#define _SERVER_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>

#include "locker.h"
#include "thread_pool.h"

#define IPADDRESS	"0.0.0.0"
#define PORT		8090
#define LISTENQ		5
#define FDSIZE		1000
#define EPOLLEVENTS	1000
#define BACKLOG 5		//最大连接数

class CServer
{  
public:  
    CServer();  
    ~CServer();
	
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
