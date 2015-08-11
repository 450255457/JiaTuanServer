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

// ��C����ģ�������IE(http client)����web(http server)����Ϊ

int main()
{

	char szWeb[] = "www.baidu.com"; // !!!ǿ��: ���pcһ��Ҫ��pingͨwww.baidu.com, ����������������оͲ�Ҫ������
	hostent *pHost = gethostbyname(szWeb);

	// �����ip��ַ
	const char* pIPAddr = inet_ntoa(*((struct in_addr *)pHost->h_addr));
	printf("web server ip is : %s\n", pIPAddr);

	// �����������Ϣ
	struct sockaddr_in  webServerAddr;
	webServerAddr.sin_family = AF_INET;
	webServerAddr.sin_addr.s_addr = inet_addr(pIPAddr);
	webServerAddr.sin_port = htons(80);

	// �����ͻ���ͨ��socket
	int sockClient = socket(AF_INET, SOCK_STREAM, 0);

	int nRet = connect(sockClient, (struct sockaddr*)&webServerAddr, sizeof(webServerAddr));
	if (nRet < 0)
	{
		printf("connect error\n");
		return 1;
	}

	// ׼������﷢��http��GET����
	char szHttpRest[1024] = { 0 };
	//sprintf(szHttpRest, "GET /index.html HTTP/1.1\r\nHost:%s\r\nConnection: Keep-Alive\r\n\r\n", szWeb);
	sprintf(szHttpRest, "GET / HTTP/1.1\r\nHost:%s\r\n\r\n", szWeb);
	printf("\n---------------------send is :---------------------\n");
	printf("%s\n", szHttpRest);

	// ����tcp����﷢��http��GET����
	nRet = send(sockClient, szHttpRest, strlen(szHttpRest) + 1, 0);
	if (nRet < 0)
	{
		printf("send error\n");
		return 1;
	}

	// �Ѷ��ﷵ�ص���Ϣ�������ļ�test.txt��
	FILE *fp = fopen("test.txt", "w");
	while (1)
	{
		// ÿ�ν���һ���ֽ�
		char szRecvBuf[21] = { 0 };
		nRet = recv(sockClient, szRecvBuf, 20, 0);
		printf("szRecvBuf = %s\n", szRecvBuf);
		// ���մ���
		if (nRet < 0)
		{
			printf("recv error\n");
			goto LABEL;
		}

		// ���������Ͽ�������
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

		// �Ѷ��ﷵ�ص���Ϣд���ļ�
		fputc(szRecvBuf[0], fp);
	}


LABEL: // ������ʲ�Ҫд������ �ܶ�ͯЬ����д��
	fclose(fp);
	close(sockClient);

	return nRet < 0 ? 1 : 0;
}