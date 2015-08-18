﻿/*****************************************
> File Name: server.cpp
> Description:server端
> Author: linden
> Date: 2015-08-25
*******************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "/libevent/include/event2/http_compat.h"
#include "/libevent/include/event2/buffer.h"
#include "/libevent/include/event2/keyvalq_struct.h"
#include "/libevent/include/event2/event_compat.h"
#include "/libevent/include/event2/http.h"

void http_handler(struct evhttp_request *req, void *arg)
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

int main(int argc, char **argv)
{
	char *host_ip = "0.0.0.0";
	int host_port = 8090;
	int timeout = 5;

	struct evhttp *httpd;

	event_init();

	//根据host_ip和host_port创建一个addrinfo结构体,然后创建一个socket,绑定到这个socket后,
	//根据这些信息得到得到一个event(回调函数设置为accept_socket),然后将这个event关联到对应的event_base,
	//之后插入到&http->sockets队列中,然后返回&http	
	httpd = evhttp_start(host_ip, host_port);

	if (httpd == NULL) {
		fprintf(stderr, "Error: Unable to listen on %s:%d\n\n", host_ip, host_port);
		exit(1);
	}

	// 设置请求超时时间
	evhttp_set_timeout(httpd, timeout);

	// 设置请求的处理函数
	evhttp_set_gencb(httpd, http_handler, NULL);

	event_dispatch();

	evhttp_free(httpd);

	return 0;
}
