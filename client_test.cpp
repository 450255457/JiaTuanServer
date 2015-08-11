/*****************************************
> File Name : client_test.cpp
> Description : the test client
> Author : linden
> Date : 2015-08-07
*******************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 80
#define BUFSIZE 1024

int main(int argc, char **argv)
{
	int sockfd, ret, i, h;
	struct sockaddr_in servaddr;
	char str1[4096], str2[4096], buf[BUFSIZE], *str;
	socklen_t len;
	fd_set   t_set1;
	struct timeval  tv;
	char szWeb[] = "www.baidu.com";
	hostent *pHost = gethostbyname(szWeb);

	// 度娘的ip地址  
	const char* pIPAddr = inet_ntoa(*((struct in_addr *)pHost->h_addr));
	printf("web server ip is : %s\n", pIPAddr);
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		printf("创建网络连接失败,本线程即将终止---socket error!\n");
		exit(0);
	};

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, pIPAddr, &servaddr.sin_addr) <= 0){
		printf("创建网络连接失败,本线程即将终止--inet_pton error!\n");
		exit(0);
	};

	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		printf("连接到服务器失败,connect error!\n");
		exit(0);
	}
	printf("connect success!\n");

	//发送数据
	char *GET = "GET www.baidu.com HTTP/1.1 \r\n\r\n"
		"Host: www.baidu.com \r\n"
		"Accept: */* \r\n"
		"Pragma:no-cache \r\n"
		"User-Agent:Mozilla/4.0 \r\n";

	ret = write(sockfd, GET, strlen(GET));
	if (ret < 0) {
		printf("Error,%d,error_info = %s'\n",errno, strerror(errno));
		exit(0);
	}else{
		printf("send size:%d\n\n", ret);
	}

	FD_ZERO(&t_set1);
	FD_SET(sockfd, &t_set1);

	while(1){
		sleep(2);
		tv.tv_sec= 0;
		tv.tv_usec= 0;
		h= 0;
		printf("--------------->1");
		h= select(sockfd +1, &t_set1, NULL, NULL, &tv);
		printf("--------------->2,h = %d\n",h);

		FILE *fp = fopen("test.txt", "w");
		int nRet = 0;
		while (1)
		{
			// 每次接收一个字节  
			char szRecvBuf[2] = { 0 };
			nRet = recv(sockfd, szRecvBuf, 1, 0);

			// 接收错误  
			if (nRet < 0)
			{
				printf("recv error\n");
				fclose(fp);
				closesocket(sockfd);
			}

			// 度娘主动断开了连接  
			if (0 == nRet)
			{
				printf("connection has beed closed by web server\n");
				fclose(fp);
				closesocket(sockfd);
			}

			static int flag = 0;
			if (0 == flag)
			{
				printf("writing data to file\n");
				flag = 1;
			}

			// 把度娘返回的信息写入文件  
			fputc(szRecvBuf[0], fp);
		}
	}


	return 0;
}














//以下为注释
#if 0
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

using namespace std;

int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

void addfd(int epoll_fd,int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLOUT | EPOLLET | EPOLLERR;
    epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}

bool write_nbytes( int sockfd, const char* buffer, int len )
{
    int bytes_write = 0;
    printf( "write out %d bytes to socket %d\n", len, sockfd );
    while( 1 ) 
    {   
        bytes_write = send( sockfd, buffer, len, 0 );
        if ( bytes_write == -1 )
        {   
            return false;
        }   
        else if ( bytes_write == 0 ) 
        {   
            return false;
        }   

        len -= bytes_write;
        buffer = buffer + bytes_write;
        if ( len <= 0 ) 
        {   
            return true;
        }   
    }   
}

bool read_once( int sockfd, char* buffer, int len )
{
    int bytes_read = 0;
    memset( buffer, '\0', len );
    bytes_read = recv( sockfd, buffer, len, 0 );
    if ( bytes_read == -1 )
    {
        return false;
    }
    else if ( bytes_read == 0 )
    {
        return false;
    }
	printf( "read in %d bytes from socket %d with content: %s\n", bytes_read, sockfd, buffer );

    return true;
}

void start_conn( int epoll_fd, int num, const char* ip, int port )
{
    int ret = 0;
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );

    for ( int i = 0; i < num; ++i )
    {
        sleep( 1 );
        int sockfd = socket( PF_INET, SOCK_STREAM, 0 );
        printf( "create 1 sock\n" );
        if( sockfd < 0 )
        {
            continue;
        }

        if (  connect( sockfd, ( struct sockaddr* )&address, sizeof( address ) ) == 0  )
        {
            printf( "build connection %d\n", i );
            addfd( epoll_fd, sockfd );
        }
    }
}

void close_conn( int epoll_fd, int sockfd )
{
    epoll_ctl( epoll_fd, EPOLL_CTL_DEL, sockfd, 0 );
    close( sockfd );
}

int main( int argc, char* argv[] )
{	
	cout << "./Client ip port num" << endl;

	char *request = "GET http://www.ip.com/ HTTP/1.1 \r\n"
		"Host: www.ip.com \r\n"
		"Accept: */* \r\n"
		"Pragma:no-cache \r\n"
		"User-Agent:Mozilla/4.0 \r\n";
    int epoll_fd = epoll_create( 100 );
    start_conn( epoll_fd,2,"127.0.0.1", 8090);
    epoll_event events[ 10000 ];
    char buffer[ 2048 ];
    while ( 1 )
    {
        int fds = epoll_wait( epoll_fd, events, 10000, 2000 );
        for ( int i = 0; i < fds; i++ )
        {   
            int sockfd = events[i].data.fd;
            if ( events[i].events & EPOLLIN )
            {   
                if ( ! read_once( sockfd, buffer, 2048 ) )
                {
                    close_conn( epoll_fd, sockfd );
                }
                struct epoll_event event;
                event.events = EPOLLOUT | EPOLLET | EPOLLERR;
                event.data.fd = sockfd;
                epoll_ctl( epoll_fd, EPOLL_CTL_MOD, sockfd, &event );
            }
            else if( events[i].events & EPOLLOUT ) 
            {
				if (!write_nbytes(sockfd, request, strlen(request)))
                {	
                    close_conn( epoll_fd, sockfd );
                }
                struct epoll_event event;
                event.events = EPOLLIN | EPOLLET | EPOLLERR;
                event.data.fd = sockfd;
                epoll_ctl( epoll_fd, EPOLL_CTL_MOD, sockfd, &event );
            }
            else if( events[i].events & EPOLLERR )
            {
                close_conn( epoll_fd, sockfd );
            }
        }
    }
}
#endif



