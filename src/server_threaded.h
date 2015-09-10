/*****************************************
> File Name : server_threaded.h
> Description : server_threaded head file
> Author : linden
> Date : 2015-08-31
*******************************************/

#ifndef _SERVER_THREADED_H
#define _SERVER_THREADED_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <event.h>
#include <signal.h>

#include "workqueue.h"

#define HEAD	"0x5489"
#define END		"0xCDEA"
#define HEAD1	0x54
#define	HEAD2	0x89
#define END1	0xCD
#define END2	0xEA

/* Port to listen on. */
#define SERVER_PORT 8090
/* Connection backlog (# of backlogged connections to accept). */
#define CONNECTION_BACKLOG 8
/* Socket read and write timeouts, in seconds. */
#define SOCKET_READ_TIMEOUT_SECONDS 10
#define SOCKET_WRITE_TIMEOUT_SECONDS 10
/* Number of worker threads.  Should match number of CPU cores reported in /proc/cpuinfo. */
#define NUM_THREADS 8
#define BUF_MAX_SIZE	4096

/* Behaves similarly to fprintf(stderr, ...), but adds file, line, and function information. */
#define errorOut(...) {\
	fprintf(stderr, "%s:%d: %s():\t", __FILE__, __LINE__, __FUNCTION__); \
	fprintf(stderr, __VA_ARGS__); \
}

/**
* Struct to carry around connection (client)-specific data.
*/
typedef struct client {
	/* The client's socket. */
	int fd;

	/* The event_base for this client. */
	struct event_base *evbase;

	/* The bufferedevent for this client. */
	struct bufferevent *buf_ev;

	/* The output buffer for this client. */
	struct evbuffer *output_buffer;

	/* Here you can add your own application-specific attributes which
	* are connection-specific. */
} client_t;

typedef struct _Packet{
	unsigned char pkg_head[2];
	unsigned char pkg_type[2];
	unsigned int pkg_len;
	char pkg_content[BUF_MAX_SIZE];
	unsigned char pkg_cs;
	unsigned char pkg_end[2];
}Packet;

class CServer
{  
public:  
	CServer();
	~CServer();
		
private:
};  

#endif
