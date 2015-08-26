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

// 用C代码模拟浏览器IE(http client)访问web(http server)的行为

int main()
{

	//char szWeb[] = "www.baidu.com";
	char szWeb[] = "127.0.0.1";
	hostent *pHost = gethostbyname(szWeb);

	// 度娘的ip地址
	const char* pIPAddr = inet_ntoa(*((struct in_addr *)pHost->h_addr));
	printf("web server ip is : %s\n", pIPAddr);

	struct sockaddr_in  webServerAddr;
	webServerAddr.sin_family = AF_INET;
	webServerAddr.sin_addr.s_addr = inet_addr(pIPAddr);
	webServerAddr.sin_port = htons(8090);

	// 创建客户端通信socket
	int sockClient = socket(AF_INET, SOCK_STREAM, 0);

	int nRet = connect(sockClient, (struct sockaddr*)&webServerAddr, sizeof(webServerAddr));
	if (nRet < 0)
	{
		printf("connect error\n");
		return 1;
	}

	// 准备向度娘发送http的GET请求
	char szHttpRest[1024] = { 0 };
	//sprintf(szHttpRest, "GET /index.html HTTP/1.1\r\nHost:%s\r\nConnection: Keep-Alive\r\n\r\n", szWeb);
	//sprintf(szHttpRest, "GET / HTTP/1.1\r\nHost:%s\r\n\r\n", szWeb);
	sprintf(szHttpRest, "GET / HTTP/1.1\r\nHost:%s\r\nbody:{\"id\":1,\"name\":\"kurama\"}\r\n\r\n", szWeb);
	printf("\n---------------------sendbuf is :---------------------\n");
	printf("%s\n", szHttpRest);

	// 利用tcp向度娘发送http的GET请求
	nRet = send(sockClient, szHttpRest, strlen(szHttpRest) + 1, 0);
	if (nRet < 0)
	{
		printf("send error\n");
		return 1;
	}

	// 把度娘返回的信息保存在文件test.txt中
	FILE *fp = fopen("test.txt", "w");
	while (1)
	{
		char szRecvBuf[2049] = { 0 };
		nRet = recv(sockClient, szRecvBuf, 2048, 0);
		printf("szRecvBuf = %s\n", szRecvBuf);
		// 接收错误
		if (nRet < 0)
		{
			printf("recv error\n");
			goto LABEL;
		}

		// 度娘主动断开了连接
		if (0 == nRet)
		{
			printf("connection has beed closed by web server\n");
			goto LABEL;
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


LABEL: // 这个单词不要写错啦， 很多童鞋容易写错
	fclose(fp);
	close(sockClient);

	return nRet < 0 ? 1 : 0;
}