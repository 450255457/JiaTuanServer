/*****************************************
> File Name : server_threaded.c
> Description : server_threaded.c  file
> Author : linden
> Date : 2015-09-06
*******************************************/

#include "server_threaded.h"

static struct event_base *evbase_accept;
static workqueue_t workqueue;

/* Signal handler function (defined below). */
static void sighandler(int signal);

/**
* Set a socket to non-blocking mode.
*/
static int setnonblock(int fd) {
	int flags;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0) return flags;
	/* 非阻塞I/O;如果read调用没有可读取的数据,或者write操作阻塞,read或write调用返回-1和EAGAIN错误 */
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) return -1;
	return 0;
}

static void closeClient(client_t *client) {
	if (client != NULL) {
		if (client->fd >= 0) {
			close(client->fd);
			client->fd = -1;
		}
	}
}

static void closeAndFreeClient(client_t *client) {
	if (client != NULL) {
		closeClient(client);
		if (client->buf_ev != NULL) {
			bufferevent_free(client->buf_ev);
			client->buf_ev = NULL;
		}
		if (client->evbase != NULL) {
			event_base_free(client->evbase);
			client->evbase = NULL;
		}
		if (client->output_buffer != NULL) {
			evbuffer_free(client->output_buffer);
			client->output_buffer = NULL;
		}
		free(client);
	}
}

/**
* Called by libevent when there is data to read.
*/
void buffered_on_read(struct bufferevent *bev, void *arg) {
	client_t *client = (client_t *)arg;
	char data[BUF_MAX_SIZE] = { 0 };
	int nbytes;
	nbytes = EVBUFFER_LENGTH(bev->input);
	evbuffer_remove(bev->input, data, nbytes);
	//解析data
	string sdata = data;
	printf("recv sdata : %s\n", sdata.c_str());
	Json::Reader reader;
	Json::Value value, return_item;
	Json::FastWriter writer_item;
	if (reader.parse(sdata, value) && (!value["FunctionName"].isNull()))
	{
		CDBManager MyDB;
		MyDB.initDB("localhost", "root", "123456", "JiaTuanSql");
		string sFunctionName = value["FunctionName"].asString();
		if ("register" == sFunctionName)
		{
			if (MyDB.user_register_func(value["PhoneNO"].asString(), value["Pwd"].asString()))
			{
				return_item["error_code"] = 0;
			}
			else
			{
				return_item["error_code"] = -111;
				return_item["error_desc"] = "register failed,maybe user_login duplicate.";
			}
		}
		else if ("login" == sFunctionName)
		{
			if (MyDB.user_login_func(value["User_login"].asString(), value["User_pass"].asString()))
			{
				return_item["error_code"] = 0;
			}
			else
			{
				return_item["error_code"] = -121;
				return_item["error_desc"] = "login failed,maybe the account or password mistake.";
			}
		}
	}
	else
	{
		return_item["error_code"] = -11;
		return_item["error_desc"] = "Error:json data parse.";
	}
	sdata = writer_item.write(return_item);
	printf("send sdata : %s\n", sdata.c_str());
	evbuffer_add(client->output_buffer, sdata.c_str(), sdata.size());
	if (bufferevent_write_buffer(bev, client->output_buffer)) {
		errorOut("Error sending data to client on fd %d\n", client->fd);
		closeClient(client);
	}



	///* Copy the data from the input buffer to the output buffer in 4096-byte chunks.
	//* There is a one-liner to do the whole thing in one shot, but the purpose of this server
	//* is to show actual real-world reading and writing of the input and output buffers,
	//* so we won't take that shortcut here. */
	//while ((nbytes = EVBUFFER_LENGTH(bev->input)) > 0) {
	//	/* Remove a chunk of data from the input buffer, copying it into our local array (data). */
	//	if (nbytes > 4096) nbytes = 4096;
	//	evbuffer_remove(bev->input, data, nbytes);
	//	/* Add the chunk of data from our local array (data) to the client's output buffer. */
	//	evbuffer_add(client->output_buffer, data, nbytes);
	//}

	///* Send the results to the client.  This actually only queues the results for sending.
	//* Sending will occur asynchronously, handled by libevent. */
	//if (bufferevent_write_buffer(bev, client->output_buffer)) {
	//	errorOut("Error sending data to client on fd %d\n", client->fd);
	//	closeClient(client);
	//}
}

