/*****************************************
> File Name: server.cpp
> Description:server端
> Author: linden
> Date: 2015-08-25
*******************************************/

/*
A trivial static http webserver using Libevent's evhttp.

This is not the best code in the world, and it does some fairly stupid stuff
that you would never want to do in a production webserver. Caveat hackor!

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <event2/http_compat.h>

#ifdef _EVENT_HAVE_NETINET_IN_H
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#endif

/* Compatibility for possible missing IPv6 declarations */
#include "libevent/util-internal.h"

#ifdef WIN32
#define stat _stat
#define fstat _fstat
#define open _open
#define close _close
#define O_RDONLY _O_RDONLY
#endif

char uri_root[512];

static const struct table_entry {
	const char *extension;
	const char *content_type;
} content_type_table[] = {
	{ "txt", "text/plain" },
	{ "c", "text/plain" },
	{ "h", "text/plain" },
	{ "html", "text/html" },
	{ "htm", "text/htm" },
	{ "css", "text/css" },
	{ "gif", "image/gif" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" },
	{ "pdf", "application/pdf" },
	{ "ps", "application/postsript" },
	{ NULL, NULL },
};

/* Try to guess a good content-type for 'path' */
static const char *
guess_content_type(const char *path)
{
	const char *last_period, *extension;
	const struct table_entry *ent;
	last_period = strrchr(path, '.');
	if (!last_period || strchr(last_period, '/'))
		goto not_found; /* no exension */
	extension = last_period + 1;
	for (ent = &content_type_table[0]; ent->extension; ++ent) {
		if (!evutil_ascii_strcasecmp(ent->extension, extension))
			return ent->content_type;
	}

not_found:
	return "application/misc";
}

/* Callback used for the /dump URI, and for every non-GET request:
* dumps all information to stdout and gives back a trivial 200 ok */
static void
dump_request_cb(struct evhttp_request *req, void *arg)
{
	const char *cmdtype;
	struct evkeyvalq *headers;
	struct evkeyval *header;
	struct evbuffer *buf;

	switch (evhttp_request_get_command(req)) {
	case EVHTTP_REQ_GET: cmdtype = "GET"; break;
	case EVHTTP_REQ_POST: cmdtype = "POST"; break;
	case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
	case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
	case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
	case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
	case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
	case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
	case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
	default: cmdtype = "unknown"; break;
	}

	printf("Received a %s request for %s\nHeaders:\n",
		cmdtype, evhttp_request_get_uri(req));

	headers = evhttp_request_get_input_headers(req);
	for (header = headers->tqh_first; header;
		header = header->next.tqe_next) {
		printf("  %s: %s\n", header->key, header->value);
	}

	buf = evhttp_request_get_input_buffer(req);
	puts("Input data: <<<");
	while (evbuffer_get_length(buf)) {
		int n;
		char cbuf[128];
		n = evbuffer_remove(buf, cbuf, sizeof(cbuf));
		if (n > 0)
			(void)fwrite(cbuf, 1, n, stdout);
	}
	puts(">>>");

	evhttp_send_reply(req, 200, "OK", NULL);
}

/* This callback gets invoked when we get any http request that doesn't match
* any other callback.  Like any evhttp server callback, it has a simple job:
* it must eventually call evhttp_send_error() or evhttp_send_reply().
*/
static void
send_document_cb(struct evhttp_request *req, void *arg)
{
	struct evbuffer *buf;
	buf = evbuffer_new();

	// 分析请求
	char *decode_uri = strdup((char*)evhttp_request_uri(req));
	struct evkeyvalq http_query;
	evhttp_parse_query(decode_uri, &http_query);
	free(decode_uri);

	// 从http头中获取参数
	const char *request_value = evhttp_find_header(&http_query, "data");

	// 返回HTTP头部
	evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=UTF-8");
	evhttp_add_header(req->output_headers, "Server", "my_httpd");
	//evhttp_add_header(req->output_headers, "Connection", "keep-alive");

	evhttp_add_header(req->output_headers, "Connection", "close");

	// 将要输出的值写入输出缓存
	if (request_value != NULL) {
		evbuffer_add_printf(buf, "%s", request_value);
	}
	else {
		evbuffer_add_printf(buf, "%s", "no error.");
	}

	// 输出
	evhttp_send_reply(req, HTTP_OK, "OK", buf);

	// 内存释放
	evhttp_clear_headers(&http_query);
	evbuffer_free(buf);
}

static void
syntax(void)
{
	fprintf(stdout, "Syntax: http-server <docroot>\n");
}

int
main(int argc, char **argv)
{
	struct event_base *base;
	struct evhttp *http;
	struct evhttp_bound_socket *handle;

	unsigned short port = 8090;
#ifdef WIN32
	WSADATA WSAData;
	WSAStartup(0x101, &WSAData);
#else
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return (1);
#endif
	//if (argc < 2) {
	//	syntax();
	//	return 1;
	//}

	base = event_base_new();
	if (!base) {
		fprintf(stderr, "Couldn't create an event_base: exiting\n");
		return 1;
	}

	/* Create a new evhttp object to handle requests. */
	http = evhttp_new(base);
	if (!http) {
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");
		return 1;
	}

	/* The /dump URI will dump all requests to stdout and say 200 ok. */
	evhttp_set_cb(http, "/dump", dump_request_cb, NULL);

	/* We want to accept arbitrary requests, so we need to set a "generic"
	* cb.  We can also add callbacks for specific paths. */
	evhttp_set_gencb(http, send_document_cb, NULL);

	/* Now we tell the evhttp what port to listen on */
	handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
	if (!handle) {
		fprintf(stderr, "couldn't bind to port %d. Exiting.\n",
			(int)port);
		return 1;
	}

	{
		/* Extract and display the address we're listening on. */
		struct sockaddr_storage ss;
		evutil_socket_t fd;
		ev_socklen_t socklen = sizeof(ss);
		char addrbuf[128];
		void *inaddr;
		const char *addr;
		int got_port = -1;
		fd = evhttp_bound_socket_get_fd(handle);
		memset(&ss, 0, sizeof(ss));
		if (getsockname(fd, (struct sockaddr *)&ss, &socklen)) {
			perror("getsockname() failed");
			return 1;
		}
		if (ss.ss_family == AF_INET) {
			got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
			inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
		}
		else if (ss.ss_family == AF_INET6) {
			got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
			inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
		}
		else {
			fprintf(stderr, "Weird address family %d\n",
				ss.ss_family);
			return 1;
		}
		addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf,
			sizeof(addrbuf));
		if (addr) {
			printf("Listening on %s:%d\n", addr, got_port);
			evutil_snprintf(uri_root, sizeof(uri_root),
				"http://%s:%d", addr, got_port);
		}
		else {
			fprintf(stderr, "evutil_inet_ntop failed\n");
			return 1;
		}
	}

	event_base_dispatch(base);

	return 0;
}


