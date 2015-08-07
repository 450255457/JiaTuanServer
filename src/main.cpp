/*****************************************
> File Name : main.cpp
> Description : main function
> Author : linden
> Date : 2015-08-07
*******************************************/

#include <unistd.h>
#include <iostream>

#include "server.h"
#include "public_function.h"

using namespace std;

int main(int argc,char *argv[]){
	CPublicFunction MyPF;
	
	cout << "Server is running..." << endl;
	CServer CS;
	int socketfd;
	socketfd = CS.socket_bind(IPADDRESS,PORT);
	if(listen(socketfd,LISTENQ) < 0){
		cout << "Error->listen:" << strerror(errno) << endl;
		return -1;
	}
//	CS.ev_select(socketfd);
	CS.do_epoll(socketfd);
	return 0;
}
	