/**
* Called by libevent when the write buffer reaches 0.  We only
* provide this because libevent expects it, but we don't use it.
*/
void buffered_on_write(struct bufferevent *bev, void *arg) {
}

/**
* Called by libevent when there is an error on the underlying socket
* descriptor.
*/
void buffered_on_error(struct bufferevent *bev, short what, void *arg) {
	closeClient((client_t *)arg);
}

static void server_job_function(struct job *job) {
	client_t *client = (client_t *)job->user_data;

	event_base_dispatch(client->evbase);
	closeAndFreeClient(client);
	free(job);
}

/**
* This function will be called by libevent when there is a connection
* ready to be accepted.
*/
void on_accept(int fd, short ev, void *arg) {
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	workqueue_t *workqueue = (workqueue_t *)arg;
	client_t *client;
	job_t *job;

	client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd < 0) {
		warn("accept failed");
		return;
	}
	printf("Run accept fd:%d\n", fd);
	/* Set the client socket to non-blocking mode. */
	if (setnonblock(client_fd) < 0) {
		;
		warn("failed to set client socket to non-blocking");
		close(client_fd);
		return;
	}

	/* Create a client object. */
	if ((client = (client_t *)malloc(sizeof(*client))) == NULL) {
		warn("failed to allocate memory for client state");
		close(client_fd);
		return;
	}
	memset(client, 0, sizeof(*client));
	client->fd = client_fd;

	/* Add any custom code anywhere from here to the end of this function
	* to initialize your application-specific attributes in the client struct. */

	if ((client->output_buffer = evbuffer_new()) == NULL) {
		warn("client output buffer allocation failed");
		closeAndFreeClient(client);
		return;
	}

	if ((client->evbase = event_base_new()) == NULL) {
		warn("client event_base creation failed");
		closeAndFreeClient(client);
		return;
	}

	/* Create the buffered event.
	*
	* The first argument is the file descriptor that will trigger
	* the events, in this case the clients socket.
	*
	* The second argument is the callback that will be called
	* when data has been read from the socket and is available to
	* the application.
	*
	* The third argument is a callback to a function that will be
	* called when the write buffer has reached a low watermark.
	* That usually means that when the write buffer is 0 length,
	* this callback will be called.  It must be defined, but you
	* don't actually have to do anything in this callback.
	*
	* The fourth argument is a callback that will be called when
	* there is a socket error.  This is where you will detect
	* that the client disconnected or other socket errors.
	*
	* The fifth and final argument is to store an argument in
	* that will be passed to the callbacks.  We store the client
	* object here.
	*/
	if ((client->buf_ev = bufferevent_new(client_fd, buffered_on_read, buffered_on_write, buffered_on_error, client)) == NULL) {
		warn("client bufferevent creation failed");
		closeAndFreeClient(client);
		return;
	}
	bufferevent_base_set(client->evbase, client->buf_ev);

	bufferevent_settimeout(client->buf_ev, SOCKET_READ_TIMEOUT_SECONDS, SOCKET_WRITE_TIMEOUT_SECONDS);

	/* We have to enable it before our callbacks will be
	* called. */
	bufferevent_enable(client->buf_ev, EV_READ);

	/* Create a job object and add it to the work queue. */
	if ((job = (job_t *)malloc(sizeof(*job))) == NULL) {
		warn("failed to allocate memory for job state");
		closeAndFreeClient(client);
		return;
	}
	job->job_function = server_job_function;
	job->user_data = client;

	workqueue_add_job(workqueue, job);
}