//
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//
//#include <event2/event.h>
//#include <event2/http.h>
//#include <event2/http_compat.h>
//#include <event2/buffer.h>
//#include <event2/keyvalq_struct.h>
//#include <event2/event_compat.h>
//
//
//void http_handler(struct evhttp_request *req, void *arg)
//{
//	struct evbuffer *buf;
//	buf = evbuffer_new();
//
//	// 分析请求
//	char *decode_uri = strdup((char*)evhttp_request_uri(req));
//	struct evkeyvalq http_query;
//	evhttp_parse_query(decode_uri, &http_query);
//	free(decode_uri);
//
//	// 从http头中获取参数
//	const char *request_value = evhttp_find_header(&http_query, "data");
//
//	// 返回HTTP头部
//	evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=UTF-8");
//	evhttp_add_header(req->output_headers, "Server", "my_httpd");
//	//evhttp_add_header(req->output_headers, "Connection", "keep-alive");
//
//	evhttp_add_header(req->output_headers, "Connection", "close");
//
//	// 将要输出的值写入输出缓存
//	if (request_value != NULL) {
//		evbuffer_add_printf(buf, "%s", request_value);
//	}
//	else {
//		evbuffer_add_printf(buf, "%s", "no error.");
//	}
//
//	// 输出
//	evhttp_send_reply(req, HTTP_OK, "OK", buf);
//
//	// 内存释放
//	evhttp_clear_headers(&http_query);
//	evbuffer_free(buf);
//}
//
//int main(int argc, char **argv)
//{
//	char *host_ip = "0.0.0.0";
//	int host_port = 8090;
//	int timeout = 5;
//
//	struct evhttp *httpd;
//
//	event_init();
//
//	//根据host_ip和host_port创建一个addrinfo结构体,然后创建一个socket,绑定到这个socket后,
//	//根据这些信息得到得到一个event(回调函数设置为accept_socket),然后将这个event关联到对应的event_base,
//	//之后插入到&http->sockets队列中,然后返回&http	
//	httpd = evhttp_start(host_ip, host_port);
//
//	if (httpd == NULL) {
//		fprintf(stderr, "Error: Unable to listen on %s:%d\n\n", host_ip, host_port);
//		exit(1);
//	}
//
//	// 设置请求超时时间
//	evhttp_set_timeout(httpd, timeout);
//
//	// 设置请求的处理函数
//	evhttp_set_gencb(httpd, http_handler, NULL);
//
//	event_dispatch();
//
//	evhttp_free(httpd);
//
//	return 0;
//}
