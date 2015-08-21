/*****************************************
> File Name : http_server.h
> Description : http_server head file
> Author : linden
> Date : 2015-08-07
*******************************************/

#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H

#define IPADDRESS	"0.0.0.0"
#define PORT		8090
#define LISTENQ		5
#define FDSIZE		1000
#define EPOLLEVENTS	1000
#define BACKLOG 5		//最大连接数

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