/**
* Run the server.  This function blocks, only returning when the server has terminated.
*/
int runServer(void) {
	int sockfd;
	struct sockaddr_in listen_addr;
	struct event ev_accept;
	int reuseaddr_on;

	/* Initialize libevent. */
	event_init();

	/* Set signal handlers
	 sigset_t：信号集,其被定义为一种数据类型
	 struct sigaction {
	 void     (*sa_handler)(int);	//此参数和signal()的参数handler 相同, 代表新的信号处理函数
	 void     (*sa_sigaction)(int, siginfo_t *, void *);
	 sigset_t   sa_mask;			//用来设置在处理该信号时暂时将sa_mask 指定的信号搁置
	 int        sa_flags;			//用来设置信号处理的其他相关操作,  A_NOCLDSTOP: 如果参数signum 为SIGCHLD, 则当子进程暂停时并不会通知父进程;SA_RESTART: 被信号中断的系统调用会自行重启;
	 void     (*sa_restorer)(void);	//此参数没有使用
	 };
	*/
	sigset_t sigset;
	sigemptyset(&sigset);	//初始化信号集，信号集里面的所有信号被清空
	/*struct sigaction siginfo = {
		.sa_handler = sighandler,
		.sa_mask = sigset,
		.sa_flags = SA_RESTART,
		};*/
	struct sigaction siginfo;
	siginfo.sa_handler = sighandler;
	siginfo.sa_mask = sigset;
	siginfo.sa_flags = SA_RESTART;
	sigaction(SIGINT, &siginfo, NULL);	//程序终止(interrupt)信号, 在用户键入INTR字符(通常是Ctrl-C)时发出 
	sigaction(SIGTERM, &siginfo, NULL);	//程序结束(terminate)信号, 与SIGKILL不同的是该信号可以被阻塞和处理.通常用来要求程序自己正常退出.shell命令kill缺省产生这个信号.
	/* Create our listening socket. */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		err(1, "listen failed");
	}
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(SERVER_PORT);
	if (bind(sockfd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
		err(1, "bind failed");
	}
	if (listen(sockfd, CONNECTION_BACKLOG) < 0) {
		err(1, "listen failed");
	}
	reuseaddr_on = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on));

	/* Set the socket to non-blocking, this is essential in event
	* based programming with libevent. */
	if (setnonblock(sockfd) < 0) {
		err(1, "failed to set server socket to non-blocking");
	}

	if ((evbase_accept = event_base_new()) == NULL) {
		perror("Unable to create socket accept event base");
		close(sockfd);
		return 1;
	}

	/* Initialize work queue. */
	if (workqueue_init(&workqueue, NUM_THREADS)) {
		perror("Failed to create work queue");
		close(sockfd);
		workqueue_shutdown(&workqueue);
		return 1;
	}

	/* We now have a listening socket, we create a read event to
	* be notified when a client connects. */
	event_set(&ev_accept, sockfd, EV_READ | EV_PERSIST, on_accept, (void *)&workqueue);
	event_base_set(evbase_accept, &ev_accept);
	event_add(&ev_accept, NULL);

	printf("Server is running...\n");

	/* Start the event loop. */
	event_base_dispatch(evbase_accept);

	event_base_free(evbase_accept);
	evbase_accept = NULL;

	close(sockfd);

	printf("Server shutdown.\n");

	return 0;
}

/**
* Kill the server.  This function can be called from another thread to kill the
* server, causing runServer() to return.
*/
void killServer(void) {
	fprintf(stdout, "Stopping socket listener event loop.\n");
	if (event_base_loopexit(evbase_accept, NULL)) {
		perror("Error shutting down server");
	}
	fprintf(stdout, "Stopping workers.\n");
	workqueue_shutdown(&workqueue);
}

static void sighandler(int signal) {
	fprintf(stdout, "Received signal %d: %s.  Shutting down.\n", signal, strsignal(signal));
	killServer();
}

/* Main function for demonstrating the echo server.
* You can remove this and simply call runServer() from your application. */
int main(int argc, char *argv[]) {
	return runServer();
}
