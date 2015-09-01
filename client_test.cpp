/*****************************************
> File Name : client_test.cpp
> Description : C ECHO client example using sockets
> Author : linden
> Date : 2015-08-07
*******************************************/

#include <unistd.h>
#include <stdio.h>		//printf
#include <string.h>		//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr

#define PORT		8090
#define IPADDRESS	"127.0.0.1"

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in server;
	char message[1024], server_reply[2048];

	//Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	server.sin_addr.s_addr = inet_addr(IPADDRESS);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);

	//Connect to remote server
	if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}

	puts("Connected\n");

	//keep communicating with server
	
		//sprintf(message, "POST / HTTP/1.1\r\nHost: www.qujiatuan.com\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 12\r\nConnection:close\r\n\r\nsn=123&n=asa");
		sprintf(message, "sn=123&n=asa");
		printf("Send message : %s\n",message);

		//Send some data
		if (send(sockfd, message, strlen(message), 0) < 0)
		{
			puts("Send failed");
			return -1;
		}
		while (1)
		{
		//Receive a reply from the server
		if (recv(sockfd, server_reply, 2000, 0) < 0)
		{
			puts("recv failed");
			break;
		}

		puts("Server reply :");
		puts(server_reply);
	}

	close(sockfd);
	return 0;
}
/*
GET / path HTTP / 1.1
Header1: Value1
Header2 : Value2
Header3 : Value3

POST /path HTTP/1.1
Header1: Value1
Header2: Value2
Header3: Value3

body data goes here...

HTTP响应的格式：
200 OK
Header1: Value1
Header2: Value2
Header3: Value3

body data goes here...

*/
