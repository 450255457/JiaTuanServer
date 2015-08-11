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
#include <netinet/in.h>
#include <arpa/inet.h>

#define IPSTR "61.147.124.120"
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

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		printf("创建网络连接失败,本线程即将终止---socket error!\n");
		exit(0);
	};

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, IPSTR, &servaddr.sin_addr) <= 0 ){
		printf("创建网络连接失败,本线程即将终止--inet_pton error!\n");
		exit(0);
	};

	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		printf("连接到服务器失败,connect error!\n");
		exit(0);
	}
	printf("与远端建立了连接\n");

	//发送数据
	memset(str2, 0, 4096);
	strcat(str2, "qqCode=474497857");
	str=(char *)malloc(128);
	len = strlen(str2);
	sprintf(str, "%d", len);

	memset(str1, 0, 4096);
	strcat(str1, "POST /webservices/qqOnlineWebService.asmx/qqCheckOnline HTTP/1.1\n");
	strcat(str1, "Host: www.webxml.com.cn\n");
	strcat(str1, "Content-Type: application/x-www-form-urlencoded\n");
	strcat(str1, "Content-Length: ");
	strcat(str1, str);
	strcat(str1, "\n\n");

	strcat(str1, str2);
	strcat(str1, "\r\n\r\n");
	printf("%s\n",str1);

	ret = write(sockfd,str1,strlen(str1));
	if (ret < 0) {
		printf("发送失败！错误代码是%d，错误信息是'%s'\n",errno, strerror(errno));
		exit(0);
	}else{
		printf("消息发送成功，共发送了%d个字节！\n\n", ret);
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
		printf("--------------->2");

		//if (h == 0) continue;
		if (h < 0) {
			close(sockfd);
			printf("在读取数据报文时SELECT检测到异常，该异常导致线程终止！\n");
			return -1;
		};

		if (h > 0){
			memset(buf, 0, 4096);
			i= read(sockfd, buf, 4095);
			if (i==0){
				close(sockfd);
				printf("读取数据报文时发现远端关闭，该线程终止！\n");
				return -1;
			}

			printf("%s\n", buf);
		}
	}
	close(sockfd);


	return 0;
}

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



